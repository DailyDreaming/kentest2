/* wigTrack - stuff to handle loading and display of
 * wig type tracks in browser. Wigs are arbitrary data graphs
 */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "obscure.h"
#include "hash.h"
#include "linefile.h"
#include "jksql.h"
#include "hdb.h"
#include "hgTracks.h"
#include "wiggle.h"
#include "hmmstats.h"
#include "scoredRef.h"
#ifndef GBROWSE
#include "customTrack.h"
#endif /* GBROWSE */
#include "wigCommon.h"
#include "imageV2.h"
#include "memgfx.h"
#include "udc.h"
#include "trashDir.h"
#include "jsonWrite.h"
#include "dnaMotif.h"

struct wigItem
/* A wig track item. */
    {
    struct wigItem *next;
    int start, end;	/* Start/end in chrom (aka browser) coordinates. */
    char *db;		/* Database */
    int ix;		/* Position in list. */
    int height;		/* Pixel height of item. */
    unsigned span;      /* each value spans this many bases */
    unsigned count;     /* number of values to use */
    unsigned offset;    /* offset in File to fetch data */
    char *file; /* path name to data file, one byte per value */
    double lowerLimit;  /* lowest data value in this block */
    double dataRange;   /* lowerLimit + dataRange = upperLimit */
    unsigned validCount;        /* number of valid data values in this block */
    double sumData;     /* sum of the data points, for average and stddev calc */
    double sumSquares;      /* sum of data points squared, for stddev calc */
    double graphUpperLimit;	/* filled in by DrawItems	*/
    double graphLowerLimit;	/* filled in by DrawItems	*/
    };

static void wigFillInColorLfArray(struct track *wigTrack, Color *colArray, int colSize,
				  struct track *colorTrack)
/* Fill in a color array with the linkedFeatures based colorTrack's
   color where it would normally have an exon. */
{
struct linkedFeatures *lf = NULL, *lfList = colorTrack->items;
struct simpleFeature *sf = NULL;
double scale = scaleForPixels(colSize);
int x1 = 0, x2 = 0;
int i = 0;
for(lf = lfList; lf != NULL; lf = lf->next)
    {
    for (sf = lf->components; sf != NULL; sf = sf->next)
	{
	x1 = round((double)((int)sf->start-winStart)*scale);
	x2 = round((double)((int)sf->end-winStart)*scale);
	if(x1 < 0)
	    x1 = 0;
	if(x2 > colSize)
	    x2 = colSize;
	if(x1 == x2)
	    x2++;
	for(i = x1; i < x2; i++)
	    colArray[i] = colorTrack->ixAltColor;
	}
    }
}

static void wigFillInColorBedArray(struct track *wigTrack, Color *colArray, int colSize,
				  struct track *colorTrack, struct hvGfx *hvg)
/* Fill in a color array with the simple bed based colorTrack's
   color where it would normally have an block. */
{
struct bed *bed = NULL, *bedList = colorTrack->items;
double scale = scaleForPixels(colSize);
int x1 = 0, x2 = 0;
int i = 0;
for (bed = bedList; bed != NULL; bed = bed->next)
    {
    x1 = round((double)((int)bed->chromStart-winStart)*scale);
    x2 = round((double)((int)bed->chromEnd-winStart)*scale);
    if(x1 < 0)
	x1 = 0;
    if(x2 > colSize)
	x2 = colSize;
    if(x1 == x2)
	x2++;
    for(i = x1; i < x2 && i < colSize; i++)
	{
	if(colorTrack->itemColor != NULL)
	    colArray[i] = colorTrack->itemColor(colorTrack, bed, hvg);
	else
	    colArray[i] = colorTrack->ixColor;
        }
    }
}

void wigFillInColorArray(struct track *wigTrack, struct hvGfx *hvg,
                         Color *colorArray, int colSize, struct track *colorTrack)
/* Fill in a color array with the colorTrack's color where
   it would normally have an exon. */
{
boolean trackLoaded = FALSE;
/* If the track is hidden currently, load the items. */
if(colorTrack->limitedVis == tvHide)
    {
    trackLoaded = TRUE;
    colorTrack->loadItems(colorTrack);
    colorTrack->ixColor = hvGfxFindRgb(hvg, &colorTrack->color);
    colorTrack->ixAltColor = hvGfxFindRgb(hvg, &colorTrack->altColor);
    }

if(colorTrack->drawItemAt == linkedFeaturesDrawAt)
    wigFillInColorLfArray(wigTrack, colorArray, colSize, colorTrack);
else if(colorTrack->drawItemAt == bedDrawSimpleAt)
    wigFillInColorBedArray(wigTrack, colorArray, colSize, colorTrack, hvg);

if(trackLoaded && colorTrack->freeItems != NULL)
    colorTrack->freeItems(colorTrack);
}

void wigSetCart(struct track *track, char *dataID, void *dataValue)
/*	set one of the variables in the wigCart.  Actually just MIN_Y or MAX_Y	*/
{
struct wigCartOptions *wigCart;
wigCart = (struct wigCartOptions *) track->wigCartData;

if (sameWord(dataID, MIN_Y))
    wigCart->minY = *((double *)dataValue);
else if (sameWord(dataID, MAX_Y))
    wigCart->maxY = *((double *)dataValue);
else
    internalErr();
}

/*	these two routines unused at this time	*/
#if defined(NOT)
static void wigItemFree(struct wigItem **pEl)
    /* Free up a wigItem. */
{
struct wigItem *el = *pEl;
if (el != NULL)
    {
    /* freeMem(el->name);	DO NOT - this belongs to tg->mapName */
    freeMem(el->db);
    freeMem(el->file);
    freez(pEl);
    }
}

static void wigItemFreeList(struct wigItem **pList)
    /* Free a list of dynamically allocated wigItem's */
{
struct wigItem *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    wigItemFree(&el);
    }
*pList = NULL;
}
#endif

/*	trackSpans - hash of hashes, first hash is keyed via trackName
 *	the value for key trackName is a hash itself where each element
 *	is a Span found in the data (==zoom level indication)
 *	The key for the second hash is the ascii string for the span
 *	value, and the value at that key is the binary equivalent for
 *	that number.
 */
static struct hash *trackSpans = NULL;	/* hash of hashes */

/*	The item names have been massaged during the Load.  An
 *	individual item may have been read in on multiple table rows and
 *	had an extension on it to make it unique from the others.  Also,
 *	each different zoom level had a different extension name.
 *	All these names were condensed into the root of the name with
 *	the extensions removed.
 */
char *wigNameCallback(struct track *tg, void *item)
/* Return name of wig level track. */
{
return tg->track;
}

/*	NOT used at this time, maybe later	*/
#if defined(NOT)
/*	This is practically identical to sampleUpdateY in sampleTracks.c
 *	In fact is is functionally identical except jkLib functions are
 *	used instead of the actual string functions.  I will consult
 *	with Ryan to see if one of these copies can be removed.
 */
static boolean sameWigGroup(char *name, char *nextName, int lineHeight)
/* Only increment height when name root (extension removed)
 * is different from previous one.  Assumes entries are sorted by name.
 */
{
int different = 0;
char *s0;
char *s1;
s0 = cloneString(name);
s1 = cloneString(nextName);
chopSuffix(s0);
chopSuffix(s1);
different = differentString(s0,s1);
freeMem(s0);
freeMem(s1);
if (different)
    return lineHeight;
else
    return 0;
}
#endif

#define FONT_HEIGHT       (tl.fontHeight)
#define WIG_DENSE_HEIGHT  FONT_HEIGHT
#define WIG_PACK_HEIGHT   FONT_HEIGHT
#define WIG_SQUISH_HEIGHT (((FONT_HEIGHT/2) & 1) ? (FONT_HEIGHT/2) : (FONT_HEIGHT/2) - 1)


int wigTotalHeight(struct track *tg, enum trackVisibility vis)
/* Wiggle track will use this to figure out the height they use
   as defined in the cart */
{
struct wigCartOptions *wigCart;
int saveHeight = tg->height;

wigCart = (struct wigCartOptions *) tg->wigCartData;

/*
 *      A track is just one
 *      item, so there is nothing to do here, either it is the tvFull
 *	height as chosen by the user from TrackUi, or it is the dense
 *	mode.
 *  ADDENDUM: wiggle options for squish and pack are being added.
 */
/*	Wiggle tracks depend upon clipping.  They are reporting
 *	totalHeight artifically high by 1 so this will leave a
 *	blank area one pixel high below the track.  hgTracks will set
 *	our clipping rectangle one less than what we report here to get
 *	this accomplished.  In the meantime our actual drawing height is
 *	recorded properly in lineHeight, heightPer and height
 */
if (vis == tvDense)
    tg->lineHeight = WIG_DENSE_HEIGHT;
else if (vis == tvPack)
    tg->lineHeight = WIG_PACK_HEIGHT;
else if (vis == tvSquish)
    tg->lineHeight = WIG_SQUISH_HEIGHT;
else if (vis == tvFull)
    tg->lineHeight = max(wigCart->minHeight, wigCart->defaultHeight);

tg->heightPer = tg->lineHeight;
tg->height = tg->lineHeight;

if (saveHeight == tg->height + 1)
    {
    tg->height = saveHeight;
    return tg->height;
    }
else
    return tg->height + 1;
}

static void wigSetItemData(struct track *tg, struct wigItem *wi,
    struct wiggle *wiggle, struct hash *spans)
/* copy values from *wiggle to *wi, maintain trackSpans hash	*/
{
static char *previousFileName = (char *)NULL;
char spanName[SMALLBUF];
struct hashEl *el;
char *trackName = tg->track;

/*	Allocate trackSpans one time only, for all tracks	*/
if (! trackSpans)
    trackSpans = newHash(4);

wi->start = wiggle->chromStart;
wi->end = wiggle->chromEnd;

if ((previousFileName == (char *)NULL) ||
	differentString(previousFileName,wiggle->file))
    {
    freez(&previousFileName);
    previousFileName = cloneString(wiggle->file);
    }
wi->file = cloneString(wiggle->file);

wi->span = wiggle->span;
wi->count = wiggle->count;
wi->offset = wiggle->offset;
wi->lowerLimit = wiggle->lowerLimit;
wi->dataRange = wiggle->dataRange;
wi->validCount = wiggle->validCount;
wi->sumData = wiggle->sumData;
wi->sumSquares = wiggle->sumSquares;

/*	see if we have a spans hash for this track already */
el = hashLookup(trackSpans, trackName);
/*	no, then let's start one	*/
if ( el == NULL)
	hashAdd(trackSpans, trackName, spans);
/*	see if this span is already in our hash for this track */
safef(spanName, sizeof(spanName), "%d", wi->span);
el = hashLookup(spans, spanName);
/*	no, then add this span to the spans list for this track */
if ( el == NULL)
	hashAddInt(spans, spanName, wi->span);
}

#ifndef GBROWSE
void ctWigLoadItems(struct track *tg)
/*	load custom wiggle track data	*/
{
struct customTrack *ct;
char *row[13];
struct lineFile *lf = NULL;
struct wiggle wiggle;
struct wigItem *wiList = NULL;
int itemsLoaded = 0;
struct hash *spans = NULL;	/* Spans encountered during load */

ct = tg->customPt;

/*	Verify this is a custom track	*/
if (ct == (void *)NULL)
    errAbort("ctWigLoadItems: did not find a custom wiggle track: %s", tg->track);

/*	and should *not* be here for a database custom track	*/
if (ct->dbTrack)
    errAbort("ctWigLoadItems: this custom wiggle track is in database: %s", tg->track);

/*	Each instance of this LoadItems will create a new spans hash
 *	It will be the value included in the trackSpans hash
 */
spans = newHash(3);
/*	Each row read will be turned into an instance of a wigItem
 *	A growing list of wigItems will be the items list to return
 */
itemsLoaded = 0;

tg->items = wiList;

lf = lineFileOpen(ct->wigFile, TRUE);
while (lineFileChopNextTab(lf, row, ArraySize(row)))
    {
    wiggleStaticLoad(row, &wiggle);
    /*  we have to do hRangeQuery's job here since we are reading a
     *  file.  We need to be on the correct chromosome, and the data
     *	needs to be in the current view.
     */
    if (sameWord(chromName,wiggle.chrom))
	{
	if ((winStart < wiggle.chromEnd) && (winEnd > wiggle.chromStart))
	    {
	    struct wigItem *wi;
	    ++itemsLoaded;
	    AllocVar(wi);
	    wigSetItemData(tg, wi, &wiggle, spans);
	    slAddHead(&wiList, wi);
	    }	/*	if in viewing window	*/
	}	/*	if in same chromosome	*/
    }	/*	while reading lines	*/

slReverse(&wiList);
tg->items = wiList;
tg->mapsSelf = TRUE;

lineFileClose(&lf);
}
#endif /* GBROWSE */

void wigLoadItems(struct track *tg)
/*      wigLoadItems - read the table rows that hRangeQuery returns
 *      With appropriate adjustment to help hRangeQuery limit its
 *	result to specific "Span" based on the basesPerPixel.
 *	From the rows returned, turn each one into a wigItem, add it to
 *	the growing wiList, and return that wiList as the tg->items.
 *
 *	To help DrawItems, we are going to make up a list of Spans that
 *	were read in for this track.  DrawItems can use that list to
 *	limit itself to the appropriate span.  (Even though we tried to
 *	use Span to limit the hRangeQuery, there may be other Spans in
 *	the result that are not needed during the drawing.)
 *	This list of spans is actually a hash of hashes.
 *	The first level is a hash of track names from each call to
 *	wigLoadItems.  The second level chosen from the track name is
 *	the actual hash of Spans for this particular track.
 *
 *	With 1K zoom Spans available, no more than approximately 1024
 *	rows will need to be loaded at any one time.
 */
{
struct sqlConnection *conn = NULL ;

// if this is a custom track we don't need an SQL connection to the database
if (!isCustomTrack(tg->table))
    conn = hAllocConn(database);
struct sqlResult *sr;
char **row;
int rowOffset;
struct wiggle wiggle;
struct wigItem *wiList = NULL;
char *whereNULL = NULL;
int itemsLoaded = 0;
struct hash *spans = NULL;	/* Spans encountered during load */
/*	Check our scale from the global variables that exist in
 *	hgTracks.c - This can give us a guide about which rows to load.
 *	If the scale is more than 1000 bases per pixel, we can try loading
 *	only those rows with Span >= 1000 to see if an appropriate zoom
 *	level exists.
 */
int basesPerPixel = (int)((double)(winEnd - winStart)/(double)insideWidth);
char *span1K = "Span >= 1000 limit 1";
char *spanOver1K = "Span >= 1000";
char whereSpan[SMALLBUF];
int spanMinimum = 1;
char *dbTableName = NULL;
struct trackDb *tdb = NULL;
int loadStart = winStart, loadEnd = winEnd;

#ifndef GBROWSE
struct customTrack *ct = NULL;
/*	custom tracks have different database	*/
if (isCustomTrack(tg->table) && tg->customPt)
    {
    hFreeConn(&conn);
    conn = hAllocConn(CUSTOM_TRASH);
    ct = tg->customPt;
    dbTableName = ct->dbTableName;
    tdb = ct->tdb;
    }
else if (isCustomTrack(tg->table) )
    {
    // we can get custom tracks through track hubs that don't have customPt
    hFreeConn(&conn);
    conn = hAllocConn(CUSTOM_TRASH);
    dbTableName = trackDbSetting(tg->tdb, "dbTableName");
    tdb = tg->tdb;
    }
else
#endif /* GBROWSE */
    {
    dbTableName = tg->table;
    tdb = tg->tdb;
    }

/*	Allocate trackSpans one time only	*/
if (! trackSpans)
    trackSpans = newHash(0);

/*	find the minimum span to see if there are actually any data
 *	points in this area at that span.  If there are not, then there
 *	is no data here even if a zoomed view covers this section.
 *	protect against less than 1 with the max(1,minSpan());
 *	This business will fix the problem mentioned in RT #1186
 */

spanMinimum = max(1,
	minSpan(conn, dbTableName, chromName, winStart, winEnd, cart, tdb));

itemsLoaded = 0;
sqlSafefFrag(whereSpan, sizeof(whereSpan), "span=%d limit 1", spanMinimum);

sr = hRangeQuery(conn, dbTableName, chromName, loadStart, loadEnd,
    whereSpan, &rowOffset);

while ((row = sqlNextRow(sr)) != NULL)
    ++itemsLoaded;
sqlFreeResult(&sr);

/*	if nothing here, bail out	*/
if (itemsLoaded < 1)
    {
    tg->items = (struct wigItem *)NULL;
    hFreeConn(&conn);
    return;
    }

itemsLoaded = 0;
if (basesPerPixel >= 1000)
    {
    sr = hRangeQuery(conn, dbTableName, chromName, loadStart, loadEnd,
	span1K, &rowOffset);
    while ((row = sqlNextRow(sr)) != NULL)
	    ++itemsLoaded;
    sqlFreeResult(&sr);
    }
/*	If that worked, excellent, then we have at least another zoom level
 *	So, for our actual load, use spanOver1K to fetch not only the 1K
 *	zooms, but potentially others that may be useful.  This will
 *	save us a huge amount in loaded rows.  On a 250 Mbase chromosome
 *	there would be 256,000 rows at the 1 base level and only
 *	256 rows at the 1K zoom level.  Otherwise, we go back to the
 *	regular query which will give us all rows.
 */
/* JK - Can't we figure out here one, and only one span to load?  This
 * would simplify drawing logic. */
if (itemsLoaded)
    {
    sr = hRangeQuery(conn, dbTableName, chromName, loadStart, loadEnd,
	     spanOver1K, &rowOffset);
    }
else
    {
    sr = hRangeQuery(conn, dbTableName, chromName, loadStart, loadEnd,
	whereNULL, &rowOffset);
    }

/*	Allocate trackSpans one time only	*/
if (! trackSpans)
    trackSpans = newHash(4);
/*	Each instance of this LoadItems will create a new spans hash
 *	It will be the value included in the trackSpans hash
 */
spans = newHash(4);
/*	Each row read will be turned into an instance of a wigItem
 *	A growing list of wigItems will be the items list to return
 */
itemsLoaded = 0;
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct wigItem *wi;
    ++itemsLoaded;
    wiggleStaticLoad(row + rowOffset, &wiggle);
    AllocVar(wi);
    wigSetItemData(tg, wi, &wiggle, spans);
    slAddHead(&wiList, wi);
    }

sqlFreeResult(&sr);
hFreeConn(&conn);

slReverse(&wiList);
tg->items = wiList;
}	/*	wigLoadItems()	*/

static void wigFreeItems(struct track *tg) {
#if defined(DEBUG)
safef(dbgMsg, DBGMSGSZ, "I haven't seen wigFreeItems ever called ?");
wigDebugPrint("wigFreeItems");
#endif
}

struct preDrawContainer *initPreDrawContainer(int width)
/* Initialize preDraw of given size */
{
struct preDrawContainer *pre;
AllocVar(pre);
int size = pre->preDrawSize = width + 2*wiggleSmoothingMax;	
pre->preDrawZero = wiggleSmoothingMax;
pre->width = width;
struct preDrawElement *preDraw = AllocArray(pre->preDraw, pre->preDrawSize);
int i;
for (i = 0; i < size; ++i)
    {
    preDraw[i].count = 0;
    preDraw[i].max = wigEncodeStartingUpperLimit;
    preDraw[i].min = wigEncodeStartingLowerLimit;
    }
return pre;
}

double wiggleLogish(double x)
/* Return log-like transform without singularity at 0. */
{
if (x >= 0)
    return log(1+x);
else
    return -log(1-x);
}

static double doTransform(double x, enum wiggleTransformFuncEnum transformFunc)
/* Do log-type transformation if asked. */
{
if (transformFunc == wiggleTransformFuncLog)
    {
    x = wiggleLogish(x);
    }
return x;
}


void preDrawWindowFunction(struct preDrawElement *preDraw, int preDrawSize,
	enum wiggleWindowingEnum windowingFunction,
	enum wiggleTransformFuncEnum transformFunc,
	boolean doNegative)
/*	apply windowing function to the values in preDraw array	*/
{
int i;

/*	Determine the raw plotting value	*/
for (i = 0; i < preDrawSize; ++i)
    {
    double dataValue;
    if (preDraw[i].count)
	{
	switch (windowingFunction)
	    {
	    case wiggleWindowingMin:
		if (fabs(preDraw[i].min)
				< fabs(preDraw[i].max))
		    dataValue = preDraw[i].min;
                else
		    dataValue = preDraw[i].max;
		break;
	    case wiggleWindowingMean:
	    case wiggleWindowingWhiskers:
		dataValue =
		    preDraw[i].sumData / preDraw[i].count;
		break;
	    default:
	    case wiggleWindowingMax:
		if (fabs(preDraw[i].min)
			> fabs(preDraw[i].max))
		    dataValue = preDraw[i].min;
                else
		    dataValue = preDraw[i].max;
		break;
	    }

	dataValue = doTransform(dataValue, transformFunc);
	if (doNegative)
	    {
	    dataValue = -dataValue;
	    int swap = preDraw[i].min;
	    preDraw[i].min = -preDraw[i].max;
	    preDraw[i].max = -swap;
	    }
	preDraw[i].plotValue = dataValue;
	preDraw[i].smooth = dataValue;
	}
    }
}

void preDrawSmoothing(struct preDrawElement *preDraw, int preDrawSize,
    enum wiggleSmoothingEnum smoothingWindow)
/*      apply smoothing function to preDraw array       */
{
/*      Are we perhaps doing smoothing ?  smoothingWindow is 1 off due
 *      to enum funny business in inc/hui.h and lib/hui.c       */
if (smoothingWindow > 0)
    {
    int winSize = smoothingWindow + 1; /* enum funny business */
    int winBegin = 0;
    int winMiddle = -(winSize/2);
    int winEnd = -winSize;
    double sum = 0.0;
    unsigned long long points = 0LL;

    for (winBegin = 0; winBegin < preDrawSize; ++winBegin)
	{
	if (winEnd >=0)
	    {
	    if (preDraw[winEnd].count)
		{
		points -= preDraw[winEnd].count;
		sum -= preDraw[winEnd].plotValue * preDraw[winEnd].count;
		}
	    }
	if (preDraw[winBegin].count)
	    {
	    points += preDraw[winBegin].count;
	    sum += preDraw[winBegin].plotValue * preDraw[winBegin].count;
	    }
	if ((winMiddle >= 0) && points && preDraw[winMiddle].count)
		preDraw[winMiddle].smooth = sum / points;
	++winEnd;
	++winMiddle;
	}
    }
}

double preDrawAutoScale(struct preDrawElement *preDraw, int preDrawZero,
    int width, enum wiggleScaleOptEnum autoScale,
    enum wiggleWindowingEnum windowingFunction,
    double *graphUpperLimit, double *graphLowerLimit,
    double *epsilon, int lineHeight,
    double maxY, double minY, enum wiggleAlwaysZeroEnum alwaysZero)
/*	if autoScaling, scan preDraw array and determine limits */
{
if ((autoScale == wiggleScaleAuto) || (autoScale == wiggleScaleCumulative))
    {
    double overallUpperLimit = wigEncodeStartingUpperLimit;
    double overallLowerLimit = wigEncodeStartingLowerLimit;
    int i, lastI = preDrawZero+width;

    /* reset limits for auto scale */
    for (i = preDrawZero; i < lastI; ++i)
	{
	/*	count is non-zero meaning valid data exists here	*/
	if (preDraw[i].count)
	    {
	    double val =  preDraw[i].smooth;
	    if (windowingFunction ==  wiggleWindowingWhiskers)
		val =  preDraw[i].max;
	    if (val > overallUpperLimit)
		overallUpperLimit = val;

	    if (windowingFunction ==  wiggleWindowingWhiskers)
		val =  preDraw[i].min;
	    if (val < overallLowerLimit)
		overallLowerLimit = val;
	    }
	}
    if (alwaysZero == wiggleAlwaysZeroOn)
	{
	if ( overallUpperLimit < 0)
	    overallUpperLimit = 0.0;
	else if ( overallLowerLimit > 0)
	    overallLowerLimit = 0.0;
	}
    double overallRange = overallUpperLimit - overallLowerLimit;
    if (overallRange == 0.0)
	{
	if (overallUpperLimit > 0.0)
            {
            *graphUpperLimit = overallUpperLimit;
            *graphLowerLimit = 0.0;
            }
        else if (overallUpperLimit < 0.0)
            {
            *graphUpperLimit = 0.0;
            *graphLowerLimit = overallUpperLimit;
            }
        else
            {
            *graphUpperLimit = 1.0;
            *graphLowerLimit = -1.0;
            }
        }
    else
        {
        *graphUpperLimit = overallUpperLimit;
        *graphLowerLimit = overallLowerLimit;
        }
    }
else
    {
    *graphUpperLimit = maxY;
    *graphLowerLimit = minY;
    }

double graphRange = *graphUpperLimit - *graphLowerLimit;
*epsilon = graphRange / lineHeight;
return(graphRange);
}

static Color * makeColorArray(struct preDrawElement *preDraw, int width,
    int preDrawZero, struct wigCartOptions *wigCart, struct track *tg, struct hvGfx *hvg)
/*	allocate and fill in a coloring array based on another track */
{
char *colorTrack = wigCart->colorTrack;
int x1;
Color *colorArray = NULL;       /*      Array of pixels to be drawn.    */

/*      Set up the color by array. Determine color of each pixel
 *      based initially on the sign of the data point. If a colorTrack
 *      is specified also fill in the color array with that.
 */
AllocArray(colorArray, width);
for(x1 = 0; x1 < width; ++x1)
    {
    int preDrawIndex = x1 + preDrawZero;
    if (preDraw[preDrawIndex].count)
	{
	double dataValue;	/*	the data value in data space	*/

	dataValue = preDraw[preDrawIndex].smooth;
	/*	negative data is the alternate color	*/
	if (dataValue < 0.0)
	    colorArray[x1] = tg->ixAltColor;
	else
	    colorArray[x1] = tg->ixColor;
	}
    }

/* Fill in colors from alternate track if necessary. */
if (colorTrack != NULL)
    {
    struct track *cTrack = hashFindVal(trackHash, colorTrack);
    if (cTrack == NULL) // rightClick update of wigColorBy track may not have colorTrack in hash
        {               // so create it on the fly
        struct trackDb *tdb = hTrackDbForTrack(database,colorTrack);
        if (tdb != NULL)
            cTrack = trackFromTrackDb(tdb);
        }
    if (cTrack != NULL)
        wigFillInColorArray(tg, hvg, colorArray, width, cTrack);
    }

return colorArray;
}

void vLineViaHvg(void *image, int x, int y, int height, Color color)
/* A vertical line drawer that works via hvGfx system. */
{
hvGfxBox(image, x, y, 1, height, color);
}

Color somewhatLighterColor32(Color color)
/* Get a somewhat lighter shade of a color - 1/3 of the way towards white.
 * Specialized here to bypass image parameter requirement.*/
{
struct rgbColor rgbColor =  mgColorIxToRgb(NULL, color);
rgbColor.r = (2*rgbColor.r+255)/3;
rgbColor.g = (2*rgbColor.g+255)/3;
rgbColor.b = (2*rgbColor.b+255)/3;
return MAKECOLOR_32(rgbColor.r, rgbColor.g, rgbColor.b);
}

struct wigGraphOutput *wigGraphOutputStack(int xOff, int yOff,  int width, int numTracks, struct hvGfx *image)
/* Get appropriate wigGraphOutput for non-transparent stacked rendering */
{
struct wigGraphOutput *wgo;
AllocVar(wgo);
wgo->image = image;
wgo->vLine = vLineViaHvg;
wgo->xOff = xOff;
wgo->yOff = yOff;
if (numTracks)
    wgo->yOffsets = needHugeMem(width * numTracks * sizeof(double));
return wgo;
}

struct wigGraphOutput *wigGraphOutputSolid(int xOff, int yOff, struct hvGfx *image)
/* Get appropriate wigGraphOutput for non-transparent rendering */
{
struct wigGraphOutput *wgo;
AllocVar(wgo);
wgo->image = image;
wgo->vLine = vLineViaHvg;
wgo->xOff = xOff;
wgo->yOff = yOff;
return wgo;
}

struct wigMouseOver *getMouseOverData(struct track *tg, struct preDrawElement *preDraw, int width, int xOff, int preDrawZero)
/* Calculate the mouseOver data. */
{
int x1;
boolean skipMouseOvers = TRUE;	/* assuming not using */
boolean dropMouseOverData = FALSE;	// will become TRUE if noAverage
int mouseOverX2 = -1;
struct wigMouseOver *mouseOverData = NULL;
double previousValue = 0;
char *mouseOverFunction = trackDbSetting(tg->tdb, "mouseOverFunction");
boolean noAverage = FALSE;
if (sameOk(mouseOverFunction, "noAverage"))
    noAverage = TRUE;

if (enableMouseOver)
    skipMouseOvers = FALSE;
					// condition is encountered
/* ===== mouseOver calculations===== */
for (x1 = 0; x1 < width; ++x1)
    {
    //int x = x1 + xOff;
    int preDrawIndex = x1 + preDrawZero;
    struct preDrawElement *p = &preDraw[preDrawIndex];
    if (enableMouseOver && !dropMouseOverData)
        {
        /* checking if mouseOver construction is allowed */
        if (!skipMouseOvers && (p->count > 0) && !(noAverage && p->count>1))
            {
            if (p->count > 0)	/* allow any number of values to display */
                {
                double thisValue = p->sumData/p->count;	/*average if count > 1*/
                if (mouseOverX2 < 0)    /* first valid data found */
                    {
                    struct wigMouseOver *dataItem;
                    AllocVar(dataItem);
                    mouseOverX2 = x1+1;
                    dataItem->x1 = x1;
                    dataItem->x2 = mouseOverX2;
                    dataItem->value = thisValue;
                    dataItem->valueCount = p->count;
                    slAddHead(&mouseOverData, dataItem);
                    previousValue = thisValue;
                    }
                else	/* see if we need a new item */
                    {
#define epsilonLimit 1.0e-6
                    if (fabs(thisValue - previousValue) > epsilonLimit)
                        {
                        /* finish off the existing run of data (list head)*/
                        mouseOverData->x2 = mouseOverX2;
                        mouseOverX2 = x1+1;
                        struct wigMouseOver *dataItem;
                        AllocVar(dataItem);
                        dataItem->x1 = x1;
                        dataItem->x2 = mouseOverX2;
                        dataItem->value = thisValue;
                        dataItem->valueCount = p->count;
                        slAddHead(&mouseOverData, dataItem);
                        previousValue = thisValue;
                        }
                    else	/* continue run of same data value */
                        mouseOverX2 = x1+1;
                    }
                }
            else
                skipMouseOvers = TRUE;	/* has become too dense to make sense */
            }
        else /* perhaps entered region without values after some data already */
            {

            if (noAverage && p->count>1)
              dropMouseOverData = TRUE;
            else if (mouseOverX2 > 0)	/* yes, been in data, end it here */
                {
                mouseOverData->x2 = mouseOverX2;
                mouseOverX2 = -1;	/* start over with new data when found*/
                }
            }
        /* potentially end the last mouseOver box */
        if (mouseOverX2 > 0 && mouseOverX2 > mouseOverData->x2)
                mouseOverData->x2 = mouseOverX2;

        }       //      if (enableMouseOver)
else
    skipMouseOvers = TRUE;
    }
if (dropMouseOverData)
    slFreeList(&mouseOverData);

return mouseOverData;
}

struct wigMouseOver *graphPreDraw(struct preDrawElement *preDraw, int preDrawZero, int width,
    struct track *tg, void *image, WigVerticalLineVirtual vLine, int xOff, int yOff, double *yOffsets, int numTrack,
    double graphUpperLimit, double graphLowerLimit, double graphRange,
    double epsilon, Color *colorArray, enum trackVisibility vis,
    struct wigCartOptions *wigCart, struct pixelCountBin *pixelBins)
/*	graph the preDraw array, returns mouse over data, or NULL */
{
int x1;
int h = tg->lineHeight;	/*	the height of our drawing window */
double scaleFactor = h/graphRange;
Color oldDrawColor = colorArray[0] + 1;	/* Just to be different from 1st drawColor. */
Color mediumColor = MG_BLACK;	// Will be overriden
Color lightColor = MG_BLACK;	// Will be overriden
Color clipColor = MG_MAGENTA;
enum wiggleTransformFuncEnum transformFunc = wigCart->transformFunc;
enum wiggleGraphOptEnum lineBar = wigCart->lineBar;
boolean whiskers = (wigCart->windowingFunction == wiggleWindowingWhiskers
			&& width < winEnd-winStart);
	/* list of mouse over data, if created here */

struct wigMouseOver *mouseOverData = getMouseOverData(tg, preDraw, width, xOff, preDrawZero);

/*	right now this is a simple pixel by pixel loop.  Future
 *	enhancements could draw boxes where pixels
 *	are all the same height in a run.
 */
for (x1 = 0; x1 < width; ++x1)
    {
    int x = x1 + xOff;
    int preDrawIndex = x1 + preDrawZero;
    struct preDrawElement *p = &preDraw[preDrawIndex];

    assert(x1/pixelBins->binSize < pixelBins->binCount);
    unsigned long *bitCount = &pixelBins->bins[x1/pixelBins->binSize];

    Color drawColor = colorArray[x1];
    if (drawColor != oldDrawColor)
        {
	mediumColor = somewhatLighterColor32(drawColor);
	lightColor = somewhatLighterColor32(mediumColor);
	oldDrawColor = drawColor;
        }

    /*	count is non-zero meaning valid data exists here	*/
    if (p->count)
	{
	/*	data value has been picked by previous scanning.
	 *	Could be smoothed, maybe not.
	 */
	double dataValue = p->smooth;

        /* save a number that represents how many pixels that would be set if we were drawing bars.
         * This may used for sorting later on */
        int iy0 = graphUpperLimit * scaleFactor;
        int iy1 = (graphUpperLimit - dataValue)*scaleFactor;
        int boxHeight = max(1,abs(iy1 - iy0));
        *bitCount += boxHeight;


	/*	The graphing coordinate conversion situation is:
	 *	graph coordinate y = 0 is graphUpperLimit data space
	 *	and total graph height is h which is graphRange in data space
	 *	The Y axis is positive down, negative up.
	 *
	 *	Taking a simple coordinate conversion from data space
	 *	to the graphing space, the data value is at:
	 *	h * ((graphUpperLimit - dataValue)/graphRange)
	 *	and a data value zero line is at:
	 *	h * (graphUpperLimit/graphRange)
	 *	These may end up to be negative meaning they are above
	 *	the upper graphing limit, or be very large, meaning they
         *      are below the lower graphing limit.  This is OK, the
         *      clipping will be taken care of by the vgBox() function.
         */

        if (vis == tvFull || vis == tvPack)
            {
#define scaleHeightToPixels(val) (min(BIGNUM,(scaleFactor * (graphUpperLimit - (val)) + yOff)))
#define doLine(image, x, y, height, color) {vLine(image, x, y, height, color); }
	    if (lineBar == wiggleGraphBar)
		{
		if (whiskers)
		    {
		    int zeroPos = max(0,scaleHeightToPixels(0));
		    int scaledVal = scaleHeightToPixels(dataValue);
		    double std = calcStdFromSums(p->sumData, p->sumSquares, p->count);
		    double mean = p->sumData/p->count;

		    if (dataValue < 0)
		        {
			int scaledMin = scaleHeightToPixels(doTransform(p->min, transformFunc));
			int lightHeight = max(1,scaledMin-zeroPos);
			int mediumHeight = lightHeight;
			if (!isnan(std))
			    { // Test needed due to bug in version 1.5 bigWiles
			    double minus = doTransform(mean - std, transformFunc);
			    int scaledMinus = scaleHeightToPixels(minus);
			    mediumHeight = max(1,scaledMinus-zeroPos);
			    }
			int darkHeight = max(1,scaledVal-zeroPos);
			if (zeroPos == (yOff+h))  // bottom pixel special case
			    zeroPos -= 1;
		        if (((zeroPos-yOff)+darkHeight) == 0)
			    darkHeight += 1;	  // top pixel special case
			doLine(image, x,zeroPos, darkHeight, drawColor);
                        doLine(image, x, zeroPos+darkHeight, mediumHeight-darkHeight,
				mediumColor);
                        doLine(image, x, zeroPos+mediumHeight, lightHeight-mediumHeight,
				lightColor);
			}
		    else
		        {
                        /* The calculations here are a little convoluted because
                         * of the history.  Originally it drew from the baseline
                         * up to the max first in the lightest color, then from the
                         * baseline to the mean+std in medium color, and finally
                         * from baseline to mean in dark color.  This ended up
                         * drawing the same pixels up to three times which messed
                         * things up in transparent overlay mode.   The code was
                         * refactored to accomplish this without having to worry
                         * about +/- 1 differences.   In particular be aware the
                         * xyzHeight calculations are done assuming the other end is
                         * the baseline. */

                        /* Calculate dark part from smoothed mean. */
                        int boxHeight = max(1,zeroPos - scaledVal);
                        if (scaledVal == (yOff+h))  // bottom pixel special case
			    scaledVal -= 1;
		        if (((scaledVal-yOff)+boxHeight) == 0)
			    boxHeight += 1;	    // top pixel special case
			int darkTop = scaledVal, darkHeight = boxHeight;

			/* Calculate medium part from smoothed mean + std */
			int mediumTop = darkTop, mediumHeight = darkHeight;
			if (!isnan(std))
			    { // Test needed due to bug in version 1.5 bigWiles
			    double plus = doTransform(mean + std, transformFunc);
			    int scaledPlus = scaleHeightToPixels(plus);
			    int boxHeight = max(1,zeroPos-scaledPlus);
			    mediumTop = scaledPlus, mediumHeight = boxHeight;
			    }

			/* Calculate light part from max. */
			int scaledMax = scaleHeightToPixels(doTransform(p->max, transformFunc));
			if (scaledMax == (h+yOff))
                            scaledMax = (h+yOff) - 1;
                        boxHeight = max(1,zeroPos-scaledMax);
                        int lightTop = scaledMax, lightHeight = boxHeight;

                        /* Draw, making sure not to overwrite pixels since
                         * would mess up transparent drawing. */
                        doLine(image,x,darkTop, darkHeight, drawColor);
			doLine(image, x, mediumTop, mediumHeight-darkHeight, mediumColor);
			doLine(image,x,lightTop,lightHeight-mediumHeight, lightColor);
			}
		    }
		else
		    {
		    int y0 = graphUpperLimit * scaleFactor;
		    int y1 = (graphUpperLimit - dataValue)*scaleFactor;
		    if (yOffsets)
			{
			if (numTrack > 0)
			    {
			    y0 = (graphUpperLimit  - yOffsets[(numTrack-1) *  width + x1]) *scaleFactor;
			    y1 = (graphUpperLimit - dataValue - yOffsets[(numTrack-1) *  width + x1])*scaleFactor;
			    }
			}

		    int boxHeight = max(1,abs(y1 - y0));
		    int boxTop = min(y1,y0);

		    //	positive data value exactly equal to Bottom pixel
		    //  make sure it draws at least a pixel there
		    if (boxTop == h)
			boxTop = h - 1;

		    // negative data value exactly equal to top pixel
		    // make sure it draws something
		    if (((boxTop+boxHeight) == 0) && !isnan(dataValue))
			boxHeight += 1;
		    doLine(image,x, yOff+boxTop, boxHeight, drawColor);
		    }
		}
	    else
		{	/*	draw a 3 pixel height box	*/
		if (whiskers)
		    {
		    int scaledMin = scaleHeightToPixels(doTransform(p->min, transformFunc));
		    int scaledMax = scaleHeightToPixels(doTransform(p->max, transformFunc));
		    double mean = p->sumData/p->count;
		    int boxHeight = max(1,scaledMin - scaledMax);
		    doLine(image, x, scaledMax, boxHeight, lightColor);
		    int scaledMean = scaleHeightToPixels(dataValue);
		    double std = calcStdFromSums(p->sumData, p->sumSquares, p->count);
		    if (!isnan(std))  // Test needed because of bug in version 1.5 bigWiles
			{
			int scaledPlus = scaleHeightToPixels(doTransform(mean+std, transformFunc));
			int scaledMinus = scaleHeightToPixels(doTransform(mean-std, transformFunc));
			int boxHeight = max(1,scaledMinus - scaledPlus);
			doLine(image, x, scaledPlus, boxHeight, mediumColor);
			}
		    doLine(image, x, scaledMean, 1, drawColor);
		    }
		else
		    {
		    double y0 = dataValue;
		    if ((yOffsets != NULL) && (numTrack > 0))
			y0 += yOffsets[(numTrack-1) *  width + x1];
		    int yPointGraph = scaleHeightToPixels(y0) - 1;
		    doLine(image, x, yPointGraph, 3, drawColor);
		    }
		}
	    double stackValue = dataValue;

	    if ((yOffsets != NULL) && (numTrack > 0))
		stackValue += yOffsets[(numTrack-1) *  width + x1];
	    if (stackValue > graphUpperLimit)
                {
		doLine(image, x, yOff, 2, clipColor);
                }
	    else if (stackValue < graphLowerLimit)
                {
		doLine(image, x, yOff + h - 1, 2, clipColor);
                }
#undef scaleHeightToPixels	/* No longer use this symbol */
            }   /*	vis == tvFull || vis == tvPack */
        else if (vis == tvDense || vis == tvSquish)
	    {
	    double grayValue;
	    int grayIndex;
	    /* honor the viewLimits, data below is white, data above is black */
	    grayValue = max(dataValue,graphLowerLimit);
	    grayValue = min(grayValue,graphUpperLimit);
	    grayIndex = ((grayValue-graphLowerLimit)/graphRange)*MAX_WIG_VALUE;

	    drawColor =
		tg->colorShades[grayInRange(grayIndex, 0, MAX_WIG_VALUE)];
	    doLine(image, x, yOff, tg->lineHeight, drawColor);
            }   /*	vis == tvDense || vis == tvSquish	*/
	}	/*	if (preDraw[].count)	*/
    }	/*	for (x1 = 0; x1 < width; ++x1)	*/

return(mouseOverData);
}	/*	graphPreDraw()	*/

static struct wigMouseOver *logoPreDrawContainer(struct preDrawContainer *preDrawContainer,
    int preDrawZero, int width, struct track *tg, struct hvGfx *hvg,
    int xOff, int yOff, double graphUpperLimit, double graphLowerLimit,
    double graphRange, enum trackVisibility vis, struct wigCartOptions *wigCart, int seqStart, int seqEnd)
{
struct preDrawElement *preDraw = preDrawContainer->preDraw;
struct wigGraphOutput *wgo = tg->wigGraphOutput;
//struct wigMouseOver *mouseOverData = NULL;
unsigned numBases = seqEnd - seqStart;
struct dnaSeq *seq = hChromSeq(database, chromName, seqStart, seqEnd);
struct pixelCountBin *pixelBins = wgo->pixelBins;
double *yOffsets = wgo->yOffsets;
int numTrack = wgo->numTrack;
Color clipColor = MG_MAGENTA;
WigVerticalLineVirtual vLine = wgo->vLine;
void *image = wgo->image;
#define doLine(image, x, y, height, color) {vLine(image, x, y, height, color); }

int h = tg->lineHeight;	/*	the height of our drawing window */
double scaleFactor = h/graphRange;
struct wigMouseOver *mouseOverData = getMouseOverData(tg, preDraw, width, xOff, preDrawZero);

double xIncr = (double)width / numBases;
unsigned baseNum;
int lastX = xOff;
for(baseNum = 0; baseNum < numBases; baseNum++)
    {
    int x1 = baseNum * xIncr;
    int x = x1 + xOff;
    int width = x - lastX;
    int base = seq->dna[baseNum];
    lastX = x;
    int preDrawIndex = x1 + preDrawZero;
    struct preDrawElement *p = &preDraw[preDrawIndex];

    assert(x1/pixelBins->binSize < pixelBins->binCount);
    unsigned long *bitCount = &pixelBins->bins[x1/pixelBins->binSize];

    /*	count is non-zero meaning valid data exists here	*/
    if (p->count)
	{
	/*	data value has been picked by previous scanning.
	 *	Could be smoothed, maybe not.
	 */
	double dataValue = p->smooth;

        /* save a number that represents how many pixels that would be set if we were drawing bars.
         * This may used for sorting later on */
        int iy0 = graphUpperLimit * scaleFactor;
        int iy1 = (graphUpperLimit - dataValue)*scaleFactor;
        int boxHeight = max(1,abs(iy1 - iy0));
        *bitCount += boxHeight;


	/*	The graphing coordinate conversion situation is:
	 *	graph coordinate y = 0 is graphUpperLimit data space
	 *	and total graph height is h which is graphRange in data space
	 *	The Y axis is positive down, negative up.
	 *
	 *	Taking a simple coordinate conversion from data space
	 *	to the graphing space, the data value is at:
	 *	h * ((graphUpperLimit - dataValue)/graphRange)
	 *	and a data value zero line is at:
	 *	h * (graphUpperLimit/graphRange)
	 *	These may end up to be negative meaning they are above
	 *	the upper graphing limit, or be very large, meaning they
         *      are below the lower graphing limit.  This is OK, the
         *      clipping will be taken care of by the vgBox() function.
         */

        if (vis == tvFull || vis == tvPack)
            {
#define scaleHeightToPixels(val) (min(BIGNUM,(scaleFactor * (graphUpperLimit - (val)) + yOff)))
#define doLine(image, x, y, height, color) {vLine(image, x, y, height, color); }
                {
                int y0 = graphUpperLimit * scaleFactor;
                int y1 = (graphUpperLimit - dataValue)*scaleFactor;
                if (yOffsets)
                    {
                    if (numTrack > 0)
                        {
                        y0 = (graphUpperLimit  - yOffsets[(numTrack-1) *  width + x1]) *scaleFactor;
                        y1 = (graphUpperLimit - dataValue - yOffsets[(numTrack-1) *  width + x1])*scaleFactor;
                        }
                    }

                int boxHeight = max(1,abs(y1 - y0));
                int boxTop = min(y1,y0);

                //	positive data value exactly equal to Bottom pixel
                //  make sure it draws at least a pixel there
                if (boxTop == h)
                    boxTop = h - 1;

                char string[2];
                string[0] = toupper(base);
                string[1] = 0;
                MgFont *font = tl.font;
                int height = dataValue * scaleFactor;
                unsigned color = MG_BLACK;
                if (base == 'a')
                    color = MG_RED;
                else if (base == 't')
                    color = MG_GREEN;
                else if (base == 'c')
                    color = MG_BROWN;
                else if (base == 'g')
                    color = MG_BLUE;
                if (abs(dataValue) > 0.1)
                    {
                    if (dataValue < 0)
                        {
                        // useful for debug, fills a box where letter goes
                        // hvGfxBox(hvg, x, yOff+graphUpperLimit * scaleFactor, width, -height, MG_BLACK);
                        hvGfxTextInBox(hvg, x, yOff+graphUpperLimit * scaleFactor, width - 1, dataValue * scaleFactor,
                            color, font, string);
                        }
                    else
                        {
                        // useful for debug, fills a box where letter goes
                        // hvGfxBox(hvg, x, yOff-height+graphUpperLimit * scaleFactor, width, height, MG_BLACK);
                        hvGfxTextInBox(hvg, x, yOff-height+graphUpperLimit * scaleFactor, width - 1, dataValue * scaleFactor,
                            color, font, string);
                        }
                    }
                if (((boxTop+boxHeight) == 0) && !isnan(dataValue))
                    boxHeight += 1;
                }
	    double stackValue = dataValue;

	    if ((yOffsets != NULL) && (numTrack > 0))
		stackValue += yOffsets[(numTrack-1) *  width + x1];
	    if (stackValue > graphUpperLimit)
                {
		doLine(image, x, yOff, 2, clipColor);
                }
	    else if (stackValue < graphLowerLimit)
                {
		doLine(image, x, yOff + h - 1, 2, clipColor);
                }
#undef scaleHeightToPixels	/* No longer use this symbol */
            }   /*	vis == tvFull || vis == tvPack */
        }
    }	/*	for (x1 = 0; x1 < width; ++x1)	*/

return(mouseOverData);
}

struct wigMouseOver *graphPreDrawContainer(struct preDrawContainer *preDrawContainer,
    int preDrawZero, int width, struct track *tg, struct hvGfx *hvg,
    int xOff, int yOff, double graphUpperLimit, double graphLowerLimit,
    double graphRange, enum trackVisibility vis, struct wigCartOptions *wigCart)
/* Draw the graphs for all tracks in container. */
{
double epsilon = graphRange / tg->lineHeight;
struct preDrawElement *preDraw = preDrawContainer->preDraw;
Color *colorArray = makeColorArray(preDraw, width, preDrawZero, wigCart, tg, hvg);
struct wigGraphOutput *wgo = tg->wigGraphOutput;
struct wigMouseOver *mouseOverData = graphPreDraw(preDraw, preDrawZero, width,
	tg, wgo->image, wgo->vLine, wgo->xOff, wgo->yOff, wgo->yOffsets,
	wgo->numTrack, graphUpperLimit, graphLowerLimit, graphRange,
	epsilon, colorArray, vis, wigCart, wgo->pixelBins);

freez(&colorArray);
return mouseOverData;
}

void drawZeroLine(enum trackVisibility vis,
    enum wiggleGridOptEnum horizontalGrid,
    double graphUpperLimit, double graphLowerLimit,
    struct hvGfx *hvg, int xOff, int yOff, int width, int lineHeight)
/*	draw a line at y=0 on the graph	*/
{
/*	Do we need to draw a zero line ?
 *	This is to be generalized in the future to allow horizontal grid
 *	lines, perhaps user specified to indicate thresholds.
 */
if ((vis == tvFull) && (horizontalGrid == wiggleHorizontalGridOn))
    {
    Color black = hvGfxFindColorIx(hvg, 0, 0, 0);
    int x1, x2, y1, y2;

    x1 = xOff;
    x2 = x1 + width;

    /*	Let's see if the zero line can be drawn	*/
    if ((0.0 <= graphUpperLimit) && (0.0 >= graphLowerLimit))
	{
	int zeroOffset;

	zeroOffset = (int)((graphUpperLimit * lineHeight) /
			(graphUpperLimit - graphLowerLimit));
	y1 = yOff + zeroOffset;
	if (y1 >= (yOff + lineHeight)) y1 = yOff + lineHeight - 1;
	y2 = y1;
	hvGfxLine(hvg,x1,y1,x2,y2,black);
	}

    }	/*	drawing horizontalGrid	*/
}	/*	drawZeroLine()	*/

void drawArbitraryYLine(enum trackVisibility vis,
    enum wiggleGridOptEnum horizontalGrid,
    double graphUpperLimit, double graphLowerLimit,
    struct hvGfx *hvg, int xOff, int yOff, int width, int lineHeight,
    double yLineMark, double graphRange, enum wiggleYLineMarkEnum yLineOnOff)
/*	draw a line at y=yLineMark on the graph	*/
{
/*	Optionally, a user requested Y marker line at some value */
if ((vis == tvFull) && (yLineOnOff == wiggleYLineMarkOn))
    {
    int x1, x2, y1, y2;
    Color black = hvGfxFindColorIx(hvg, 0, 0, 0);

    x1 = xOff;
    x2 = x1 + width;

    /*	Let's see if this marker line can be drawn	*/
    if ((yLineMark <= graphUpperLimit) && (yLineMark >= graphLowerLimit))
	{
	int Offset;

	Offset = lineHeight * ((graphUpperLimit - yLineMark)/graphRange);
	y1 = yOff + Offset;
	if (y1 >= (yOff + lineHeight)) y1 = yOff + lineHeight - 1;
	y2 = y1;
	hvGfxLine(hvg,x1,y1,x2,y2,black);
	}

    }	/*	drawing y= line marker	*/
}	/*	drawArbitraryYLine()	*/

void wigMapSelf(struct track *tg, struct hvGfx *hvg, int seqStart, int seqEnd,
    int xOff, int yOff, int width)
/*	if self mapping, create the mapping box	*/
{
/*	Map this wiggle area if we are self mapping	*/
if (tg->mapsSelf)
    {
    char *itemName;
#ifndef GBROWSE
    if (isCustomTrack(tg->table) && tg->customPt)
	{
	struct customTrack *ct = tg->customPt;
	itemName = (char *)needMem(LARGEBUF * sizeof(char));
	safef(itemName, LARGEBUF, "%s %s", ct->wigFile, tg->track);
	}
    else
#endif /* GBROWSE */
    itemName = cloneString(tg->track);

    // Don't bother if we are imageV2 and a dense child.
    if (!theImgBox || tg->limitedVis != tvDense || !tdbIsCompositeChild(tg->tdb))
        {
        char *title = NULL;
        if (trackDbSetting(tg->tdb, "hoverMetadata"))
            title = trackDbSetting(tg->tdb, "metadata");
        mapBoxHc(hvg, seqStart, seqEnd, xOff, yOff, width, tg->height, tg->track,
                        itemName, title);
        }
    freeMem(itemName);
    }
}

int wigFindSpan(struct track *tg, double basesPerPixel)
/* Return span to use at this scale */
{
int usingDataSpan = 1;
int minimalSpan = 100000000;	/*	a lower limit safety check */
struct hashEl *el, *elList;

/*	Take a look through the potential spans, and given what we have
 *	here for basesPerPixel, pick the largest usingDataSpan that is
 *	not greater than the basesPerPixel
 */
el = hashLookup(trackSpans, tg->track);	/*  What Spans do we have */
elList = hashElListHash(el->val);		/* Our pointer to spans hash */
for (el = elList; el != NULL; el = el->next)
    {
    int Span;
    Span = ptToInt(el->val);
    if ((Span < basesPerPixel) && (Span > usingDataSpan))
	usingDataSpan = Span;
    if (Span < minimalSpan)
	minimalSpan = Span;
    }
hashElFreeList(&elList);

/*	There may not be a span of 1, use whatever is lowest	*/
if (minimalSpan > usingDataSpan)
    usingDataSpan = minimalSpan;

return usingDataSpan;
}

void wigTrackSetGraphOutputDefault(struct track *tg, int xOff, int yOff, int width, struct hvGfx *hvg)
/* Set up to draw on hvg if no other destination set already */
{
if (tg->wigGraphOutput == NULL)
    {
    tg->wigGraphOutput = wigGraphOutputSolid(xOff, yOff, hvg);
void AllocPixelBins(struct wigGraphOutput *wgo, int width);
    AllocPixelBins(tg->wigGraphOutput, width);
    }
}

static void setMinMax(struct track *tg, double graphLowerLimit, double graphUpperLimit)
// We need to check to see if this track changes our min/max for all tracks with this parent.
// We use tdb entry to store this because it may be per view, which has been smashed out in track list
{
struct trackDb *parent = tg->tdb->parent;
struct tdbExtras *extras = parent->tdbExtras;

if (extras == NULL)
    {
    AllocVar(extras);
    parent->tdbExtras = extras;
    }

struct minMax *minMax = extras->minMax;
if (minMax == NULL)
    {
    AllocVar(minMax);
    extras->minMax = minMax;
    minMax->min = graphLowerLimit;
    minMax->max = graphUpperLimit;
    }
else
    {
    if (minMax->min > graphLowerLimit)
        minMax->min = graphLowerLimit;
    if (minMax->max < graphUpperLimit)
        minMax->max = graphUpperLimit;
    }
}

void wigPreDrawPredraw(struct track *tg, int seqStart, int seqEnd,
                    struct hvGfx *hvg, int xOff, int yOff, int width,
                    MgFont *font, Color color, enum trackVisibility vis,
                    struct preDrawContainer *preContainer, int preDrawZero,
                    int preDrawSize, double *retGraphUpperLimit, double *retGraphLowerLimit)
/* Figure out graph limits after running windowingFunction and smoothing if needed.
 * This code is shared by wig, bigWig, and bedGraph drawers. */
{

/*	determined from data	*/
double graphUpperLimit=0;	/*	scaling choice will set these	*/
double graphLowerLimit=0;	/*	scaling choice will set these	*/
double epsilon;			/*	range of data in one pixel	*/
struct wigCartOptions *wigCart = (struct wigCartOptions *) tg->wigCartData;

/*	width - width of drawing window in pixels
 *	pixelsPerBase - pixels per base
 *	basesPerPixel - calculated as 1.0/pixelsPerBase
 */

struct preDrawElement *preDraw = preContainer->preDraw;

preDrawWindowFunction(preDraw, preDrawSize, wigCart->windowingFunction,
	wigCart->transformFunc, wigCart->doNegative);
preDrawSmoothing(preDraw, preDrawSize, wigCart->smoothingWindow);
if (!preContainer->skipAutoscale) // multiWig does own autoscaling
    {
    preDrawAutoScale(preDraw, preDrawZero, width,
	wigCart->autoScale, wigCart->windowingFunction,
	&preContainer->graphUpperLimit, &preContainer->graphLowerLimit,
	&epsilon, tg->lineHeight,
	wigCart->maxY, wigCart->minY, wigCart->alwaysZero);
    }

graphUpperLimit = preContainer->graphUpperLimit;
graphLowerLimit = preContainer->graphLowerLimit;

if (retGraphUpperLimit != NULL)
    *retGraphUpperLimit = graphUpperLimit;
if (retGraphLowerLimit != NULL)
    *retGraphLowerLimit = graphLowerLimit;

if (sameString(tg->tdb->type, "mathWig") && (wigCart->autoScale == wiggleScaleCumulative))
    wigCart->autoScale = wiggleScaleAuto;

if (wigCart->autoScale == wiggleScaleCumulative)
    setMinMax(tg, graphLowerLimit, graphUpperLimit);
}

void wigDrawPredraw(struct track *tg, int seqStart, int seqEnd,
                    struct hvGfx *hvg, int xOff, int yOff, int width,
                    MgFont *font, Color color, enum trackVisibility vis,
                    struct preDrawContainer *preContainer, int preDrawZero,
                    int preDrawSize, double graphUpperLimit, double graphLowerLimit)
/* Draw once we've figured out predraw (numerical values to graph) we draw it here.
 * This code is shared by wig, bigWig, and bedGraph drawers. */
{
struct wigCartOptions *wigCart = (struct wigCartOptions *) tg->wigCartData;
double graphRange;   /*	scaling choice will set this */

// if we want the cumulative autoscale, grab it from our parent
if (wigCart->autoScale == wiggleScaleCumulative)
    {
    tg->graphLowerLimit = graphLowerLimit = tg->tdb->parent->tdbExtras->minMax->min;
    tg->graphUpperLimit = graphUpperLimit = tg->tdb->parent->tdbExtras->minMax->max;
    }

/* if we're autoscaling and the range is 0 this implies that all values
 * in the given range are the same.  We create a bottom of the scale
 * by subtracting one from the only value.
 * This results in drawing a box that fills the range. */
if (graphUpperLimit == graphLowerLimit)
    {
    graphLowerLimit = graphUpperLimit - 1;
    }
graphRange = graphUpperLimit - graphLowerLimit;

wigTrackSetGraphOutputDefault(tg, xOff, yOff, width, hvg);

struct wigMouseOver *mouseOverData = NULL;
if (zoomedToCodonLevel && trackDbSettingOn(tg->tdb, "logo"))
//if (zoomedToBaseLevel && trackDbSettingOn(tg->tdb, "logo"))
    mouseOverData = logoPreDrawContainer(preContainer,
        preDrawZero, width, tg, hvg, xOff, yOff,
        graphUpperLimit, graphLowerLimit, graphRange, vis, wigCart, seqStart, seqEnd);
else
    mouseOverData = graphPreDrawContainer(preContainer,
        preDrawZero, width, tg, hvg, xOff, yOff,
        graphUpperLimit, graphLowerLimit, graphRange, vis, wigCart);

drawZeroLine(vis, wigCart->horizontalGrid,
    graphUpperLimit, graphLowerLimit,
    hvg, xOff, yOff, width, tg->lineHeight);

drawArbitraryYLine(vis, (enum wiggleGridOptEnum)wigCart->yLineOnOff,
    graphUpperLimit, graphLowerLimit,
    hvg, xOff, yOff, width, tg->lineHeight, wigCart->yLineMark, graphRange,
    wigCart->yLineOnOff);

if (enableMouseOver && mouseOverData)
    {
    jsonWriteObjectStart(mouseOverJson, tg->track);
    jsonWriteString(mouseOverJson, "t", tg->tdb->type);
    jsonWriteListStart(mouseOverJson, "d");
    slReverse(&mouseOverData);
    struct wigMouseOver *dataItem = mouseOverData;
    for (; dataItem; dataItem = dataItem->next)
        {
        jsonWriteObjectStart(mouseOverJson, NULL);
        jsonWriteNumber(mouseOverJson, "x1", (long long)dataItem->x1);
        jsonWriteNumber(mouseOverJson, "x2", (long long)dataItem->x2);
        jsonWriteDouble(mouseOverJson, "v", dataItem->value);
        jsonWriteNumber(mouseOverJson, "c", dataItem->valueCount);
        jsonWriteObjectEnd(mouseOverJson);
        }
    jsonWriteListEnd(mouseOverJson);
    jsonWriteObjectEnd(mouseOverJson);
    slFreeList(&mouseOverData);
    // hidden element to pass along jsonUrl file name and also the trigger
    // that this track has data to display.
    hPrintf("<div id='mouseOver_%s' name='%s' class='hiddenText mouseOverData' jsonUrl='%s'></div>\n", tg->track, tg->track, mouseOverJsonFile->forCgi);
    }
else if (enableMouseOver)
    {
    jsonWriteObjectStart(mouseOverJson, tg->track);
    jsonWriteString(mouseOverJson, "t", tg->tdb->type);
    jsonWriteString(mouseOverJson, "mo", "noAverage");
    jsonWriteObjectEnd(mouseOverJson);
    }

wigMapSelf(tg, hvg, seqStart, seqEnd, xOff, yOff, width);
}	/*	void wigDrawPredraw()	*/

struct preDrawContainer *wigLoadPreDraw(struct track *tg, int seqStart, int seqEnd, int width)
/* Do bits that load the predraw buffer tg->preDrawContainer. */
{
/* Just need to do this once... */
if (tg->preDrawContainer)
    return tg->preDrawContainer;

struct wigItem *wi;
double pixelsPerBase = scaleForPixels(width);
double basesPerPixel = 1.0;
int itemCount = 0;
char *currentFile = NULL;
//char *currentFileRewrite = NULL;
struct udcFile *wibFH = NULL;	/*	file handle to binary file */
int i;				/* an integer loop counter	*/
int x1 = 0;			/*	screen coordinates	*/
int x2 = 0;			/*	screen coordinates	*/
int usingDataSpan = 1;		/* will become larger if possible */

if (tg->items == NULL)
    return NULL;

if (pixelsPerBase > 0.0)
    basesPerPixel = 1.0 / pixelsPerBase;

/*	width - width of drawing window in pixels
 *	pixelsPerBase - pixels per base
 *	basesPerPixel - calculated as 1.0/pixelsPerBase
 */
itemCount = 0;

/* Allocate predraw and save it and related info in the track. */
struct preDrawContainer *pre = tg->preDrawContainer = initPreDrawContainer(width);
struct preDrawElement *preDraw = pre->preDraw;	/* to accumulate everything in prep for draw */
int preDrawZero = pre->preDrawZero;		/* location in preDraw where screen starts */
int preDrawSize = pre->preDrawSize;		/* size of preDraw array */

usingDataSpan = wigFindSpan(tg, basesPerPixel);

/*	walk through all the data and prepare the preDraw array	*/
for (wi = tg->items; wi != NULL; wi = wi->next)
    {
    int dataOffset = 0;		/*	within data block during drawing */

    ++itemCount;


    /*	Now that we know what Span to draw, see if this item should be
     *	drawn at all.
     */
    if (usingDataSpan == wi->span)
	{
	/*	Check our data file, see if we need to open a new one */
	if (differentStringNullOk(currentFile,""))
	    {
	    if (differentStringNullOk(currentFile,wi->file))
		{
		if (wibFH > 0)
		    {
		    udcFileClose(&wibFH);
		    freeMem(currentFile);
		    }
                currentFile = cloneString(wi->file);
		wibFH = udcFileMayOpen(hReplaceGbdb(currentFile), NULL);
		if (wibFH==NULL)
		    errAbort("hgTracks/wigLoadPreDraw: failed to open wiggle %s", currentFile);
		}
	    }
	else
	    {
            currentFile = cloneString(wi->file);
            wibFH = udcFileMayOpen(hReplaceGbdb(currentFile), NULL);
	    if (wibFH==NULL)
		errAbort("hgTracks/wigLoadPreDraw: failed to open wiggle %s", currentFile);
	    }
/*	Ready to draw, what do we know:
 *	the feature being processed:
 *	chrom coords:  [wi->start : wi-end)
 *
 *	The data to be drawn: to be read from file f at offset wi->Offset
 *	data points available: wi->Count, representing wi->Span bases
 *	for each data point
 *
 *	The drawing window, in pixels:
 *	xOff = left margin, yOff = top margin, h = height of drawing window
 *	drawing window in chrom coords: seqStart, seqEnd
 *      'basesPerPixel' is known, 'pixelsPerBase' is known
 */
        /*      let's check end point screen coordinates.  If they are
         *      the same, then this entire data block lands on one pixel,
         *      no need to walk through it, just use the block's specified
         *      max/min.  It is OK if these end up + or - values, we do want to
         *      keep track of pixels before and after the screen for
         *      later smoothing operations.  x1d,x2d are pixel coordinates
         */
double x1d = (double)(wi->start - seqStart) * pixelsPerBase;
	x1 = round(x1d);
double x2d = (double)((wi->start+(wi->count * usingDataSpan))-seqStart) * pixelsPerBase;
	x2 = round(x2d);

        /* this used to be if (x2 > x1) which often caused reading of blocks
	 * when they were merely x2 = x1 + 1 due to rounding errors as
	 * they became integers.  This double comparison for something over
	 * 0.5 will account for rounding errors that are really small, but
	 * still handle a slipping window size as it walks across the screen
	 */
	if ((x2d - x1d) > 0.5)
	    {
	    unsigned char *readData;	/* the bytes read in from the file */
	    udcSeek(wibFH, wi->offset);
	    readData = (unsigned char *) needMem((size_t) (wi->count + 1));
	    udcRead(wibFH, readData,
		(size_t) wi->count * (size_t) sizeof(unsigned char));
	    /*	walk through all the data in this wiggle data block	*/
	    for (dataOffset = 0; dataOffset < wi->count; ++dataOffset)
		{
		unsigned char datum = readData[dataOffset];
		if (datum != WIG_NO_DATA)
		    {
		    /* (wi->start-seqStart) == base where this wiggle data block
		     *	begins.  Add to that (dataOffset * usingDataSpan) which
		     *	is how many bases this specific datum is from the start
		     *	of this wiggle data block.
		     * x1,x2 are the pixel begin and end for this data item */
		    x1 = ((wi->start-seqStart) + (dataOffset * usingDataSpan)) * pixelsPerBase;
		    /* (usingDataSpan * pixelsPerBase) is the number of pixels
		     *	occupied by this one data item
		     */
		    x2 = x1 + (usingDataSpan * pixelsPerBase);
		    for (i = x1; i <= x2; ++i)
			{
			int xCoord = preDrawZero + i;
			if ((xCoord >= 0) && (xCoord < preDrawSize))
			    {
			    double dataValue =
			       BIN_TO_VALUE(datum,wi->lowerLimit,wi->dataRange);

			    ++preDraw[xCoord].count;
			    if (dataValue > preDraw[xCoord].max)
				preDraw[xCoord].max = dataValue;
			    if (dataValue < preDraw[xCoord].min)
				preDraw[xCoord].min = dataValue;
			    preDraw[xCoord].sumData += dataValue;
			    preDraw[xCoord].sumSquares += dataValue * dataValue;
			    }
			}
		    }
		}
	    freeMem(readData);
            }
        else
	    {	/*	only one pixel for this block of data */
	    int xCoord = preDrawZero + x1;
	    /*	if the point falls within our array, record it.
	     *	the (wi->validCount > 0) is a safety check.  It
	     *	should always be true unless the data was
	     *	prepared incorrectly.
	     */
	    if ((wi->validCount > 0) && (xCoord >= 0) && (xCoord < preDrawSize))
		{
		double upperLimit;
		preDraw[xCoord].count += wi->validCount;
		upperLimit = wi->lowerLimit + wi->dataRange;
		if (upperLimit > preDraw[xCoord].max)
		    preDraw[xCoord].max = upperLimit;
		if (wi->lowerLimit < preDraw[xCoord].min)
		    preDraw[xCoord].min = wi->lowerLimit;
		preDraw[xCoord].sumData += wi->sumData;
		preDraw[xCoord].sumSquares += wi->sumSquares;
		}
	    }
	}	/*	Draw if span is correct	*/
    }	/*	for (wi = tg->items; wi != NULL; wi = wi->next)	*/
if (wibFH > 0)
    {
    udcFileClose(&wibFH);
    wibFH = 0;
    freeMem(currentFile);
    }
return pre;
}

static void wigPreDrawItems(struct track *tg, int seqStart, int seqEnd,
	struct hvGfx *hvg, int xOff, int yOff, int width,
	MgFont *font, Color color, enum trackVisibility vis)
/* Draw wiggle items that resolve to doing a box for each pixel. */
{
struct preDrawContainer *pre = wigLoadPreDraw(tg, seqStart, seqEnd, width);
if (pre != NULL)
    {
    wigPreDrawPredraw(tg, seqStart, seqEnd, hvg, xOff, yOff, width, font, color, vis,
                   pre, pre->preDrawZero, pre->preDrawSize,
                   &tg->graphUpperLimit, &tg->graphLowerLimit);
    }
}

void wigMultiRegionGraphLimits(struct track *tg)
/* Set common graphLimits across all windows */
{
double graphUpperLimit = -BIGDOUBLE;
double graphLowerLimit = BIGDOUBLE;
struct track *tgSave = tg;
struct window *w;
// find graphLimits across all windows.
for(w=windows,tg=tgSave; w; w=w->next,tg=tg->nextWindow)
    {
    if (tg->graphUpperLimit > graphUpperLimit)
	graphUpperLimit = tg->graphUpperLimit;
    if (tg->graphLowerLimit < graphLowerLimit)
	graphLowerLimit = tg->graphLowerLimit;
    }
// set same common graphLimits in all windows.
for(w=windows,tg=tgSave; w; w=w->next,tg=tg->nextWindow)
    {
    tg->graphUpperLimit = graphUpperLimit;
    tg->graphLowerLimit = graphLowerLimit;
    }
}

static void wigDrawItems(struct track *tg, int seqStart, int seqEnd,
	struct hvGfx *hvg, int xOff, int yOff, int width,
	MgFont *font, Color color, enum trackVisibility vis)
/* Draw wiggle items that resolve to doing a box for each pixel. */
{
struct preDrawContainer *pre = tg->preDrawContainer;
if (pre != NULL)
    {
    wigDrawPredraw(tg, seqStart, seqEnd, hvg, xOff, yOff, width, font, color, vis,
                   pre, pre->preDrawZero, pre->preDrawSize,
                   tg->graphUpperLimit, tg->graphLowerLimit);
    }
}

void wigLeftAxisLabels(struct track *tg, int seqStart, int seqEnd,
                       struct hvGfx *hvg, int xOff, int yOff, int width, int height,
                       boolean withCenterLabels, MgFont *font, Color color,
                       enum trackVisibility vis, char *shortLabel,
                       double graphUpperLimit, double graphLowerLimit, boolean showNumbers)
/* Draw labels on left for a wiggle-type track. */
{
int fontHeight = tl.fontHeight+1;
int centerOffset = 0;
double lines[2];	/*	lines to label	*/
int numberOfLines = 1;	/*	at least one: 0.0	*/
int i;			/*	loop counter	*/
struct wigCartOptions *wigCart = (struct wigCartOptions *) tg->wigCartData;

lines[0] = 0.0;
lines[1] = wigCart->yLineMark;
if (wigCart->yLineOnOff == wiggleYLineMarkOn)
    ++numberOfLines;

if (withCenterLabels)
    centerOffset = fontHeight;

/*	We only do Dense and Full	*/
if (tg->limitedVis == tvDense)
    {
    hvGfxTextRight(hvg, xOff, yOff+centerOffset, width - 1, height-centerOffset,
	color, font, shortLabel);
    }
else if (tg->limitedVis == tvFull)
    {
    int centerLabel = (height/2)-(fontHeight/2);
    int labelWidth = mgFontStringWidth(font, shortLabel);

    /* track label is centered in the whole region */
    hvGfxText(hvg, xOff, yOff+centerLabel, color, font, shortLabel);

    /*	Is there room left to draw the min, max ?	*/
    if (showNumbers && height >= (3 * fontHeight))
	{
	boolean zeroOK = TRUE;
	char upper[SMALLBUF];
        char lower[SMALLBUF];
        char upperTic = '-';    /* as close as we can get with ASCII */
                        /* the ideal here would be to draw tic marks in
                         * exactly the correct location.
                         */
        Color drawColor;
        if (withCenterLabels)
	    {
	    centerOffset = fontHeight;
	    upperTic = '_';	/*	this is correct	*/
	    }

	/*  In areas where there is no data, these limits do not change */
	if (graphUpperLimit < graphLowerLimit)
	    {
	    double d = graphLowerLimit;
	    graphLowerLimit = graphUpperLimit;
	    graphUpperLimit = d;
            if (hvg->rc)
                {
                safef(upper, sizeof(upper), " %c No data", upperTic);
                safef(lower, sizeof(lower), "_ No data");
                }
            else
                {
                safef(upper, sizeof(upper), "No data %c", upperTic);
                safef(lower, sizeof(lower), "No data _");
                }
	    zeroOK = FALSE;
	    }
	else
	    {
	    enum wiggleTransformFuncEnum transformFunc = wigCart->transformFunc;
	    boolean gotLog = (transformFunc == wiggleTransformFuncLog);
	    char *transform = (gotLog ? "ln(x+1) " : "");
            if (hvg->rc)
                {
                safef(upper, sizeof(upper), "%c %s%g", upperTic, transform, graphUpperLimit);
                safef(lower, sizeof(lower), "_ %g", graphLowerLimit);
                }
            else
                {
                safef(upper, sizeof(upper), "%s%g %c", transform, graphUpperLimit, upperTic);
                safef(lower, sizeof(lower), "%g _", graphLowerLimit);
                }
	    }
	drawColor = color;
	if (graphUpperLimit < 0.0) drawColor = tg->ixAltColor;
	hvGfxTextRight(hvg, xOff, yOff, width - 1, fontHeight, drawColor,
	    font, upper);
	drawColor = color;
	if (graphLowerLimit < 0.0) drawColor = tg->ixAltColor;
	hvGfxTextRight(hvg, xOff, yOff+height-fontHeight, width - 1, fontHeight,
	    drawColor, font, lower);

	for (i = 0; i < numberOfLines; ++i )
	    {
	    double lineValue = lines[i];
	    /*	Maybe zero can be displayed */
	    /*	It may overwrite the track label ...	*/
	    if (zeroOK && (lineValue < graphUpperLimit) &&
		(lineValue > graphLowerLimit))
		{
		int offset;
		int Width;

		drawColor = hvGfxFindColorIx(hvg, 0, 0, 0);
		offset = centerOffset +
		    (int)(((graphUpperLimit - lineValue) *
				(height - centerOffset)) /
			(graphUpperLimit - graphLowerLimit));
		/*	reusing the lower string here	*/
                if (hvg->rc)
                    safef(lower, sizeof(lower), "- %g", ((i == 0) ? 0.0 : lineValue));
                else
                    safef(lower, sizeof(lower), "%g -", ((i == 0) ? 0.0 : lineValue));
		/*	only draw if it is far enough away from the
		 *	upper and lower labels, and it won't overlap with
		 *	the center label.
		 */
		Width = mgFontStringWidth(font,lower);
		if ( !( (offset < centerLabel+fontHeight) &&
		    (offset > centerLabel-(fontHeight/2)) &&
		    (Width+labelWidth >= width) ) &&
		    (offset > (fontHeight*2)) &&
		    (offset < height-(fontHeight*2)) )
		    {
		    hvGfxTextRight(hvg, xOff, yOff+offset-(fontHeight/2),
			width - 1, fontHeight, drawColor, font, lower);
		    }
		}	/*	drawing a zero label	*/
	    }	/*	drawing 0.0 and perhaps yLineMark	*/
	}	/* if (height >= (3 * fontHeight))	*/
    }	/*	if (tg->visibility == tvFull)	*/
} /* wigAxisLeftLabels */

void wigLeftLabels(struct track *tg, int seqStart, int seqEnd,
	struct hvGfx *hvg, int xOff, int yOff, int width, int height,
	boolean withCenterLabels, MgFont *font, Color color,
	enum trackVisibility vis)
/*	drawing left labels	*/
{
wigLeftAxisLabels(tg, seqStart, seqEnd, hvg, xOff, yOff, width, height, withCenterLabels,
                  font, color, vis, tg->shortLabel, tg->graphUpperLimit, tg->graphLowerLimit,TRUE);
}

struct wigCartOptions *wigCartOptionsNew(struct cart *cart, struct trackDb *tdb, int wordCount, char *words[])
/* Create a wigCartOptions from cart contents and tdb. */
{
struct wigCartOptions *wigCart;

int defaultHeight;	/*	truncated by limits	*/
double yLineMark;	/*	from trackDb or cart */
int maxHeight = atoi(DEFAULT_HEIGHT_PER);
int minHeight = MIN_HEIGHT_PER;

AllocVar(wigCart);

/*	These Fetch functions look for variables in the cart bounded by
 *	limits specified in trackDb or returning defaults
 */
wigCart->lineBar = wigFetchGraphTypeWithCart(cart,tdb,tdb->track, (char **) NULL);
wigCart->horizontalGrid = wigFetchHorizontalGridWithCart(cart,tdb,tdb->track, (char **) NULL);

wigCart->autoScale = wigFetchAutoScaleWithCart(cart,tdb,tdb->track, (char **) NULL);
wigCart->windowingFunction = wigFetchWindowingFunctionWithCart(cart,tdb,tdb->track, (char **) NULL);
wigCart->smoothingWindow = wigFetchSmoothingWindowWithCart(cart,tdb,tdb->track, (char **) NULL);

wigFetchMinMaxPixelsWithCart(cart,tdb,tdb->track, &minHeight, &maxHeight, &defaultHeight);
wigFetchYLineMarkValueWithCart(cart,tdb,tdb->track, &yLineMark);
wigCart->yLineMark = yLineMark;
wigCart->yLineOnOff = wigFetchYLineMarkWithCart(cart,tdb,tdb->track, (char **) NULL);
wigCart->alwaysZero = (enum wiggleAlwaysZeroEnum)wigFetchAlwaysZeroWithCart(cart,tdb,tdb->track, (char **) NULL);
wigCart->transformFunc = (enum wiggleTransformFuncEnum)wigFetchTransformFuncWithCart(cart,tdb,tdb->track, (char **) NULL);
wigCart->doNegative = wigFetchDoNegativeWithCart(cart,tdb,tdb->track, (char **) NULL);

wigCart->maxHeight = maxHeight;
wigCart->defaultHeight = defaultHeight;
wigCart->minHeight = minHeight;

wigFetchMinMaxYWithCart(cart,tdb,tdb->track, &wigCart->minY, &wigCart->maxY, NULL, NULL, wordCount, words);

wigCart->colorTrack = trackDbSetting(tdb, "wigColorBy");

char *containerType = trackDbSetting(tdb, "container");
if (containerType != NULL && sameString(containerType, "multiWig"))
     wigCart->isMultiWig = TRUE;

wigCart->aggregateFunction = wigFetchAggregateFunctionWithCart(cart,tdb,tdb->track, (char **) NULL);

// can't do mean with whiskers in stacked mode
if ((wigCart->aggregateFunction == wiggleAggregateStacked) &&
    ( wigCart->windowingFunction == wiggleWindowingWhiskers))
    wigCart->windowingFunction = wiggleWindowingMax;
return wigCart;
}

/* Make track group for wig multiple alignment.
 *	WARNING ! - track->visibility is merely the default value
 *	from the trackDb entry at this time.  It will be set after this
 *	 by hgTracks from its cart UI setting.  When called in
 *	 TotalHeight it will then be the requested visibility.
 */
void wigMethods(struct track *track, struct trackDb *tdb,
	int wordCount, char *words[])
{
struct wigCartOptions *wigCart = wigCartOptionsNew(cart, tdb, wordCount, words);
track->minRange = wigCart->minY;
track->maxRange = wigCart->maxY;
track->graphUpperLimit = wigEncodeStartingUpperLimit;
track->graphLowerLimit = wigEncodeStartingLowerLimit;
wigCart->bedGraph = FALSE;	/*	signal to left labels	*/

track->loadItems = wigLoadItems;
track->freeItems = wigFreeItems;
track->preDrawItems = wigPreDrawItems;
track->preDrawMultiRegion = wigMultiRegionGraphLimits;
track->drawItems = wigDrawItems;
track->itemName = wigNameCallback;
track->mapItemName = wigNameCallback;
track->totalHeight = wigTotalHeight;
track->itemHeight = tgFixedItemHeight;

track->itemStart = tgItemNoStart;
track->itemEnd = tgItemNoEnd;
/*	the wigMaf parent will turn mapsSelf off	*/
track->mapsSelf = TRUE;
track->wigCartData = (void *) wigCart;
track->colorShades = shadesOfGray;
track->drawLeftLabels = wigLeftLabels;
track->loadPreDraw = wigLoadPreDraw;
/*	the lfSubSample type makes the image map function correctly */
track->subType = lfSubSample;     /*make subType be "sample" (=2)*/

}	/*	wigMethods()	*/
