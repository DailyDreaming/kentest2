/* mRNA descriptions. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "dystring.h"
#include "spDb.h"
#include "hdb.h"
#include "genePred.h"
#include "bed.h"
#include "hgGene.h"
#include "genbank.h"


static boolean mrnaDescriptionsExists(struct section *section, 
	struct sqlConnection *conn, char *geneId)
/* Return TRUE if mrna  on this one. */
{
struct psl *list = NULL;
if (hTableExists(sqlGetDatabase(conn), "all_mrna"))
    {
    struct sqlResult *sr;
    char **row;
    struct psl *psl;
    int rowOffset;
    char extra[64];
    safef(extra, sizeof(extra), "strand='%c'", curGenePred->strand[0]);
    sr = hRangeQuery(conn, "all_mrna", curGeneChrom, curGeneStart, curGeneEnd,
    	extra, &rowOffset);
    while ((row = sqlNextRow(sr)) != NULL)
         {
	 psl = pslLoad(row+rowOffset);
	 slAddHead(&list, psl);
	 }
    slReverse(&list);
    section->items = list;
    }
return slCount(list) > 0;
}

static int basesShared(struct genePred *gp, struct psl *psl)
/* Return number of bases a&b share. */
{
int intersect = 0;
int i, blockCount = psl->blockCount;
int s,e;
for (i=0; i<blockCount; ++i)
    {
    s = psl->tStarts[i];
    e = s + psl->blockSizes[i];
    if (psl->strand[1] == '-')
	reverseIntRange(&s, &e, psl->tSize);
    intersect += gpRangeIntersection(gp, s, e);
    }
return intersect;
}

static void mrnaDescriptionsPrint(struct section *section, 
	struct sqlConnection *conn, char *geneId)
/* Print out mrna descriptions annotations. */
{
struct psl *psl, *pslList = section->items;
for (psl = pslList; psl != NULL; psl = psl->next)
    {
    if (basesShared(curGenePred, psl) > 12)	/* Filter out possible little noisy flecks. */
        {
	char query[512];
	char *description;
	sqlSafef(query, sizeof(query),
	    "select d.name from %s g,%s d"
	    " where g.acc='%s' and g.description = d.id"
	    , gbCdnaInfoTable, descriptionTable, psl->qName);
	description = sqlQuickString(conn, query);
	if (description != NULL)
	    {
	    char *url = "https://www.ncbi.nlm.nih.gov/entrez/query.fcgi"
	    		"?cmd=Search&db=Nucleotide&term=%s&doptcmdl=GenBank"
			"&tool=genome.ucsc.edu";
	    hPrintf("<A HREF=\"");
	    hPrintf(url, psl->qName);
	    hPrintf("\" TARGET=_blank>");
	    hPrintf("%s</A> - ", psl->qName);
	    hPrintf("%s<BR>", description);
	    }
	freeMem(description);
	}
    }
}

struct section *mrnaDescriptionsSection(struct sqlConnection *conn, 
	struct hash *sectionRa)
/* Create mrnaDescriptions section. */
{
struct section *section = sectionNew(sectionRa, "mrnaDescriptions");
section->exists = mrnaDescriptionsExists;
section->print = mrnaDescriptionsPrint;
return section;
}
