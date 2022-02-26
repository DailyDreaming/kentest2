/* gad - do GAD section. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "dystring.h"
#include "cheapcgi.h"
#include "spDb.h"
#include "hgGene.h"
#include "hdb.h"
#include "net.h"


static boolean gadExists(struct section *section, 
	struct sqlConnection *conn, char *geneId)
/* Return TRUE if gadAll table exists and it has an entry with the gene symbol */
{
char query[1024];
char *geneSymbol;

if (sqlTableExists(conn, "gadAll") == TRUE)
    {
    sqlSafef(query, sizeof(query), "select k.geneSymbol from kgXref k, gadAll g"
	" where k.kgId='%s' and k.geneSymbol = g.geneSymbol", geneId);
    geneSymbol = sqlQuickString(conn, query);
    if (geneSymbol != NULL) return(TRUE);
/*
    sqlSafefFrag(condStr, sizeof(condStr), 
    "k.kgId='%s' and k.geneSymbol = g.geneSymbol", geneId);
    geneSymbol = sqlGetField(database, "kgXref k, gadAll g", "k.geneSymbol", condStr);
*/
    }
return(FALSE);
}

static void gadPrint(struct section *section, 
	struct sqlConnection *conn, char *geneId)
/* Print out GAD section. */
{
int refPrinted = 0;
boolean showCompleteGadList;

char query[1024];
struct sqlResult *sr;
char **row;
struct dyString *currentCgiUrl;
char *upperDisease;

char *url = 
cloneString("http://geneticassociationdb.nih.gov");
char *itemName;

if (url != NULL && url[0] != 0)
    {
    sqlSafef(query, sizeof(query), "select k.geneSymbol from kgXref k, gadAll g"
	" where k.kgId='%s' and k.geneSymbol = g.geneSymbol", geneId);
    itemName = sqlQuickString(conn, query);
    showCompleteGadList = FALSE;
    if (cgiOptionalString("showAllRef") != NULL)
    	{
        if (sameWord(cgiOptionalString("showAllRef"), "Y") ||
	    sameWord(cgiOptionalString("showAllRef"), "y") )
	    {
	    showCompleteGadList = TRUE;
	    }
	}
    currentCgiUrl = cgiUrlString();
   
    printf("<B>Genetic Association Database (archive): ");
    printf("<A HREF=\"%s\" target=_blank>", url);
    printf("%s</B></A>\n", itemName);

    printf("<BR><B>CDC HuGE Published Literature:  ");
    printf("<A HREF=\"https://phgkb.cdc.gov/PHGKB/searchSummary.action"
        "?Mysubmit=Search&firstQuery=%s&__checkbox_gwas=true\" target=_blank>",
        itemName);
    printf("%s</B></A>\n", itemName);

    /* List diseases associated with the gene */
    sqlSafef(query, sizeof(query),
    "select distinct broadPhen from gadAll where geneSymbol='%s' and association = 'Y' order by broadPhen",
    itemName);
    sr = sqlMustGetResult(conn, query);
    row = sqlNextRow(sr);
    
    if (row != NULL) 
    	{
	upperDisease = replaceChars(row[0], "'", "''");
	touppers(upperDisease);
	printf("<BR><B>Positive Disease Associations:  </B>");
	printf("<A HREF=\"http://geneticassociationdb.nih.gov\" target=_blank>");
	printf("%s</B></A>\n", row[0]);
        row = sqlNextRow(sr);
    	}
    while (row != NULL)
        {
	upperDisease = replaceChars(row[0], "'", "''");
	touppers(upperDisease);
	printf(", <A HREF=\"http://geneticassociationdb.nih.gov\" target=_blank>");
	printf("%s</B></A>\n", row[0]);
        row = sqlNextRow(sr);
	}
    sqlFreeResult(&sr);

    refPrinted = 0;
    sqlSafef(query, sizeof(query), 
       "select broadPhen,reference,title,journal, pubMed, conclusion from gadAll where geneSymbol='%s' and association = 'Y' order by broadPhen",
       itemName);
    sr = sqlMustGetResult(conn, query);
    row = sqlNextRow(sr);
    
    if (row != NULL) printf("<BR><B>Related Studies: </B><OL>");
    while (row != NULL)
        {
        printf("<LI><B>%s </B>", row[0]);

	printf("<br>%s, %s, %s.\n", row[1], row[2], row[3]);
	if (!sameWord(row[4], ""))
	    {
	    printf(" [PubMed ");
	    printf("<A HREF=\"%s%s%s'\" target=_blank>",
	    "https://www.ncbi.nlm.nih.gov/entrez/query.fcgi?db=pubmed&cmd=Retrieve&dopt=Abstract&list_uids=",
	    row[4],"&query_hl=1&itool=genome.ucsc.edu");
	    printf("%s</B></A>]\n", row[4]);
	    }
	printf("<br><i>%s</i>\n", row[5]);
	
	printf("</LI>\n");
        refPrinted++;
        if ((!showCompleteGadList) && (refPrinted >= 3)) break;
	row = sqlNextRow(sr);
    	}
    sqlFreeResult(&sr);
    printf("</OL>");
    
    if ((!showCompleteGadList) && (row != NULL))
    	{
        printf("<B>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; more ...  </B>");
        printf(
	      "<A HREF=\"%s?showAllRef=Y&%s&#35;gad\">click here to view the complete list</A> ", 
	      "hgGene", currentCgiUrl->string);
    	}
    }
}

struct section *gadSection(struct sqlConnection *conn, 
	struct hash *sectionRa)
/* Create gad section. */
{
struct section *section = sectionNew(sectionRa, "gad");
section->exists = gadExists;
section->print = gadPrint;
return section;
}

