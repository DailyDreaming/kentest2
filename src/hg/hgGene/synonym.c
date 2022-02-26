/* Synonym - print out other names for this gene. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "hdb.h"
#include "linefile.h"
#include "dystring.h"
#include "hgGene.h"
#include "spDb.h"
#include "ccdsGeneMap.h"
#include "genbank.h"


static void printOurMrnaUrl(FILE *f, char *accession)
/* Print URL for Entrez browser on a nucleotide. */
{
fprintf(f, "../cgi-bin/hgc?%s&g=mrna&i=%s&c=%s&o=%d&t=%d&l=%d&r=%d&db=%s",
    cartSidUrlString(cart), accession, curGeneChrom, curGeneStart, curGeneEnd, curGeneStart,
    curGeneEnd, database);
}

static void printOurRefseqUrl(FILE *f, char *accession)
/* Print URL for Entrez browser on a nucleotide. */
{
fprintf(f, "../cgi-bin/hgc?%s&g=refGene&i=%s&c=%s&o=%d&l=%d&r=%d&db=%s",
    cartSidUrlString(cart),  accession, curGeneChrom, curGeneStart, curGeneStart,
    curGeneEnd, database);
}

static int countAlias(char *id, struct sqlConnection *conn)
/* Count how many valid gene symbols to be printed */
{
char query[256];
struct sqlResult *sr;
int cnt = 0;
char **row;
sqlSafef(query, sizeof(query), "select alias from kgAlias where kgId = '%s' order by alias", id);
sr = sqlGetResult(conn, query);

row = sqlNextRow(sr);
while (row != NULL)
    {
    /* skip kgId and the maint gene symbol (curGeneName) */
    if ((!sameWord(id, row[0])) && (!sameWord(row[0], curGeneName))) 
    	{
	cnt++;
	}
    row = sqlNextRow(sr);
    }
sqlFreeResult(&sr);
return(cnt);
}

char *aliasString(char *id, struct sqlConnection *conn)
/* return alias string as it would be printed in html, can free after use */
{
char query[256];
struct sqlResult *sr = NULL;
char **row;
int totalCount;
int cnt = 0;

totalCount = countAlias(id,conn);
if (totalCount > 0)
    {
    struct dyString *aliasReturn = dyStringNew(0);
    dyStringPrintf(aliasReturn, "<B>Alternate Gene Symbols:</B> ");
    sqlSafef(query, sizeof(query), "select alias from kgAlias where kgId = '%s' order by alias", id);
    sr = sqlGetResult(conn, query);
    row = sqlNextRow(sr);
    while (cnt < totalCount)
    	{
        /* skip kgId and the maint gene symbol (curGeneName) */
        if ((!sameWord(id, row[0])) && (!sameWord(row[0], curGeneName))) 
		{
    		dyStringPrintf(aliasReturn, "%s", row[0]);
		if (cnt < (totalCount-1)) dyStringPrintf(aliasReturn, ", ");
		cnt++;
		}
    	row = sqlNextRow(sr);
    	}
    dyStringPrintf(aliasReturn, "<BR>");   
    sqlFreeResult(&sr);
    return dyStringCannibalize(&aliasReturn);
    }
return NULL;
}

static void printAlias(char *id, struct sqlConnection *conn)
/* Print out description of gene given ID. */
{
char *aliases = aliasString(id, conn);

if (aliases)
    {
    hPrintf("%s", aliases);
    freeMem(aliases);
    }
}

static void printGeneSymbol (char *geneId, char *table, char *idCol, struct sqlConnection *conn)
/* Print out official Entrez gene symbol from a cross-reference table.*/
{
char query[256];
struct sqlResult *sr = NULL;
char **row;
char *geneSymbol;

if (sqlTableExists(conn, table))
    {
    hPrintf("<B>Entrez Gene Official Symbol:</B> ");
    sqlSafef(query, sizeof(query), "select geneSymbol from %s where %s = '%s'", table, idCol, geneId);
    sr = sqlGetResult(conn, query);
    if (sr != NULL)
        {
        row = sqlNextRow(sr);

        geneSymbol = cloneString(row[0]);
        if (!sameString(geneSymbol, ""))
            hPrintf("%s<BR>", geneSymbol);
        }
    }
sqlFreeResult(&sr);
}

static char *getRefSeqAcc(char *id, char *table, char *idCol, struct sqlConnection *conn)
/* Finds RefSeq accession from a cross-reference table. */
{
char query[256];
struct sqlResult *sr = NULL;
char **row;
char *refSeqAcc = NULL;

if (sqlTableExists(conn, table))
    {
    sqlSafef(query, sizeof(query), "select refSeq from %s where %s = '%s'", table, idCol, id);
    sr = sqlGetResult(conn, query);
    if (sr != NULL)
        {
        row = sqlNextRow(sr);
        refSeqAcc = cloneString(row[0]);
        }
    }
sqlFreeResult(&sr);
return refSeqAcc;
}


static void printCcds(char *kgId, struct sqlConnection *conn)
/* Print out CCDS ids most closely matching the kg. */
{
struct ccdsGeneMap *ccdsKgs = NULL;
if (sqlTableExists(conn, "ccdsKgMap"))
    ccdsKgs = ccdsGeneMapSelectByGene(conn, "ccdsKgMap", kgId, 0.0);
if (ccdsKgs != NULL)
    {
    struct ccdsGeneMap *ccdsKg;
    hPrintf("<B>CCDS:</B> ");
    /* since kg is not by location (even though we have a
     * curGeneStart/curGeneEnd), we need to use the location in the 
     * ccdsGeneMap */
    for (ccdsKg = ccdsKgs; ccdsKg != NULL; ccdsKg = ccdsKg->next)
        {
        if (ccdsKg != ccdsKgs)
            hPrintf(", ");
        hPrintf("<A href=\"../cgi-bin/hgc?%s&g=ccdsGene&i=%s&c=%s&o=%d&l=%d&r=%d&db=%s\">%s</A>",
                cartSidUrlString(cart), ccdsKg->ccdsId, ccdsKg->chrom, ccdsKg->chromStart, ccdsKg->chromStart,
                ccdsKg->chromEnd, database, ccdsKg->ccdsId);
        }
    hPrintf("<BR>\n");   
    }
}

static char *addCommaSpace(char *inStr)
/* return all converted character '|' found in inStr to string ', '  */
{
char outStr[2048];

char *chp, *chpOut;
chp = inStr;
chpOut = outStr;

while (*chp != '\0')
    {
    *chpOut = *chp;
    if (*chp == '|')
       {
       *chpOut = ',';
       chpOut ++;
       *chpOut = ' ';
       }
    chp++;
    chpOut++;
    }
*chpOut = '\0';

//return inStr;
return strdup(outStr);
}

static void rgdGene2SynonymPrint(struct section *section,
        struct sqlConnection *conn, char *rgdGeneId)
{
char *geneSym = NULL, *geneName = NULL;
char query[256], **row;
struct sqlResult *sr;
if (rgdGeneId != NULL)
    {
    sqlSafef(query, sizeof(query), 
	    "select old_symbol, old_name from rgdGene2Raw where gene_rgd_id = '%s'", 
	    rgdGeneId+4L);
    sr = sqlGetResult(conn, query);
    if ((row = sqlNextRow(sr)) != NULL)
	{
	if (row[0][0] != 0 && !sameString(row[0], "n/a"))
	    {
	    geneSym = cloneString(row[0]);
	    hPrintf("<B>Symbol:</B> %s ", addCommaSpace(geneSym));
	    hPrintf("<BR>\n");
	    }
	if (row[1][0] != 0 && !sameString(row[0], "n/a"))
	    {
	    geneName = cloneString(row[1]);
	    hPrintf("<B>Name:</B> %s ", addCommaSpace(geneName));
	    hPrintf("<BR>\n");
	    }
	}
    sqlFreeResult(&sr);

    sqlSafef(query, sizeof(query), 
	    "select value from rgdGene2ToRefSeq where name= '%s'", rgdGeneId);
    sr = sqlGetResult(conn, query);
    if ((row = sqlNextRow(sr)) != NULL)
	{
        hPrintf("<B>RefSeq Accession: </B> <A HREF=\"");
	printOurRefseqUrl(stdout, row[0]);
	hPrintf("\">%s</A><BR>\n", row[0]);
	}
    sqlFreeResult(&sr);
    
    sqlSafef(query, sizeof(query), 
	    "select value from rgdGene2ToUniProt where name= '%s'", rgdGeneId);
    sr = sqlGetResult(conn, query);
    if ((row = sqlNextRow(sr)) != NULL)
    	{
	char *spId, *spDisplayId, *oldDisplayId;
	spId = row[0];
	hPrintf("<B>Protein: </B>");
        hPrintf("<A HREF=\"http://www.uniprot.org/uniprot/%s\" "
	    "TARGET=_blank>%s</A>\n",
	    spId, spId);
    
        /* show SWISS-PROT display ID if it is different than the accession ID */
        /* but, if display name is like: Q03399 | Q03399_HUMAN, then don't show display name */
        spDisplayId = spAnyAccToId(spConn, spId);
        if (spDisplayId == NULL) 
    	    {
	    errAbort("<br>The corresponding protein %s of this gene is not found in our current UniProtKB DB.", spId);
	    }   
	
        if (strstr(spDisplayId, spId) == NULL)
	    {
	    hPrintf(" (aka %s", spDisplayId);
	    /* show once if the new and old displayId are the same */
 	    oldDisplayId = oldSpDisplayId(spDisplayId);
	    if (oldDisplayId != NULL)
 	    	{
            	if (!sameWord(spDisplayId, oldDisplayId)
                    && !sameWord(spId, oldDisplayId))
	    	    {
	    	    hPrintf(" or %s", oldDisplayId);
	    	    }  
	        }	
	    hPrintf(")<BR>\n");
	    }  
   	}
    sqlFreeResult(&sr);
    }
}
static void synonymPrint(struct section *section, 
	struct sqlConnection *conn, char *id)
/* Print out SwissProt comments - looking up typeId/commentVal. */
{
char *protAcc = getSwissProtAcc(conn, spConn, id);
char *spDisplayId;
char *refSeqAcc = "";
char *mrnaAcc = "";
char *oldDisplayId;
char condStr[255];
char *kgProteinID;
char *parAcc; /* parent accession of a variant splice protein */
char *chp;

if (isRgdGene(conn))
    {
    rgdGene2SynonymPrint(section,conn, id);
    return;
    }
if (sqlTableExists(conn, "kgAlias"))
    printAlias(id, conn);
if (sameWord(genome, "Zebrafish"))
    {
    char *xrefTable = "ensXRefZfish";
    char *geneIdCol = "ensGeneId";
    /* get Gene Symbol and RefSeq accession from Zebrafish-specific */
    /* cross-reference table */
    printGeneSymbol(id, xrefTable, geneIdCol, conn);
    refSeqAcc = getRefSeqAcc(id, xrefTable, geneIdCol, conn);
    hPrintf("<B>ENSEMBL ID:</B> %s", id);
    }
else
    {
    char query[256];
    char *toRefTable = genomeOptionalSetting("knownToRef");
    if (toRefTable != NULL && sqlTableExists(conn, toRefTable))
        {
	sqlSafef(query, sizeof(query), "select value from %s where name='%s'", toRefTable,
		id);
	refSeqAcc = emptyForNull(sqlQuickString(conn, query));
	}
    if (sqlTableExists(conn, "kgXref"))
	{
	sqlSafef(query, sizeof(query), "select mRNA from kgXref where kgID='%s'", id);
	mrnaAcc = emptyForNull(sqlQuickString(conn, query));
	}
    if (sameWord(genome, "C. elegans"))
	hPrintf("<B>WormBase ID:</B> %s<BR>", id);
    else
	hPrintf("<B>UCSC ID:</B> %s<BR>", isGencode2 ? curAlignId : curGeneId );
    }
    
if (refSeqAcc[0] != 0)
    {
    hPrintf("<B>RefSeq Accession: </B> <A HREF=\"");
    printOurRefseqUrl(stdout, refSeqAcc);
    hPrintf("\">%s</A><BR>\n", refSeqAcc);
    }
else if (mrnaAcc[0] != 0)
    {
    sqlSafefFrag(condStr, sizeof(condStr), "acc = '%s'", mrnaAcc);
    if (sqlGetField(database, gbCdnaInfoTable, "acc", condStr) != NULL)
        {
    	hPrintf("<B>Representative RNA: </B> <A HREF=\"");
    	printOurMrnaUrl(stdout, mrnaAcc);
    	hPrintf("\">%s</A><BR>\n", mrnaAcc);
    	}
    else
    /* do not show URL link if it is not found in gbCdnaInfoTable */
    	{
    	hPrintf("<B>Representative RNA: %s </B>", mrnaAcc);
    	}
    }
if (protAcc != NULL)
    {
    kgProteinID = cloneString("");
    if (hTableExists(sqlGetDatabase(conn), "knownGene")
        && (isNotEmpty(curGeneChrom) &&
	      differentWord(curGeneChrom,"none")))
    	{
    	sqlSafefFrag(condStr, sizeof(condStr), "name = '%s' and chrom = '%s' and txStart=%d and txEnd=%d", 
	        id, curGeneChrom, curGeneStart, curGeneEnd);
    	kgProteinID = sqlGetField(sqlGetDatabase(conn), "knownGene", "proteinID", condStr);
    	}

    hPrintf("<B>Protein: ");
    if (strstr(kgProteinID, "-") != NULL)
        {
	parAcc = cloneString(kgProteinID);
	chp = strstr(parAcc, "-");
	*chp = '\0';
	
        /* show variant splice protein and the UniProt link here */
	hPrintf("<A HREF=\"http://www.uniprot.org/uniprot/%s\" "
	    "TARGET=_blank>%s</A></B>, splice isoform of ",
	    kgProteinID, kgProteinID);
        hPrintf("<A HREF=\"http://www.uniprot.org/uniprot/%s\" "
	    "TARGET=_blank>%s</A></B>\n",
	    parAcc, parAcc);
	}
    else
        {
        hPrintf("<A HREF=\"http://www.uniprot.org/uniprot/%s\" "
	    "TARGET=_blank>%s</A></B>\n",
	    protAcc, protAcc);
	}
    /* show SWISS-PROT display ID if it is different than the accession ID */
    /* but, if display name is like: Q03399 | Q03399_HUMAN, then don't show display name */
    spDisplayId = spAnyAccToId(spConn, protAcc);
    if (spDisplayId == NULL) 
    	{
	errAbort("<br>%s seems to no longer be a valid protein ID in our latest UniProtKB DB.", protAcc);
	}
	
    if (strstr(spDisplayId, protAcc) == NULL)
	{
	hPrintf(" (aka %s", spDisplayId);
	/* show once if the new and old displayId are the same */
 	oldDisplayId = oldSpDisplayId(spDisplayId);
	if (oldDisplayId != NULL)
 	    {
            if (!sameWord(spDisplayId, oldDisplayId)
                && !sameWord(protAcc, oldDisplayId))
	    	{
	    	hPrintf(" or %s", oldDisplayId);
	    	}
	    }
	hPrintf(")<BR>\n");
	}
    }
printCcds(id, conn);

}

struct section *synonymSection(struct sqlConnection *conn,
	struct hash *sectionRa)
/* Create synonym (aka Other Names) section. */
{
struct section *section = sectionNew(sectionRa, "synonym");
section->print = synonymPrint;
return section;
}
