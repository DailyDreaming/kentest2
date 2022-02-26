/* pslTrack - stuff to handle loading and display of
 * psl (blat format) based tracks. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hCommon.h"
#include "hash.h"
#include "linefile.h"
#include "jksql.h"
#include "hdb.h"
#include "hgTracks.h"
#include "psl.h"
#include "genbank.h"

#ifndef GBROWSE
#include "../gsid/gsidTable/gsidTable.h"
#define SELECT_SUBJ	"selectSubject"
struct gsidSubj *gsidSelectedSubjList = NULL;
struct gsidSeq *gsidSelectedSeqList = NULL;
#endif /* GBROWSE */

int pslGrayIx(struct psl *psl, boolean isXeno, int maxShade)
/* Figure out gray level for an RNA block. */
{
double misFactor;
double hitFactor;
int res;

if (isXeno)
    {
    misFactor = (psl->misMatch + psl->qNumInsert + psl->tNumInsert)*2.5;
    }
else
    {
    misFactor = (psl->misMatch + psl->qNumInsert)*5;
    }
misFactor /= (psl->match + psl->misMatch + psl->repMatch);
hitFactor = 1.0 - misFactor;
res = round(hitFactor * maxShade);
if (res < 1) res = 1;
if (res >= maxShade) res = maxShade-1;
return res;
}

static char *skipDb(char *tableName)
/* retun a pointer past the datbase part of the table name (if any) */
{
char *dot = tableName;

if ((dot = strchr(tableName, '.')) == NULL)
    return tableName;

return dot + 1;
}

static void filterMrna(struct track *tg, struct linkedFeatures **pLfList)
/* Apply filters if any to mRNA linked features. */
{
struct linkedFeatures *lf, *next, *newList = NULL, *oldList = NULL;
struct mrnaUiData *mud = tg->extraUiData;
struct mrnaFilter *fil;
char *type;
boolean anyFilter = FALSE;
unsigned colorIx = 0;
boolean isExclude = FALSE;
boolean andLogic = TRUE;
char query[256];
struct sqlResult *sr;
char **row;
struct sqlConnection *conn = NULL;

if (mud == NULL)
    return;

/* First make a quick pass through to see if we actually have
 * to do the filter. do this even if there are no items so 
 * track can be labeled as filtered. */
for (fil = mud->filterList; fil != NULL; fil = fil->next)
    {
    fil->pattern = cartUsualStringClosestToHome(cart, tg->tdb,FALSE,fil->suffix, "");
    if (fil->pattern[0] != 0)
        anyFilter = TRUE;
    }
if (!anyFilter)
    return;
labelTrackAsFiltered(tg);
if (*pLfList == NULL)
    return;

type = cartUsualStringClosestToHome(cart, tg->tdb, FALSE, mud->filterTypeSuffix, "red");
if (sameString(type, "exclude"))
    isExclude = TRUE;
else if (sameString(type, "include"))
    isExclude = FALSE;
else
    colorIx = getFilterColor(type, MG_BLACK);
type = cartUsualStringClosestToHome(cart, tg->tdb, FALSE, mud->logicTypeSuffix, "and");
andLogic = sameString(type, "and");

/* Make a pass though each filter, and start setting up search for
 * those that have some text. */
conn = hAllocConn(database);
for (fil = mud->filterList; fil != NULL; fil = fil->next)
    {
    if (fil->pattern[0] != 0)   // Filled above
	{
	fil->hash = newHash(10);
	if ((fil->mrnaTableIx = sqlFieldIndex(conn, gbCdnaInfoTable, skipDb(fil->table))) < 0)
	    internalErr();
	}
    }

/* Scan tables id/name tables to build up hash of matching id's. */
for (fil = mud->filterList; fil != NULL; fil = fil->next)
    {
    struct hash *hash = fil->hash;
    int wordIx, wordCount;
    char *words[128];

    if (hash != NULL)
	{
	boolean anyWild;
	char *dupPat = cloneString(fil->pattern);
	wordCount = chopLine(dupPat, words);
	for (wordIx=0; wordIx <wordCount; ++wordIx)
	    {
	    char *pattern = cloneString(words[wordIx]);
	    /* Special case for accessions as gbCdnaInfoTable is very large to
	       read into memory. */
	    if(sameString(fil->table, "acc"))
		{
		touppers(pattern);
		hashAdd(hash, pattern, NULL);
		freez(&pattern);
		continue;
		}

	    /* Load up entire table looking for matches. */
	    if (lastChar(pattern) != '*')
		{
		int len = strlen(pattern)+1;
		pattern = needMoreMem(pattern, len, len+1);
		pattern[len-1] = '*';
		}
	    anyWild = (strchr(pattern, '*') != NULL || strchr(pattern, '?') != NULL);
	    sqlSafef(query, sizeof query, "select id,name from %s", fil->table);
	    touppers(pattern);
	    sr = sqlGetResult(conn, query);
	    while ((row = sqlNextRow(sr)) != NULL)
		{
		boolean gotMatch;
		touppers(row[1]);
		if (anyWild)
		    gotMatch = wildMatch(pattern, row[1]);
		else
		    gotMatch = sameString(pattern, row[1]);
		if (gotMatch)
		    {
		    hashAdd(hash, row[0], NULL);
		    }
		}
	    sqlFreeResult(&sr);
	    freez(&pattern);
	    }
	freez(&dupPat);
	}
    }

/* Scan through linked features coloring and or including/excluding ones that
 * match filter. */
for (lf = *pLfList; lf != NULL; lf = next)
    {
    boolean passed = andLogic;
    next = lf->next;
    sqlSafef(query, sizeof query, "select * from %s where acc = '%s'", gbCdnaInfoTable, lf->name);
    sr = sqlGetResult(conn, query);
    if ((row = sqlNextRow(sr)) != NULL)
	{
	for (fil = mud->filterList; fil != NULL; fil = fil->next)
	    {
	    if (fil->hash != NULL)
		{
		if (hashLookup(fil->hash, row[fil->mrnaTableIx]) == NULL)
		    {
		    if (andLogic)
			passed = FALSE;
		    }
		else
		    {
		    if (!andLogic)
		        passed = TRUE;
		    }
		}
	    }
	}
    sqlFreeResult(&sr);
    if (passed ^ isExclude)
	{
	slAddHead(&newList, lf);
	if (colorIx != 0)
	    lf->filterColor = colorIx;
	}
    else
        {
	slAddHead(&oldList, lf);
	}
    }

slReverse(&newList);
slReverse(&oldList);
if (colorIx > 0)
   {
   /* Draw stuff that passes filter first in full mode, last in dense. */
   if (tg->visibility == tvDense)
       {
       newList = slCat(oldList, newList);
       }
   else
       {
       newList = slCat(newList, oldList);
       }
   }
*pLfList = newList;
tg->limitedVisSet = FALSE;	/* Need to recalculate this after filtering. */

/* Free up hashes, etc. */
for (fil = mud->filterList; fil != NULL; fil = fil->next)
    {
    hashFree(&fil->hash);
    }
hFreeConn(&conn);
}

static boolean shouldFilterGenbankPatentSequences(struct track *tg)
/* is this a genbank track with patent sequence filtering enabled */
{
char name[256];
safef(name, sizeof(name), "%s.%s", tg->tdb->track, SHOW_PATENT_SEQUENCES_SUFFIX);
return (sameString(tg->tdb->track, "mrna")|| sameString(tg->tdb->track, "intronEst"))
        && !cartUsualBoolean(cart, name, FALSE);
}

static void filterGenbankPatentSequences(struct track *tg, struct linkedFeatures **pLfList)
/* remove genbank patent sequences  */
{
struct linkedFeatures *lf, *newLf = NULL;
while ((lf = slPopHead(pLfList)) != NULL)
    {
    if (!isGenbankPatentAccession(lf->name))
        slAddHead(&newLf, lf);
    }
slReverse(&newLf);
*pLfList = newLf;
}

struct simpleFeature *sfFromPslX(struct psl *psl,int grayIx, int sizeMul)
{
        struct simpleFeature *sf = NULL, *sfList = NULL;
        unsigned *starts = psl->tStarts;
        unsigned *qStarts = psl->qStarts;
        unsigned *sizes = psl->blockSizes;
        int i, blockCount = psl->blockCount;
        boolean rcTarget = (psl->strand[1] == '-');

        for (i=0; i<blockCount; ++i)
                {
                AllocVar(sf);
                sf->start = sf->end = starts[i];
                sf->end += sizes[i]*sizeMul;
                sf->qStart = sf->qEnd = qStarts[i];
                sf->qEnd += sizes[i];
                if (rcTarget)
                    {
	                int s, e;
	                s = psl->tSize - sf->end;
	                e = psl->tSize - sf->start;
	                sf->start = s;
	                sf->end = e;
	                }
                sf->grayIx = grayIx;
                slAddHead(&sfList, sf);
                }
        return(sfList);
}

struct linkedFeatures *lfFromPslx(struct psl *psl,
	int sizeMul, boolean isXeno, boolean nameGetsPos, struct track *tg)
/* Create a linked feature item from pslx.  Pass in sizeMul=1 for DNA,
 * sizeMul=3 for protein.
 * Don't free psl afterwards! */
{
int grayIx = pslGrayIx(psl, isXeno, maxShade);
struct linkedFeatures *lf;
boolean rcTarget = (psl->strand[1] == '-');

AllocVar(lf);
lf->score = (psl->match - psl->misMatch - psl->repMatch);
lf->grayIx = grayIx;
if (nameGetsPos)
    {
    char buf[256];
    safef(buf, sizeof(buf), "%s:%d-%d %s:%d-%d", psl->qName, psl->qStart, psl->qEnd,
    	psl->tName, psl->tStart, psl->tEnd);
    lf->extra = cloneString(buf);
    int len = strlen(psl->qName) + 32;
    lf->name = needMem(len);
    safef(lf->name, len, "%s %s %dk", psl->qName, psl->strand, psl->qStart/1000);
    }
else
    lf->name = cloneString(psl->qName);
lf->orientation = orientFromChar(psl->strand[0]);
if (rcTarget)
    lf->orientation = -lf->orientation;

lf->components = sfFromPslX(psl, grayIx, sizeMul);
lf->start = lf->tallStart = psl->tStart;
lf->end = lf->tallEnd = psl->tEnd;

/* Hang on to psl for use in drawing phase (this is why caller must not free psl!): */
lf->original = psl;

return lf;
}

struct linkedFeatures *lfFromPsl(struct psl *psl, boolean isXeno)
/* Create a linked feature item from psl.
 * Don't free psl afterwards! */
{
return lfFromPslx(psl, 1, isXeno, FALSE, NULL);
}

static void connectedLfFromPslsInRange(struct sqlConnection *conn,
    struct track *tg, int start, int end, char *chromName,
    boolean isXeno, boolean nameGetsPos, int sizeMul)
/* Return linked features from range of table after have
 * already connected to database.. */
{
struct sqlResult *sr = NULL;
char **row;
int rowOffset;
char *optionChrStr;
struct linkedFeatures *lfList = NULL, *lf;
char optionChr[128]; /* Option -  chromosome filter */
char extraWhere[128];

safef( optionChr, sizeof(optionChr), "%s.chromFilter", tg->track);
optionChrStr = cartUsualString(cart, optionChr, "All");
if (startsWith("chr",optionChrStr))
    {
    safef(extraWhere, sizeof(extraWhere), "qName = \"%s\"",optionChrStr);
    sr = hRangeQuery(conn, tg->table, chromName, start, end, extraWhere, &rowOffset);
    }
else
    {
    safef(extraWhere, sizeof(extraWhere), " ");
    sr = hRangeQuery(conn, tg->table, chromName, start, end, NULL, &rowOffset);
    }

if (sqlCountColumns(sr) < 21+rowOffset)
    errAbort("trackDb has incorrect table type for track \"%s\"",
	     tg->track);
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct psl *psl = pslLoad(row+rowOffset);
    lf = lfFromPslx(psl, sizeMul, isXeno, nameGetsPos, tg);
    slAddHead(&lfList, lf);
    // Don't free psl - may be used by baseColor code (and freeing is slow)
    }
slReverse(&lfList);
if (tg->visibility != tvDense)
    slSort(&lfList, linkedFeaturesCmpStart);
if (tg->extraUiData)
    filterMrna(tg, &lfList);
if (shouldFilterGenbankPatentSequences(tg))
    filterGenbankPatentSequences(tg, &lfList);
tg->items = lfList;
sqlFreeResult(&sr);
}

static void lfFromPslsInRange(struct track *tg, int start, int end,
	char *chromName, boolean isXeno, boolean nameGetsPos, int sizeMul)
/* Return linked features from range of table. */
{
struct sqlConnection *conn = hAllocConn(database);
connectedLfFromPslsInRange(conn, tg, start, end, chromName,
	isXeno, nameGetsPos, sizeMul);
hFreeConn(&conn);
}

static void loadXenoPslWithPos(struct track *tg)
/* load up all of the psls from correct table into tg->items item list*/
{
lfFromPslsInRange(tg, winStart,winEnd, chromName, TRUE, TRUE, 1);
}

void pslChromMethods(struct track *tg, char *colorChromDefault)
/* Fill in custom parts of xeno psl <otherdb> track */
{
char option[128]; /* Option -  rainbow chromosome color */
char *optionStr ;
safef( option, sizeof(option), "%s.color", tg->track);
optionStr = cartUsualString(cart, option, colorChromDefault);
tg->mapItemName = lfMapNameFromExtra;
if( sameString( optionStr, "on" )) /*use chromosome coloring*/
    tg->itemColor = lfChromColor;
else
    tg->itemColor = NULL;
tg->loadItems = loadXenoPslWithPos;
}

void loadPsl(struct track *tg)
/* load up all of the psls from correct table into tg->items item list*/
{
lfFromPslsInRange(tg, winStart,winEnd, chromName, FALSE, FALSE, 1);
}

void loadProteinPsl(struct track *tg)
/* load up all of the psls from correct table into tg->items item list*/
{
lfFromPslsInRange(tg, winStart,winEnd, chromName, TRUE, FALSE, 3);
}

void loadXenoPsl(struct track *tg)
/* load up all of the psls from correct table into tg->items item list*/
{
lfFromPslsInRange(tg, winStart,winEnd, chromName, TRUE, FALSE, 1);
}

void pslMethods(struct track *track, struct trackDb *tdb,
	int argc, char *argv[])
/* Load up psl type methods. */
{
char *subType = ".";
if (argc >= 2)
   subType = argv[1];
linkedFeaturesMethods(track);
if (!tdb->useScore)
    track->colorShades = NULL;
if (sameString(subType, "protein"))
    {
    track->subType = lfSubXeno;
    track->loadItems = loadProteinPsl;
    }
else if (sameString(subType, "xeno"))
    {
    track->subType = lfSubXeno;
    track->loadItems = loadXenoPsl;
    if (argc >= 3)
	{
	pslChromMethods(track,
                    trackDbSettingOrDefault(tdb, "colorChromDefault", "on"));
	}
    }
else if (sameString(subType, "version"))
    {
    track->mapItemName = refGeneMapName;
    track->loadItems = loadPsl;
    }
else
    track->loadItems = loadPsl;
if (sameString(subType, "est"))
    track->drawItems = linkedFeaturesAverageDense;
}
