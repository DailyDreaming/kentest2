/* Handle details page for ENCODE tracks. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "cart.h"
#include "hgc.h"
#include "hCommon.h"
#include "hgColors.h"
#include "customTrack.h"
#include "web.h"
#include "encode/encodePeak.h"
#include "peptideMapping.h"
#include "chromAlias.h"

#ifdef UNUSED
static boolean pairInList(struct slPair *pair, struct slPair *list)
/* Return TRUE if pair is in list. */
{
struct slPair *el;
for (el = list; el != NULL; el = el->next)
    if (sameString(pair->name, el->name) && sameString(pair->val, el->val))
        return TRUE;
return FALSE;
}

static boolean selGroupListMatch(struct trackDb *tdb, struct slPair *selGroupList)
/* Return TRUE if tdb has match to every item in selGroupList */
{
char *subGroups = trackDbSetting(tdb, "subGroups");
if (subGroups == NULL)
    return FALSE;
struct slPair *groupList = slPairFromString(subGroups);
struct slPair *selGroup;
for (selGroup = selGroupList; selGroup != NULL; selGroup = selGroup->next)
    {
    if (!pairInList(selGroup, groupList))
        return FALSE;
    }
return TRUE;
}

static void rAddMatching(struct trackDb *tdb, struct slPair *selGroupList, struct slName **pList)
/* Add track and any descendents that match selGroupList to pList */
{
if (selGroupListMatch(tdb, selGroupList))
    slNameAddHead(pList, tdb->track);
struct trackDb *sub;
for (sub = tdb->subtracks; sub != NULL; sub = sub->next)
    rAddMatching(sub, selGroupList, pList);
}
#endif//def UNUSED

static void printPeak(char **row, int rowOffset, char *item, char *chrom, int start, int end, enum encodePeakType peakType)
{
char **rowPastOffset = row + rowOffset;
if ((sqlUnsigned(rowPastOffset[1]) != start) ||  (sqlUnsigned(rowPastOffset[2]) != end))
    return;
if (!sameString(rowPastOffset[3], item))
    return;

float signal = -1;
float pValue = -1;
float qValue = -1;


/* Name */
if (rowPastOffset[3][0] != '.')
    printf("<B>Name:</B> %s<BR>\n", rowPastOffset[3]);
/* Position */
printf("<B>Position:</B> "
   "<A HREF=\"%s&db=%s&position=%s%%3A%d-%d\">%s:%d-%d</a><BR>\n",
   hgTracksPathAndSettings(), database, chrom, start+1, end, chrom, start+1, end);
/* Print peak base */
if ((peakType == narrowPeak) || (peakType == encodePeak))
    {
    int peak = sqlSigned(rowPastOffset[9]);
    if (peak > -1)
        printf("<B>Peak point:</B> %d<BR>\n", start + peak + 1); // one based
    }
/* Strand, score */
if (rowPastOffset[5][0] != '.')
        printf("<B>Strand:</B> %c<BR>\n", rowPastOffset[5][0]);
printf("<B>Score:</B> %d<BR>\n", sqlUnsigned(rowPastOffset[4]));
/* signalVal, pVal */
if (peakType != gappedPeak)
    {
    signal = sqlFloat(rowPastOffset[6]);
    pValue = sqlFloat(rowPastOffset[7]);
    qValue = sqlFloat(rowPastOffset[8]);
    }
else
    {
    signal = sqlFloat(rowPastOffset[12]);
    pValue = sqlFloat(rowPastOffset[13]);
    qValue = sqlFloat(rowPastOffset[14]);
    }
if (signal >= 0)
    printf("<B>Signal value:</B> %.3f<BR>\n", signal);
if (pValue >= 0)
    printf("<B>P-value (-log10):</B> %.3f<BR>\n", pValue);
if (qValue >= 0)
    printf("<B>Q-value (FDR): </B> %.3f<BR>\n", qValue);
}

void doBigEncodePeak(struct trackDb *tdb, struct customTrack *ct, char *item)
/*  details for encodePeak type tracks. */
{
enum encodePeakType peakType = narrowPeak;
char *chrom = cartString(cart,"c");
int start = cgiInt("o");
int end = cgiInt("t");
boolean firstTime = TRUE;
//peakType = encodePeakInferTypeFromTable(db, table, tdb->type);
if (peakType == 0)
    errAbort("unrecognized peak type from table %s", tdb->table);
genericHeader(tdb, NULL);  // genericClickHandlerPlus gets there first anyway (maybe except for encodePeak custom tracks).
char *fileName = bbiNameFromSettingOrTable(tdb, NULL, tdb->table);
struct bbiFile *bbi =  bigBedFileOpenAlias(fileName, chromAliasChromToAliasHash(database));
struct lm *lm = lmInit(0);
struct bigBedInterval *bb, *bbList =  bigBedIntervalQuery(bbi, chrom, start, end, 0, lm);
int fieldCount = 10;
char *bedRow[fieldCount];
char startBuf[16], endBuf[16];

for (bb = bbList; bb != NULL; bb = bb->next)
    {
    bigBedIntervalToRow(bb, chrom, startBuf, endBuf, bedRow, ArraySize(bedRow));
    if (firstTime)
        firstTime = FALSE;
    else // print separator
        printf("<BR>\n");
    printPeak(bedRow, 0, item, chrom, start, end, peakType);
    }
}

void doEncodePeak(struct trackDb *tdb, struct customTrack *ct, char *item)
/*  details for encodePeak type tracks. */
{
struct sqlConnection *conn;
struct sqlResult *sr;
enum encodePeakType peakType;
char **row;
char *db;
char *table = tdb->table;
char *chrom = cartString(cart,"c");
int start = cgiInt("o");
int end = cgiInt("t");
int rowOffset;
boolean firstTime = TRUE;
/* connect to DB */
if (ct)
    {
    db = CUSTOM_TRASH;
    table = ct->dbTableName;
    }
else
    db = database;
conn = hAllocConn(db);
peakType = encodePeakInferTypeFromTable(db, table, tdb->type);
if (peakType == 0)
    errAbort("unrecognized peak type from table %s", tdb->table);
genericHeader(tdb, NULL);  // genericClickHandlerPlus gets there first anyway (maybe except for encodePeak custom tracks).
sr = hOrderedRangeQuery(conn, table, chrom, start, end,
			NULL, &rowOffset);
while((row = sqlNextRow(sr)) != NULL)
    {
    if (firstTime)
        firstTime = FALSE;
    else // print separator
        printf("<BR>\n");
    printPeak(row, rowOffset, item, chrom, start, end, peakType);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
}

int encodeFiveCInterCmp(const void *va, const void *vb)
/* reverse sort on bed nine's reserved field which in this */
/* case is the where the strength of the interaction is stored */
{
const struct bed *a = *((struct bed **)va);
const struct bed *b = *((struct bed **)vb);
return b->itemRgb - a->itemRgb;
}

void doEncodeFiveC(struct sqlConnection *conn, struct trackDb *tdb)
/* Print details for 5C track */
{
char *interTable = trackDbRequiredSetting(tdb, "interTable");
char *interTableKind = trackDbRequiredSetting(tdb, "interTableKind");
char **row;
char *chrom = cartString(cart,"c");
int start = cgiInt("o");
int end = cgiInt("t");
int rowOffset;
int outCount = 0;
struct sqlResult *sr;
struct bed *interList = NULL, *inter;
genericHeader(tdb, NULL);
sr = hOrderedRangeQuery(conn, interTable, chrom, start, end, NULL, &rowOffset);
printf("<B>Position:</B> "
       "<A HREF=\"%s&db=%s&position=%s%%3A%d-%d\">%s:%d-%d</a><BR>\n",
       hgTracksPathAndSettings(), database, chrom, start+1, end, chrom, start+1, end);
while ((row = sqlNextRow(sr)) != NULL)
    {
    inter = bedLoadN(row + rowOffset, 9);
    slAddHead(&interList, inter);
    }
slSort(&interList, encodeFiveCInterCmp);
webNewSection("Top %s interations", interTableKind);
webPrintLinkTableStart();
webPrintLabelCell("Position");
webPrintLabelCell("5C signal");
webPrintLabelCell("Distance");
webPrintLinkTableNewRow();
for (inter = interList; inter != NULL; inter = inter->next)
    {
    char s[1024];
    int distance = 0;
    safef(s, sizeof(s), "<A HREF=\"%s&db=%s&position=%s%%3A%d-%d\">%s:%d-%d</A>",
       hgTracksPathAndSettings(), database, chrom, inter->thickStart+1, inter->thickEnd, chrom, inter->thickStart+1, inter->thickEnd);
    webPrintLinkCell(s);
    safef(s, sizeof(s), "%d", inter->itemRgb);
    webPrintLinkCell(s);
    if (start > inter->thickStart)
	distance = inter->thickEnd - start;
    else
	distance = inter->thickStart - end;
    safef(s, sizeof(s), "%d", distance);
    webPrintLinkCell(s);
    if (++outCount == 50)
	break;
    if (inter->next != NULL)
	webPrintLinkTableNewRow();
    }
webPrintLinkTableEnd();
sqlFreeResult(&sr);
}

void doPeptideMapping(struct sqlConnection *conn, struct trackDb *tdb, char *item)
/* Print details for a peptideMapping track.  */
{
char *chrom = cartString(cart,"c");
int start = cgiInt("o");
int end = cgiInt("t");
char query[256];
char **row;
struct sqlResult *sr;
struct peptideMapping pos;
int rowOffset = 0;  // skip bin field
int found = 0;
genericHeader(tdb, NULL);

/* Just get the current item. */
sqlSafef(query, sizeof(query), 
      "select * from %s where name='%s' and chrom='%s' and chromStart=%d and chromEnd=%d", 
      tdb->track, item, chrom, start, end);
sr = sqlGetResult(conn, query);

if (sqlFieldColumn(sr, "bin") == 0)
    rowOffset = 1;
    
while ((row = sqlNextRow(sr)) != NULL)
    {
    ++found;
    peptideMappingStaticLoad(row + rowOffset, &pos);
    if (found == 1)
	{
	printf("<B>Item:</B> %s<BR>\n", pos.name);
	printPos(pos.chrom, pos.chromStart, pos.chromEnd, pos.strand, TRUE, item);

	printf("<BR>\n");
	printf("Additional details for all peptide mappings of %s:<BR>\n", item);

	webPrintLinkTableStart();
	webPrintLabelCell("Score");
	webPrintLabelCell("Raw Score");
	webPrintLabelCell("Spectrum ID");
	webPrintLabelCell("Peptide Rank");
	}
    webPrintLinkTableNewRow();
    webPrintIntCell(pos.score);
    webPrintDoubleCell(pos.rawScore);
    webPrintLinkCell(pos.spectrumId);
    webPrintIntCell(pos.peptideRank);
    }
if (found == 0)
    errAbort("No items in range");

webPrintLinkTableEnd();
sqlFreeResult(&sr);

/* Draw table of other locations */
printf("<BR>\n");
printf("<B>Peptide Repeat Count:</B> %d<BR>\n", pos.peptideRepeatCount);
if (pos.peptideRepeatCount > 1)
    {
    struct hash *hash = hashNew(8);
    struct peptideMapping anotherPos;
    sqlSafef(query, sizeof(query), 
          "select * from %s where name='%s' and not (chrom='%s' and chromStart=%d and chromEnd=%d)", 
	  tdb->track, item, chrom, start, end);
    printf("<BR>\n");
    webPrintLinkTableStart();
    webPrintLabelCell("Other genomic loci");
    sr = sqlGetResult(conn, query);
    while ((row = sqlNextRow(sr)) != NULL)
	{
	char s[1024];
	peptideMappingStaticLoad(row + rowOffset, &anotherPos);
	char k[1024];
	safef(k, sizeof k, "%s.%d.%d", anotherPos.chrom, anotherPos.chromStart, anotherPos.chromEnd);
	if (!hashLookup(hash, k))
	    {
	    hashAdd(hash, k, NULL);
	    safef(s, sizeof(s), "<A HREF=\"%s&db=%s&position=%s%%3A%d-%d\">%s:%d-%d</A>",
		  hgTracksPathAndSettings(), database, anotherPos.chrom, anotherPos.chromStart+1, 
		  anotherPos.chromEnd, anotherPos.chrom, anotherPos.chromStart+1, anotherPos.chromEnd);
	    webPrintLinkTableNewRow();
	    webPrintLinkCell(s);
	    }
	}
    webPrintLinkTableEnd();
    sqlFreeResult(&sr);
    freeHash(&hash);
    }    

}
