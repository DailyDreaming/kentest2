/* ctd - do CTD section. */

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


static boolean ctdExists(struct section *section, 
	struct sqlConnection *conn, char *geneId)
/* Return TRUE if CTD database exists and it has an entry with the gene symbol */
{
char query[1024];
char *geneSymbol;
if (isRgdGene(conn))
    {
    if (sqlTableExists(conn, "rgdGene2Xref") == FALSE) return FALSE;
    }
else
    {
    if (sqlTableExists(conn, "kgXref") == FALSE) return FALSE;
    }

if (sqlTableExists(conn, "hgFixed.ctdSorted") == TRUE)
    {
    if (isRgdGene(conn))
	{
	sqlSafef(query, sizeof(query), "select ChemicalId from rgdGene2Xref x, hgFixed.ctdSorted c"
	" where x.info=c.GeneSymbol and infoType = 'Name' and rgdGeneId='%s' limit 1", geneId);
	geneSymbol = sqlQuickString(conn, query);
	}
    else
        {
	sqlSafef(query, sizeof(query), "select ChemicalId from kgXref x, hgFixed.ctdSorted c"
	" where x.geneSymbol=c.GeneSymbol and kgId='%s' limit 1", geneId);
	geneSymbol = sqlQuickString(conn, query);
	}

    if (geneSymbol != NULL) return(TRUE);
    }
return(FALSE);
}

static void ctdPrint(struct section *section, 
	struct sqlConnection *conn, char *geneId)
/* Print out CTD section. */
{
char query[256];
struct sqlResult *sr;
char **row;
char *chemId, *chemName;
int chemCnt;
int first = 1;
boolean showCompleteCtdList;
struct dyString *currentCgiUrl;

showCompleteCtdList = FALSE;
if (cgiOptionalString("showAllCtdRef") != NULL)
    {
    if (sameWord(cgiOptionalString("showAllCtdRef"), "Y") ||
	sameWord(cgiOptionalString("showAllCtdRef"), "y") )
	{
	showCompleteCtdList = TRUE;
	}
    cartRemove(cart, "showAllCtdRef");
    }
currentCgiUrl = cgiUrlString();
    
/* List chemicals related to this gene */
if (isRgdGene(conn))
    {
    sqlSafef(query, sizeof(query),
          "select ChemicalId, ChemicalName from rgdGene2Xref x, hgFixed.ctdSorted c where x.info=c.GeneSymbol and rgdGeneId='%s' and infoType='Name'", 
	  geneId);
    }
else
    {
    sqlSafef(query, sizeof(query),
          "select ChemicalId, ChemicalName from kgXref x, hgFixed.ctdSorted c where x.geneSymbol=c.GeneSymbol and kgId='%s'", 
	  geneId);
    }

sr = sqlMustGetResult(conn, query);
row = sqlNextRow(sr);
    
chemCnt = 0;
while (row != NULL) 
    {
    chemId   = cloneString(row[0]);
    chemName = cloneString(row[1]);
   
    if (first)
    	{
	printf("<B>The following chemicals interact with this gene</B>\n");
	printf("<UL>");
	first = 0;
	}
    
    printf("<LI><A HREF=\"http://ctdbase.org/detail.go?type=chem&acc=%s\" target=_blank>", 
    	   chemId);
    printf("%s</A>\n", chemId);
    printf("%s\n", chemName);
    printf("</LI>");	
    chemCnt++;
    row = sqlNextRow(sr);
    /* Initially, just show no more than 10 items */
    if ((!showCompleteCtdList) && (chemCnt >= 10) && (row != NULL))
	{
	chemCnt++;
	break;
	}
    }
sqlFreeResult(&sr);

if (!first) printf("</UL>");

/* Offer user the option to see the complete list */
if ((!showCompleteCtdList) && chemCnt > 10)
    {
    printf("<B>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; more ...  </B>");
    printf("<A HREF=\"%s?showAllCtdRef=Y&%s&#35;ctd\">click here to view the complete list</A> " 
	   ,"hgGene", currentCgiUrl->string);
    }

return;
}

struct section *ctdSection(struct sqlConnection *conn, 
	struct hash *sectionRa)
/* Create CTD section. */
{
struct section *section = sectionNew(sectionRa, "ctd");
section->exists = ctdExists;
section->print = ctdPrint;
return section;
}

