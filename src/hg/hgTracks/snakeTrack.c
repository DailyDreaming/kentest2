/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifdef USE_HAL
/* snakeTrack - stuff to load and display snake type tracks in browser.  */

#include "common.h"
#include "hash.h"
#include "localmem.h"
#include "linefile.h"
#include "jksql.h"
#include "hdb.h"
#include "hgTracks.h"
#include "chainBlock.h"
#include "chainLink.h"
#include "chainDb.h"
#include "chainCart.h"
#include "errCatch.h"
#include "twoBit.h"
#include "bigWarn.h"
#include <pthread.h>
#include "trackHub.h"
#include "limits.h"
#include "snakeUi.h"
#include "bits.h"
#include "trix.h"

#include "halBlockViz.h"
#include "bigPsl.h"

// this is the number of pixels used by the target self-align bar
#define DUP_LINE_HEIGHT	4
// this is the number of pixels used when displaying the insertion lengths
#define INSERT_TEXT_HEIGHT 10

struct snakeFeature
    {
    struct snakeFeature *next;
    int start, end;			/* Start/end in browser coordinates. */
    int qStart, qEnd;			/* query start/end */
    int level;				/* level in snake */
    int orientation;			/* strand... -1 is '-', 1 is '+' */
    boolean drawn;			/* did we draw this feature? */
    char *qSequence;			/* may have sequence, or NULL */
    char *tSequence;			/* may have sequence, or NULL */
    char *qName;			/* chrom name on other species */
    unsigned pixX1, pixX2;              /* pixel coordinates within window */
    };

static int snakeFeatureCmpTStart(const void *va, const void *vb)
/* sort by start position on the target sequence */
{
const struct snakeFeature *a = *((struct snakeFeature **)va);
const struct snakeFeature *b = *((struct snakeFeature **)vb);
int diff = a->start - b->start;

return diff;
}

static int snakeFeatureCmpQStart(const void *va, const void *vb)
/* sort by start position on the query sequence */
{
const struct snakeFeature *a = *((struct snakeFeature **)va);
const struct snakeFeature *b = *((struct snakeFeature **)vb);
int diff = a->qStart - b->qStart;

if (diff == 0)
    {
    diff = a->start - b->start;
    }

return diff;
}

// static machinery to calculate full and pack snake

#define NUM_LEVELS	1000

struct level
{
boolean init;		/* has this level been initialized */
int orientation;	/* strand.. see above */
unsigned long edge;	/* the leading edge of this level */
int adjustLevel;	/* used to compress out the unused levels */
boolean hasBlock;	/* are there any blocks in this level */

// these fields are used to compact full snakes
unsigned *connectsToLevel;  /* what levels does this level connect to */
Bits *pixels;               /* keep track of what pixels are used */
struct snakeFeature *blocks; /* the ungapped blocks attached to this level */
};

static struct level Levels[NUM_LEVELS]; /* for packing the snake, not re-entrant! */
static int maxLevel = 0;     	     /* deepest level */	

/* blocks that make it through the min size filter */
static struct snakeFeature *newList = NULL;   

static void clearLevels()
/* clear out the data structure that we use to calculate snakes */
{
int ii;

for(ii=0; ii < sizeof(Levels) / sizeof(Levels[0]); ii++)
    Levels[ii].init = FALSE;
maxLevel = 0;
}

static void calcFullSnakeHelper(struct snakeFeature *list, int level)
// calculate a full snake
// updates global newList with unsorted blocks that pass the min
// size filter.
{
struct snakeFeature *cb = list;
struct snakeFeature *proposedList = NULL;

if (level > maxLevel)
    maxLevel = level;
if (level > ArraySize(Levels))
    errAbort("too many levels");

if (Levels[level].init == FALSE)
    {
    // initialize the level if this is the first time we've seen it
    Levels[level].init = TRUE;
    Levels[level].orientation = list->orientation;
    if (list->orientation == -1)
	Levels[level].edge = LONG_MAX; // bigger than the biggest chrom
    else
	Levels[level].edge = 0;
    }

// now we step through the blocks and assign them to levels
struct snakeFeature *next;
struct snakeFeature *insertHead = NULL;
for(; cb; cb = next)
    {
    // we're going to add this block to a different list 
    // so keep track of the next pointer
    next = cb->next;

    // if this block can't fit on this level add to the insert
    // list and move on to the next block
    if ((Levels[level].orientation != cb->orientation) ||
	((cb->orientation == 1) && (Levels[level].edge > cb->start)) ||
	((cb->orientation == -1) && (Levels[level].edge < cb->end)))
	{
	// add this to the list of an insert
	slAddHead(&insertHead, cb);
	continue;
	}

    // the current block will fit on this level
    // go ahead and deal with the blocks that wouldn't 
    if (insertHead)
	{
	// we had an insert, go ahead and calculate where that goes
	slReverse(&insertHead);
	calcFullSnakeHelper(insertHead, level + 1);
	insertHead = NULL;
	}

    // assign the current block to this level
    cb->level = level;

    if (cb->orientation == 1)
	Levels[level].edge = cb->end;
    else
	Levels[level].edge = cb->start;

    // add this block to the list of proposed blocks for this level
    slAddHead(&proposedList, cb);
    }

// we're at the end of the list.  Deal with any blocks
// that didn't fit on this level
if (insertHead)
    {
    slReverse(&insertHead);
    int nextLevel = level + 1;
    calcFullSnakeHelper(insertHead, nextLevel);
    insertHead = NULL;
    }

// do we have any proposed blocks
if (proposedList == NULL) 
    return;

// we parsed all the blocks in the list to see if they fit in our level
// now let's see if the list of blocks for this level is big enough
// to actually add

struct snakeFeature *temp;
double scale = scaleForWindow(insideWidth, winStart, winEnd);
int start, end;

// order the blocks so lowest start position is first
if (Levels[level].orientation == 1)
    slReverse(&proposedList);

start=proposedList->start;
for(temp=proposedList; temp->next; temp = temp->next)
    ;
end=temp->end;

// calculate how big the string of blocks is in screen coordinates
double apparentSize = scale * (end - start);

if (apparentSize < 0)
    errAbort("size of a list of blocks should not be less than zero");

// check to see if the apparent size is big enough
if (apparentSize < 1)
    return;

// transfer proposedList to new block list
for(temp=proposedList; temp; temp = next)
    {
    next = temp->next;
    temp->next = NULL;
    slAddHead(&newList, temp);
    }
}

struct snakeInfo
{
int maxLevel;
} snakeInfo;

static void freeFullLevels()
// free the connection levels and bitmaps for each level
{
int ii;

for(ii=0; ii <= maxLevel; ii++)
    {
    freez(&Levels[ii].connectsToLevel);
    bitFree(&Levels[ii].pixels);
    }
}

static void initializeFullLevels(struct snakeFeature *sfList)
/* initialize levels for compact snakes */
{
int ii;

for(ii=0; ii <= maxLevel; ii++)
    {
    Levels[ii].connectsToLevel = needMem((maxLevel+1) * sizeof(unsigned));
    Levels[ii].pixels = bitAlloc(insideWidth);
    Levels[ii].blocks = NULL;
    }

struct snakeFeature *sf, *next;
int prevLevel = -1;
double scale = scaleForWindow(insideWidth, winStart, winEnd);

for(sf=sfList; sf; sf = next)
    {
    next = sf->next;

    if (positiveRangeIntersection(sf->start, sf->end, winStart, winEnd))
	{
	int sClp = (sf->start < winStart) ? winStart : sf->start;
	sf->pixX1 = round((sClp - winStart)*scale);
	int eClp = (sf->end > winEnd) ? winEnd : sf->end;
	sf->pixX2 = round((eClp - winStart)*scale);
	}
    else
	{
	sf->pixX1 = sf->pixX2 = 0;
	}

    bitSetRange(Levels[sf->level].pixels, sf->pixX1, sf->pixX2 - sf->pixX1);

    if (prevLevel != -1)
	Levels[sf->level].connectsToLevel[prevLevel] = 1;

    if(next != NULL)
	Levels[sf->level].connectsToLevel[next->level] = 1;

    prevLevel = sf->level;

    slAddHead(&Levels[sf->level].blocks, sf);
    }
}

static int highestLevelBelowUs(int level)
// is there a level below us that doesn't connect to us
{
int newLevel;

// find the highest level below us that we connect to
for(newLevel = level - 1; newLevel >= 0; newLevel--)	 
    if (Levels[level].connectsToLevel[newLevel])
	break;

// if the next level is more than one below us, we may
// be able to push our blocks to it.
if ((newLevel < 0) || (newLevel + 1 == level))
    return -1;

return newLevel + 1;
}

// the number of pixels we need clear on either side of the blocks we push
#define EXTRASPACE	10

static boolean checkBlocksOnLevel(int level, struct snakeFeature *sfList)
// is there enough pixels on this level to hold our blocks?
{
struct snakeFeature *sf;

for(sf=sfList; sf;  sf = sf->next)
    {
    if (bitCountRange(Levels[level].pixels, sf->pixX1 - EXTRASPACE, (sf->pixX2 - sf->pixX1) + 2*EXTRASPACE))
	return FALSE;
    }
return TRUE;
}

static void collapseLevel(int oldLevel, int newLevel)
// push level up to higher level
{
struct snakeFeature *sf;
struct snakeFeature *next;

for(sf=Levels[oldLevel].blocks; sf;  sf = next)
    {
    next = sf->next;
    sf->level = newLevel;
    slAddHead(&Levels[newLevel].blocks, sf);

    bitSetRange(Levels[sf->level].pixels, sf->pixX1, sf->pixX2 - sf->pixX1);
    }
Levels[oldLevel].blocks = NULL;
Levels[oldLevel].pixels = bitAlloc(insideWidth);
}

static void remapConnections(int oldLevel, int newLevel)
// if we've moved a level, we need to remap all the connections
{
int ii, jj;

for(ii=0; ii <= maxLevel; ii++)
    {
    for(jj=0; jj <= maxLevel; jj++)
	{
	if (Levels[ii].connectsToLevel[oldLevel])
	    {
	    Levels[ii].connectsToLevel[oldLevel] = 0;
	    Levels[ii].connectsToLevel[newLevel] = 1;
	    }
	}
    }
}

static struct snakeFeature *compactSnakes(struct snakeFeature *sfList)
// collapse levels that will fit on a higer level
{
int ii;

initializeFullLevels(sfList);

// start with third level to see what can be compacted
for(ii=2; ii <= maxLevel; ii++) 
    {
    int newLevel;
    // is there a level below us that doesn't connect to us
    if ((newLevel = highestLevelBelowUs(ii)) > 0)
	{
	int jj;
	for (jj=newLevel; jj < ii; jj++)
	    {
	    if (checkBlocksOnLevel(jj, Levels[ii].blocks))
		{
		collapseLevel(ii, jj);
		remapConnections(ii, jj);
		break;
		}
	    }
	}
    }

// reattach blocks in the levels to linkedFeature
struct snakeFeature *newList = NULL;
for(ii=0; ii <= maxLevel; ii++)
    {
    newList = slCat(Levels[ii].blocks, newList);
    }
slSort(&newList, snakeFeatureCmpQStart);

// figure out the new max
int newMax = 0;
for(ii=0; ii <= maxLevel; ii++)
    if (Levels[ii].blocks != NULL)
	newMax = ii;

freeFullLevels();
maxLevel = newMax;

return newList;
}

static void calcFullSnake(struct track *tg, void *item)
// calculate a full snake
{
struct linkedFeatures  *lf = (struct linkedFeatures *)item;

// if there aren't any blocks, don't bother
if (lf->components == NULL)
    return;

// we use the codons field to keep track of whether we already
// calculated the height of this snake
if (lf->codons == NULL)
    {
    clearLevels();
    struct snakeFeature *sf;

    // this will destroy lf->components, and add to newList
    calcFullSnakeHelper((struct snakeFeature *)lf->components, 0);
    lf->components = (struct simpleFeature *)newList;
    newList = NULL;
    slSort(&lf->components, snakeFeatureCmpQStart);

    // now we're going to compress the levels that aren't used
    // to do that, we need to see which blocks are on the screen,
    // or connected to something on the screen
    int oldMax = maxLevel;
    clearLevels();
    struct snakeFeature *prev = NULL;
    for(sf=(struct snakeFeature *)lf->components; sf; prev = sf, sf = sf->next)
	{
	if (Levels[sf->level].init == FALSE)
	    {
	    Levels[sf->level].init = TRUE;
	    Levels[sf->level].orientation = sf->orientation;
	    Levels[sf->level].hasBlock = FALSE;
	    }
	else
	    {
	    if (Levels[sf->level].orientation != sf->orientation)
		errAbort("found snakeFeature with wrong orientation");
	    if ((sf->orientation == 1) && (Levels[sf->level].edge > sf->start)) 
		errAbort("found snakeFeature that violates edge");
	    if ((sf->orientation == -1) && (Levels[sf->level].edge < sf->end))
		errAbort("found snakeFeature that violates edge");
	    }
	if (sf->orientation == 1)
	    Levels[sf->level].edge = sf->end;
	else
	    Levels[sf->level].edge = sf->start;
	if (sf->level > maxLevel)
	    maxLevel = sf->level;
	if(positiveRangeIntersection(winStart, winEnd, sf->start, sf->end))
	    {
	    Levels[sf->level].hasBlock = TRUE;
	    if (sf->next != NULL)
		Levels[sf->next->level].hasBlock = TRUE;
	    if (prev != NULL)
		Levels[prev->level].hasBlock = TRUE;

	    }
	}

    // now figure out how to remap the blocks
    int ii;
    int count = 0;
    for(ii=0; ii < oldMax + 1; ii++)
	{
	Levels[ii].adjustLevel = count;
	if ((Levels[ii].init) && (Levels[ii].hasBlock))
	    count++;
	}
    maxLevel = count;

    // remap blocks
    for(sf=(struct snakeFeature *)lf->components; sf; sf = sf->next)
	sf->level = Levels[sf->level].adjustLevel;

    // now compact the snakes
    lf->components = (void *)compactSnakes((struct snakeFeature *)lf->components);
    struct snakeInfo *si;
    AllocVar(si);
    si->maxLevel = maxLevel;
    lf->codons = (struct simpleFeature *)si;
    }
}

static void calcPackSnakeHelper(struct snakeFeature *list, int level)
// calculate a packed snake
// updates global newList with unsorted blocks that pass the min
// size filter.
{
struct snakeFeature *cb = list;
struct snakeFeature *proposedList = NULL;

if (level > maxLevel)
    maxLevel = level;
if (level > ArraySize(Levels))
    errAbort("too many levels");

if (Levels[level].init == FALSE)
    {
    // initialize the level if this is the first time we've seen it
    Levels[level].init = TRUE;
    Levels[level].edge = 0;
    }

// now we step through the blocks and assign them to levels
struct snakeFeature *next;
struct snakeFeature *insertHead = NULL;
for(; cb; cb = next)
    {
    // we're going to add this block to a different list 
    // so keep track of the next pointer
    next = cb->next;

    // if this block can't fit on this level add to the insert
    // list and move on to the next block
    if ( Levels[level].edge > cb->start)
	{
	// add this to the list of an insert
	slAddHead(&insertHead, cb);
	continue;
	}

    // the current block will fit on this level
    // go ahead and deal with the blocks that wouldn't 
    if (insertHead)
	{
	// we had an insert, go ahead and calculate where that goes
	slReverse(&insertHead);
	calcPackSnakeHelper(insertHead, level + 1);
	insertHead = NULL;
	}

    // assign the current block to this level
    cb->level = level;

    Levels[level].edge = cb->end;

    // add this block to the list of proposed blocks for this level
    slAddHead(&proposedList, cb);
    }

// we're at the end of the list.  Deal with any blocks
// that didn't fit on this level
if (insertHead)
    {
    slReverse(&insertHead);
    calcPackSnakeHelper(insertHead,  level + 1);
    insertHead = NULL;
    }

// do we have any proposed blocks
if (proposedList == NULL) 
    return;

// we parsed all the blocks in the list to see if they fit in our level
// now let's see if the list of blocks for this level is big enough
// to actually add

struct snakeFeature *temp;
double scale = scaleForWindow(insideWidth, winStart, winEnd);
int start, end;

// order the blocks so lowest start position is first
slReverse(&proposedList);

start=proposedList->start;
for(temp=proposedList; temp->next; temp = temp->next)
    ;
end=temp->end;

// calculate how big the string of blocks is in screen coordinates
double apparentSize = scale * (end - start);

if (apparentSize < 0)
    errAbort("size of a list of blocks should not be less than zero");

// check to see if the apparent size is big enough
if (apparentSize < 1)
    return;

// transfer proposedList to new block list
for(temp=proposedList; temp; temp = next)
    {
    next = temp->next;
    temp->next = NULL;
    slAddHead(&newList, temp);
    }
}

static void calcPackSnake(struct track *tg, void *item)
{
struct linkedFeatures  *lf = (struct linkedFeatures *)item;
if (lf->components == NULL)
    return;

// we use the codons field to keep track of whether we already
// calculated the height of this snake
if (lf->codons == NULL)
    {
    clearLevels();

    // this will destroy lf->components, and add to newList
    calcPackSnakeHelper((struct snakeFeature *)lf->components, 0);
    lf->components = (struct simpleFeature *)newList;
    newList = NULL;
    
    //slSort(&lf->components, snakeFeatureCmpQStart);

    struct snakeInfo *si;
    AllocVar(si);
    si->maxLevel = maxLevel;
    lf->codons = (struct simpleFeature *)si;
    }
}

static int snakeItemHeight(struct track *tg, void *item)
// return height of a single packed snake 
{
if ((item == NULL) || (tg->visibility == tvSquish) || (tg->visibility == tvDense)) 
    return 0;

struct linkedFeatures  *lf = (struct linkedFeatures *)item;
if (lf->components == NULL)
    return 0;

if (tg->visibility == tvFull) 
    calcFullSnake(tg, item);
else if (tg->visibility == tvPack) 
    calcPackSnake(tg, item);

struct snakeInfo *si = (struct snakeInfo *)lf->codons;
int lineHeight = tg->lineHeight ;
int multiplier = 1;

if (tg->visibility == tvFull)
    multiplier = 2;
return (si->maxLevel + 1) * (multiplier * lineHeight);
}

static int linkedFeaturesCmpScore(const void *va, const void *vb)
/* Help sort linkedFeatures by score */
{
const struct linkedFeatures *a = *((struct linkedFeatures **)va);
const struct linkedFeatures *b = *((struct linkedFeatures **)vb);
if (a->score > b->score)
    return -1;
else if (a->score < b->score)
    return 1;
return 0;
}

static int snakeHeight(struct track *tg, enum trackVisibility vis)
/* calculate height of all the snakes being displayed */
{
if (tg->networkErrMsg != NULL)
    {
    // we had a parallel load failure
    tg->drawItems = bigDrawWarning;
    tg->totalHeight = bigWarnTotalHeight;
    return bigWarnTotalHeight(tg, vis);
    }

if (vis == tvDense)
    return tg->lineHeight;

if (vis == tvSquish)
    return tg->lineHeight/2;

int height = DUP_LINE_HEIGHT + INSERT_TEXT_HEIGHT; 
struct slList *item = tg->items;

item = tg->items;

for (item=tg->items;item; item = item->next)
    {
    height += tg->itemHeight(tg, item);
    }
if (height < DUP_LINE_HEIGHT + tg->lineHeight)
    height = DUP_LINE_HEIGHT + tg->lineHeight;
return height;
}

static void snakeDraw(struct track *tg, int seqStart, int seqEnd,
        struct hvGfx *hvg, int xOff, int yOff, int width, 
        MgFont *font, Color color, enum trackVisibility vis)
/* Draw linked features items. */
{
struct slList *item;
int y;
struct linkedFeatures  *lf;
double scale = scaleForWindow(width, seqStart, seqEnd);
int height = snakeHeight(tg, vis);

hvGfxSetClip(hvg, xOff, yOff, width, height);

if ((tg->visibility == tvFull) || (tg->visibility == tvPack))
    {
    // score snakes by how many bases they cover
    for (item = tg->items; item != NULL; item = item->next)
	{
	lf = (struct linkedFeatures *)item;
	struct snakeFeature  *sf;

	lf->score = 0;
	for (sf =  (struct snakeFeature *)lf->components; sf != NULL;  sf = sf->next)
	    {
	    lf->score += sf->end - sf->start;
	    }
	}

    slSort(&tg->items, linkedFeaturesCmpScore);
    }

y = yOff;
for (item = tg->items; item != NULL; item = item->next)
    {
    if(tg->itemColor != NULL) 
	color = tg->itemColor(tg, item, hvg);
    tg->drawItemAt(tg, item, hvg, xOff, y, scale, font, color, vis);
    if (vis == tvFull)
	y += tg->itemHeight(tg, item);
    } 
}

//  this is a 16 color palette with every other color being a lighter version of
//  the color before it
static int snakePalette2[] =
{
0x1f77b4, 0xaec7e8, 0xff7f0e, 0xffbb78, 0x2ca02c, 0x98df8a, 0xd62728, 0xff9896, 0x9467bd, 0xc5b0d5, 0x8c564b, 0xc49c94, 0xe377c2, 0xf7b6d2, 0x7f7f7f, 0xc7c7c7, 0xbcbd22, 0xdbdb8d, 0x17becf, 0x9edae5
};

static int snakePalette[] =
{
0x1f77b4, 0xff7f0e, 0x2ca02c, 0xd62728, 0x9467bd, 0x8c564b, 0xe377c2, 0x7f7f7f, 0xbcbd22, 0x17becf
};

static Color hashColor(char *name)
{
bits32 hashVal = hashString(name);
unsigned int colorInt = snakePalette2[hashVal % (sizeof(snakePalette2)/sizeof(Color))];

return MAKECOLOR_32(((colorInt >> 16) & 0xff),((colorInt >> 8) & 0xff),((colorInt >> 0) & 0xff));
}

static void boundMapBox(struct hvGfx *hvg, int start, int end, int x, int y, int width, int height,
                       char *track, char *item, char *statusLine, char *directUrl, boolean withHgsid,
                       char *extra)
// make sure start x and end x position are on the screen
// otherwise the tracking box code gets confused
{
if (x > insideWidth)
    return;

if (x < 0)
    {
    width -= -x;
    x = 0;
    }

if (x + width > insideWidth)
    {
    width -= x + width - insideWidth;
    }

mapBoxHgcOrHgGene(hvg, start, end, x, y, width, height,
                       track, item, statusLine, directUrl, withHgsid,
                       extra);
}

static void snakeDrawAt(struct track *tg, void *item,
	struct hvGfx *hvg, int xOff, int y, double scale, 
	MgFont *font, Color color, enum trackVisibility vis)
/* Draw a single simple bed item at position. */
{
unsigned showSnpWidth = cartOrTdbInt(cart, tg->tdb, 
    SNAKE_SHOW_SNP_WIDTH, SNAKE_DEFAULT_SHOW_SNP_WIDTH);
struct linkedFeatures  *lf = (struct linkedFeatures *)item;


if (tg->visibility == tvFull) 
    calcFullSnake(tg, item);
else if (tg->visibility == tvPack)
    calcPackSnake(tg, item);

if (lf->components == NULL)
    return;

boolean isHalSnake = lf->isHalSnake;

struct snakeFeature  *sf = (struct snakeFeature *)lf->components, *prevSf = NULL;
int s = tg->itemStart(tg, item);
int sClp = (s < winStart) ? winStart : s;
int x1 = round((sClp - winStart)*scale) + xOff;
int textX = x1;
int yOff = y;
boolean withLabels = (withLeftLabels && (vis == tvFull) && !tg->drawName);
unsigned   labelColor = MG_BLACK;

// draw the labels
if (withLabels)
    {
    char *name = tg->itemName(tg, item);
    int nameWidth = mgFontStringWidth(font, name);
    int dotWidth = tl.nWidth/2;
    boolean snapLeft = FALSE;
    boolean drawNameInverted = FALSE;
    textX -= nameWidth + dotWidth;
    snapLeft = (textX < fullInsideX);
    /* Special tweak for expRatio in pack mode: force all labels
     * left to prevent only a subset from being placed right: */
    snapLeft |= (startsWith("expRatio", tg->tdb->type));
#ifdef IMAGEv2_NO_LEFTLABEL_ON_FULL
    if (theImgBox == NULL && snapLeft)
#else///ifndef IMAGEv2_NO_LEFTLABEL_ON_FULL
    if (snapLeft)        /* Snap label to the left. */
#endif ///ndef IMAGEv2_NO_LEFTLABEL_ON_FULL
        {
        textX = leftLabelX;
        assert(hvgSide != NULL);
        hvGfxUnclip(hvgSide);
        hvGfxSetClip(hvgSide, leftLabelX, yOff, fullInsideX - leftLabelX, tg->height);
        if(drawNameInverted)
            {
            int boxStart = leftLabelX + leftLabelWidth - 2 - nameWidth;
            hvGfxBox(hvgSide, boxStart, y, nameWidth+1, tg->heightPer - 1, color);
            hvGfxTextRight(hvgSide, leftLabelX, y, leftLabelWidth-1, tg->heightPer,
                        MG_WHITE, font, name);
            }
        else
            hvGfxTextRight(hvgSide, leftLabelX, y, leftLabelWidth-1, tg->heightPer,
                        labelColor, font, name);
        hvGfxUnclip(hvgSide);
        hvGfxSetClip(hvgSide, insideX, yOff, insideWidth, tg->height);
        }
    else
        {
        int pdfSlop=nameWidth/5;
        hvGfxUnclip(hvg);
        hvGfxSetClip(hvg, textX-1-pdfSlop, y, nameWidth+1+pdfSlop, tg->heightPer);
        if(drawNameInverted)
            {
            hvGfxBox(hvg, textX - 1, y, nameWidth+1, tg->heightPer-1, color);
            hvGfxTextRight(hvg, textX, y, nameWidth, tg->heightPer, MG_WHITE, font, name);
            }
        else
            hvGfxTextRight(hvg, textX, y, nameWidth, tg->heightPer, labelColor, font, name);
        hvGfxUnclip(hvg);
        hvGfxSetClip(hvg, insideX, yOff, insideWidth, tg->height);
        }
    }

// let's draw some blue bars for the duplications
struct hal_target_dupe_list_t* dupeList = lf->dupeList;

int count = 0;
if ((tg->visibility == tvFull) || (tg->visibility == tvPack)) 
    {
    for(; dupeList ; dupeList = dupeList->next, count++)
	{
	struct hal_target_range_t *range = dupeList->tRange;

	unsigned int colorInt = snakePalette[count % (sizeof(snakePalette)/sizeof(Color))];
	Color color = MAKECOLOR_32(((colorInt >> 16) & 0xff),((colorInt >> 8) & 0xff),((colorInt >> 0) & 0xff));

	for(; range; range = range->next)
	    {
	    int s = range->tStart;
	    int e = range->tStart + range->size;
	    int sClp = (s < winStart) ? winStart : s;
	    int eClp = (e > winEnd) ? winEnd : e;
	    int x1 = round((sClp - winStart)*scale) + xOff;
	    int x2 = round((eClp - winStart)*scale) + xOff;
	    hvGfxBox(hvg, x1, y , x2-x1, DUP_LINE_HEIGHT - 1 , color);
	    }
	}
    y+=DUP_LINE_HEIGHT;
    }

// now we're going to draw the boxes

s = sf->start;
int lastE = -1;
int lastS = -1;
int offY = y;
int lineHeight = tg->lineHeight ;
int  qs, qe;
int heightPer = tg->heightPer;
int lastX = -1;
int lastQEnd = 0;
int lastLevel = -1;
int e;
qe = lastQEnd = 0;
for (sf =  (struct snakeFeature *)lf->components; sf != NULL; lastQEnd = qe, prevSf = sf, sf = sf->next)
    {
    qs = sf->qStart;
    qe = sf->qEnd;
    if (vis == tvDense)
	y = offY;
    else if ((vis == tvPack) || (vis == tvSquish))
	y = offY + (sf->level * 1) * lineHeight;
    else if (vis == tvFull)
	y = offY + (sf->level * 2) * lineHeight;
    s = sf->start; e = sf->end;

    int sx, ex;
    if (!positiveRangeIntersection(winStart, winEnd, s, e))
	continue;
    sx = round((double)((int)s-winStart)*scale) + xOff;
    ex = round((double)((int)e-winStart)*scale) + xOff;

    // color by strand
    static Color darkBlueColor = 0;
    static Color darkRedColor = 0;
    if (darkRedColor == 0)
	{
	//the light blue: rgb(149, 204, 252)
	//the light red: rgb(232, 156, 156)
	darkRedColor = hvGfxFindColorIx(hvg, 232,156,156);
	darkBlueColor = hvGfxFindColorIx(hvg, 149,204,252);
	}
    
    char *colorBy = cartOrTdbString(cart, tg->tdb, 
	SNAKE_COLOR_BY, SNAKE_DEFAULT_COLOR_BY);

    extern Color getChromColor(char *name, struct hvGfx *hvg);
    if (sameString(colorBy, SNAKE_COLOR_BY_STRAND_VALUE))
	color = (sf->orientation == -1) ? darkRedColor : darkBlueColor;
    else if (sameString(colorBy, SNAKE_COLOR_BY_CHROM_VALUE))
	color = hashColor(sf->qName);
    else
	color =  darkBlueColor;

    int w = ex - sx;
    if (w == 0) 
	w = 1;
    assert(w > 0);
    char buffer[1024];
	
    if (vis == tvFull)
	safef(buffer, sizeof buffer, "%d-%d",sf->qStart,sf->qEnd);
    else
	safef(buffer, sizeof buffer, "%s:%d-%d",sf->qName,sf->qStart,sf->qEnd);
    if (sx < insideX)
	{
	int olap = insideX - sx;
	sx = insideX;
	w -= olap;
	}
    char qAddress[4096];
    if ((vis == tvFull) || (vis == tvPack) )
	{
	safef(qAddress, sizeof qAddress, "qName=%s&qs=%d&qe=%d&qWidth=%d",sf->qName,  qs, qe,  winEnd - winStart);
	boundMapBox(hvg, s, e, sx+1, y, w-2, heightPer, tg->track,
		    buffer, buffer, NULL, TRUE, qAddress);
	}
    hvGfxBox(hvg, sx, y, w, heightPer, color);

    // now draw the mismatches if we're at high enough resolution 
    if ((isHalSnake && sf->qSequence != NULL) && (winBaseCount < showSnpWidth) && ((vis == tvFull) || (vis == tvPack)))
	{
	char *twoBitString = trackDbSetting(tg->tdb, "twoBit");
	static struct twoBitFile *tbf = NULL;
	static char *lastTwoBitString = NULL;
	static struct dnaSeq *seq = NULL;
	static char *lastQName = NULL;

	// sequence for chain snakes is in 2bit files which we cache
	if (!isHalSnake)
	    {
	    if (twoBitString == NULL)
		twoBitString = "/gbdb/hg19/hg19.2bit";

	    if ((lastTwoBitString == NULL) ||
		differentString(lastTwoBitString, twoBitString))
		{
		if (tbf != NULL)
		    {
		    lastQName = NULL;
		    twoBitClose(&tbf);
		    }
		tbf = twoBitOpen(twoBitString);
		}

	    // we're reading in the whole chrom
	    if ((lastQName == NULL) || differentString(sf->qName, lastQName))
		seq = twoBitReadSeqFrag(tbf, sf->qName,  0, 0);
	    lastQName = sf->qName;
	    lastTwoBitString = twoBitString;
	    }

	char *ourDna;
	if (isHalSnake)
	    ourDna = sf->qSequence;
	else
	    ourDna = &seq->dna[sf->qStart];

	int seqLen = sf->qEnd - sf->qStart;
	toUpperN(ourDna, seqLen);
	if (!isHalSnake && (sf->orientation == -1))
	    reverseComplement(ourDna,seqLen);

	// get the reference sequence
	char *refDna;
        if (isHalSnake && differentString(tg->tdb->type, "pslSnake"))
	    {
	    refDna = sf->tSequence;
	    }
	else
	    {
	    struct dnaSeq *extraSeq = hDnaFromSeq(database, chromName, sf->start, sf->end, dnaUpper);
	    refDna = extraSeq->dna;
	    }
	int si = s;
	char *ptr1 = refDna;
	char *ptr2 = ourDna;
	for(; si < e; si++,ptr1++,ptr2++)
	    {
	    // if mismatch!  If reference is N ignore, if query is N, paint yellow
	    if ( (*ptr1 != *ptr2) && !((*ptr1 == 'N') || (*ptr1 == 'n')))
		{
		int misX1 = round((double)((int)si-winStart)*scale) + xOff;
		int misX2 = round((double)((int)(si+1)-winStart)*scale) + xOff;
		int w1 = misX2 - misX1;
		if (w1 < 1)
		    w1 = 1;

		Color boxColor = MG_RED;
		if ((*ptr2 == 'N') || (*ptr2 == 'n'))
		    boxColor = hvGfxFindRgb(hvg, &undefinedYellowColor);
		hvGfxBox(hvg, misX1, y, w1, heightPer, boxColor);
		}
	    }

	// if we're zoomed to base level, draw sequence of mismatch
	if (zoomedToBaseLevel)
	    {
	    int mysx = round((double)((int)s-winStart)*scale) + xOff;
	    int myex = round((double)((int)e-winStart)*scale) + xOff;
	    int myw = myex - mysx;
	    spreadAlignString(hvg, mysx, y, myw, heightPer, MG_WHITE, font, ourDna,
		refDna, seqLen, TRUE, FALSE);
	    }

	}
    sf->drawn = TRUE;
    lastLevel = sf->level;
    //lastX = x;
    }

if (vis != tvFull)
    return;

// now we're going to draw the lines between the blocks

lastX = -1;
lastQEnd = 0;
lastLevel = 0;
qe = lastQEnd = 0;
prevSf = NULL;
for (sf =  (struct snakeFeature *)lf->components; sf != NULL; lastQEnd = qe, prevSf = sf, sf = sf->next)
    {
    int y1, y2;
    int sx, ex;
    qs = sf->qStart;
    qe = sf->qEnd;
    if (lastLevel == sf->level)
	{
	y1 = offY + (lastLevel * 2) * lineHeight + lineHeight/2;
	y2 = offY + (sf->level * 2) * lineHeight + lineHeight/2;
	}
    else if (lastLevel > sf->level)
	{
	y1 = offY + (lastLevel * 2 ) * lineHeight;
	y2 = offY + (sf->level * 2 + 1) * lineHeight + lineHeight/3;
	}
    else
	{
	y1 = offY + (lastLevel * 2 + 1) * lineHeight - 1;
	y2 = offY + (sf->level * 2 ) * lineHeight - lineHeight/3;;
	}
    s = sf->start; e = sf->end;

    sx = round((double)((int)s-winStart)*scale) + xOff;
    ex = round((double)((int)e-winStart)*scale) + xOff;
    color = (sf->orientation == -1) ? MG_RED : MG_BLUE;

    if (lastX != -1)
	{
	char buffer[1024];
#define MG_ORANGE  0xff0082E6
	int color = MG_GRAY;

	if (lastQEnd != qs) {
            long long queryInsertSize = llabs(lastQEnd - qs);
            long long targetInsertSize;
            if (sf->orientation == 1)
                targetInsertSize = s - lastE;
            else
                targetInsertSize = lastS - e;
            int blue = 0;
            int red = 0;
            int green = 0;
            if (queryInsertSize > targetInsertSize) {
                double frac = ((double) queryInsertSize - targetInsertSize) / targetInsertSize;
                if (frac > 1.0)
                    frac = 1.0;
                red = 255 - 255 * frac;
                blue = 255 * frac;
            } else {
                double frac = ((double) targetInsertSize - queryInsertSize) / targetInsertSize;
                if (frac > 1.0)
                    frac = 1.0;
                red = 255 - 255 * frac;
                green = 255 * frac;
            }
            color = hvGfxFindColorIx(hvg, red, green, blue);
        }
        double queryGapNFrac = 0.0;
        double queryGapMaskedFrac = 0.0;
        if ((qs > lastQEnd) && qs - lastQEnd < 1000000) {
            // sketchy
            char *fileName = trackDbSetting(tg->tdb, "bigDataUrl");
            char *otherSpecies = trackDbSetting(tg->tdb, "otherSpecies");
            int handle = halOpenLOD(fileName, NULL);
            char *queryGapDna = halGetDna(handle, otherSpecies, sf->qName, lastQEnd, qs, NULL);
            long long numNs = 0;
            long long numMasked = 0;
            char *i = queryGapDna;
            while (*i != '\0') {
                if (*i == 'N' || *i == 'n') {
                    numNs++;
                    numMasked++;
                }
                if (*i == 'a' || *i == 't' || *i == 'g' || *i == 'c') {
                        numMasked++;
                }
                i++;
            }
            free(queryGapDna);
            queryGapMaskedFrac = ((double) numMasked) / (qs - lastQEnd);
            queryGapNFrac = ((double) numNs) / (qs - lastQEnd);
        }

	// draw the vertical orange bars if there is an insert in the other sequence
	if ((winBaseCount < showSnpWidth) )
	    {
	    if ((sf->orientation == 1) && (qs != lastQEnd) && (lastE == s))
		{
		hvGfxLine(hvg, sx, y2 - lineHeight/2 , sx, y2 + lineHeight/2, MG_ORANGE);
		safef(buffer, sizeof buffer, "%dbp (%.1lf%% N, %.1lf%% masked)", qs - lastQEnd, queryGapNFrac*100, queryGapMaskedFrac*100);
		boundMapBox(hvg, s, e, sx, y2 - lineHeight/2, 1, lineHeight, tg->track,
				    "foo", buffer, NULL, TRUE, NULL);
		safef(buffer, sizeof buffer, "%dbp", qs - lastQEnd);
                hvGfxTextCentered(hvg, sx - 10, y2 + lineHeight/2, 20, INSERT_TEXT_HEIGHT, MG_ORANGE, font, buffer);
		}
	    else if ((sf->orientation == -1) && (qs != lastQEnd) && (lastS == e))
		{
		hvGfxLine(hvg, ex, y2 - lineHeight/2 , ex, y2 + lineHeight/2, MG_ORANGE);
		safef(buffer, sizeof buffer, "%dbp (%.1lf%% N, %.1lf%% masked)", qs - lastQEnd, queryGapNFrac*100, queryGapMaskedFrac*100);
		boundMapBox(hvg, s, e, ex, y2 - lineHeight/2, 1, lineHeight, tg->track,
				    "foo", buffer, NULL, TRUE, NULL);
		safef(buffer, sizeof buffer, "%dbp", qs - lastQEnd);
                hvGfxTextCentered(hvg, ex - 10, y2 + lineHeight/2, 20, INSERT_TEXT_HEIGHT, MG_ORANGE, font, buffer);
		}
	    }

	// now draw the lines between blocks
	if ((!((lastX == sx) && (y1 == y2))) &&
	    (sf->drawn  || ((prevSf != NULL) && (prevSf->drawn))) &&
	    (((lastE >= winStart) && (lastE <= winEnd)) || 
	    ((s > winStart) && (s < winEnd))))
	    {
	    if (lastLevel == sf->level)
		{
                if (sf->orientation == 1)
                    safef(buffer, sizeof buffer, "%dbp (%dbp in ref) (%.1lf%% N, %.1lf%% masked)", qs - lastQEnd, s - lastE, queryGapNFrac*100, queryGapMaskedFrac*100);
                else
                    safef(buffer, sizeof buffer, "%dbp (%dbp in ref) (%.1lf%% N, %.1lf%% masked)", qs - lastQEnd, lastS - e, queryGapNFrac*100, queryGapMaskedFrac*100);
		if (sf->orientation == -1)
		    {
		    if (lastX != ex)
			{
			hvGfxLine(hvg, ex, y1, lastX, y2, color);
			boundMapBox(hvg, s, e, ex, y1, lastX-ex, 1, tg->track,
				"", buffer, NULL, TRUE, NULL);
                        if (lastQEnd != qs) {
                            safef(buffer, sizeof buffer, "%dbp", qs - lastQEnd);
                            hvGfxTextCentered(hvg, ex, y2 + lineHeight/2, lastX-ex, INSERT_TEXT_HEIGHT, MG_ORANGE, font, buffer);
                        }
			}
		    }
		else
		    {
		    if (lastX != sx)
			{
			hvGfxLine(hvg, lastX, y1, sx, y2, color);
			boundMapBox(hvg, s, e, lastX, y1, sx-lastX, 1, tg->track,
				"", buffer, NULL, TRUE, NULL);
                        if (lastQEnd != qs) {
                            safef(buffer, sizeof buffer, "%dbp", qs - lastQEnd);
                            hvGfxTextCentered(hvg, lastX, y2 + lineHeight/2, sx-lastX, INSERT_TEXT_HEIGHT, MG_ORANGE, font, buffer);
                        }
			}
		    }
		}
	    else if (lastLevel > sf->level)
		{
		hvGfxLine(hvg, lastX, y1, sx, y2, color);
		hvGfxLine(hvg, sx, y2, sx, y2 - lineHeight - lineHeight/3, color);
		char buffer[1024];
		safef(buffer, sizeof buffer, "%d-%d %dbp gap (%.1lf%% N, %.1lf%% masked)",prevSf->qStart,prevSf->qEnd, qs - lastQEnd, queryGapNFrac*100, queryGapMaskedFrac*100);
		boundMapBox(hvg, s, e, sx, y2 - lineHeight - lineHeight/3, 2, lineHeight + lineHeight/3, tg->track,
	                    "", buffer, NULL, TRUE, NULL);
		safef(buffer, sizeof buffer, "%dbp", qs - lastQEnd);
                if (lastQEnd != qs)
                    hvGfxTextCentered(hvg, sx - 10, y2 + lineHeight/2, 20, INSERT_TEXT_HEIGHT, MG_ORANGE, font, buffer);

		}
	    else
		{
		char buffer[1024];
		safef(buffer, sizeof buffer, "%d-%d %dbp gap (%.1lf%% N, %.1lf%% masked)",prevSf->qStart,prevSf->qEnd, qs - lastQEnd, queryGapNFrac*100, queryGapMaskedFrac*100);
		if (sf->orientation == -1)
		    {
		    hvGfxLine(hvg, lastX-1, y1, ex, y2, color);
		    hvGfxLine(hvg, ex, y2, ex, y2 + lineHeight , color);
		    boundMapBox(hvg, s, e, ex-1, y2, 2, lineHeight , tg->track,
				"", buffer, NULL, TRUE, NULL);
                    safef(buffer, sizeof buffer, "%dbp", qs - lastQEnd);
                    if (lastQEnd != qs)
                        hvGfxTextCentered(hvg, ex - 10, y2 + lineHeight, 20, INSERT_TEXT_HEIGHT, MG_ORANGE, font, buffer);
		    }
		else
		    {
		    hvGfxLine(hvg, lastX-1, y1, sx, y2, color);
		    hvGfxLine(hvg, sx, y2, sx, y2 + lineHeight , color);
		    boundMapBox(hvg, s, e, sx-1, y2, 2, lineHeight , tg->track,
				"", buffer, NULL, TRUE, NULL);

                    safef(buffer, sizeof buffer, "%dbp", qs - lastQEnd);
                    if (lastQEnd != qs)
                        hvGfxTextCentered(hvg, sx - 10, y2 + lineHeight, 20, INSERT_TEXT_HEIGHT, MG_ORANGE, font, buffer);
		    }
		}
	    }
	}
    if (sf->orientation == -1)
	lastX = sx;
    else
	lastX = ex;
    lastS = s;
    lastE = e;
    lastLevel = sf->level;
    lastQEnd = qe;
    }
}

static char *doChromIxSearch(char *trixFile, char *searchName)
/* search ixFile for the searchName, return name if found */
{
struct trix *trix = trixOpen(trixFile);
char *trixWords[1];
int trixWordCount = 1;
trixWords[0] = strLower(searchName);
char *name = NULL;  // assume NOT found

struct trixSearchResult *tsList = trixSearch(trix, trixWordCount, trixWords, tsmExact);
if (tsList)
   name = tsList->itemId;  // FOUND
return name;
}

static struct hal_block_results_t *pslSnakeBlocks(char *fileName, struct track *track, char *chrom, unsigned start, unsigned end, unsigned int maxItems)
/* create HAL-like blocks from a bigPsl file. */
{
struct hal_block_results_t *head;

AllocVar(head);

struct lm *lm = lmInit(0);
//struct bigBedInterval *bb, *bbList = bigBedSelectRangeExt(track, chrom, start, end, lm, maxItems);
struct bigBedInterval *bb, *bbList = bigBedSelectRangeExt(track, chrom, 0, hChromSize(database,chromName), lm, maxItems);

struct bbiFile *bbi = fetchBbiForTrack(track);
int seqTypeField =  0;
if (sameString(track->tdb->type, "bigPsl"))
    {
    seqTypeField =  bbExtraFieldIndex(bbi, "seqType");
    }
bbiFileClose(&bbi);
for (bb = bbList; bb != NULL; bb = bb->next)
    {
    char *seq, *cds;
    struct psl *psl = pslFromBigPsl(chromName, bb, seqTypeField,  &seq, &cds); 
    unsigned *targetStart = psl->tStarts;
    unsigned *queryStart = psl->qStarts;
    unsigned *size = psl->blockSizes;
    int ii;

    if ((seq != NULL) && (psl->strand[0] == '-'))
        reverseComplement(seq, strlen(seq));
    for(ii = 0; ii < psl->blockCount; ii++)
        {
        struct hal_block_t *block;
        AllocVar(block);
        slAddHead(&head->mappedBlocks, block);

        block->qChrom = psl->qName;
        block->tStart = *targetStart++;
        block->qStart = *queryStart++;
        block->size = *size++;
        block->strand = psl->strand[0];
        block->qSequence = &seq[block->qStart];

        if (block->strand == '-')
            block->qStart = psl->qSize - block->qStart;
        }
    }

return head;
}

void halSnakeLoadItems(struct track *tg)
// load up a snake from a HAL file.   This code is called in threads
// so *no* use of globals please. All but full snakes are read into a single
// linked feature.
{
unsigned showSnpWidth = cartOrTdbInt(cart, tg->tdb, 
    SNAKE_SHOW_SNP_WIDTH, SNAKE_DEFAULT_SHOW_SNP_WIDTH);

char * chromAlias = NULL;	// create later when needed
char * chromAliasFile = trackDbSetting(tg->tdb, "searchTrix");
if (chromAliasFile)
   chromAlias = doChromIxSearch(chromAliasFile, chromName);

char *aliasName = chromName;
if (chromAlias)
   {
       if (differentWord(chromAlias, aliasName))
          aliasName = chromAlias;
   }

boolean isPsl = sameString(tg->tdb->type, "pslSnake");

// if we have a network error we want to put out a message about it
struct errCatch *errCatch = errCatchNew();
if (errCatchStart(errCatch))
    {
    char *fileName = trackDbSetting(tg->tdb, "bigDataUrl");
    char *otherSpecies = trackDbSetting(tg->tdb, "otherSpecies");
    char *errString = "<HAL error message not set>";
    int handle = -1;
    if (!isPsl)
        {
        handle = halOpenLOD(fileName, &errString);
        if (handle < 0)
            {
            errAbort("HAL open error: %s\n", errString);
            goto out;
            }
        }
    boolean isPackOrFull = (tg->visibility == tvFull) || 
	(tg->visibility == tvPack);
    hal_dup_type_t dupMode =  (isPackOrFull) ? HAL_QUERY_AND_TARGET_DUPS :
	HAL_QUERY_DUPS;
    hal_seqmode_type_t needSeq = isPackOrFull && (winBaseCount < showSnpWidth) ? HAL_LOD0_SEQUENCE : HAL_NO_SEQUENCE;
    int mapBackAdjacencies = (tg->visibility == tvFull);
    char codeVarName[1024];
    safef(codeVarName, sizeof codeVarName, "%s.coalescent", tg->tdb->track);
    char *coalescent = cartOptionalString(cart, codeVarName);
    char *otherDbName = trackHubSkipHubName(database);
    struct hal_block_results_t *head = NULL;
    if (isPsl)
        {
        head = pslSnakeBlocks(fileName, tg, chromName, winStart, winEnd, 10000000);
        }
    else 
        {
        head = halGetBlocksInTargetRange(handle, otherSpecies, otherDbName, aliasName, winStart, winEnd, 0, needSeq, dupMode,mapBackAdjacencies, coalescent, &errString);
        }

    // did we get any blocks from HAL
    if (head == NULL)
	{
	errAbort("HAL get blocks error: %s\n", errString);
	goto out;
	}
    struct hal_block_t* cur = head->mappedBlocks;
    struct linkedFeatures *lf = NULL;
    struct hash *qChromHash = newHash(5);
    struct linkedFeatures *lfList = NULL;
    char buffer[4096];

#ifdef NOTNOW
    struct hal_target_dupe_list_t* targetDupeBlocks = head->targetDupeBlocks;

    for(;targetDupeBlocks; targetDupeBlocks = targetDupeBlocks->next)
	{
	printf("<br>id: %d qChrom %s\n", targetDupeBlocks->id, targetDupeBlocks->qChrom);
	struct hal_target_range_t *range = targetDupeBlocks->tRange;
	for(; range; range = range->next)
	    {
	    printf("<br>   %ld : %ld\n", range->tStart, range->size);
	    }
	}
#endif

    while (cur)
    {
	struct hashEl* hel;

	if (tg->visibility == tvFull)
	    safef(buffer, sizeof buffer, "%s", cur->qChrom);
	else
	    {
	    // make sure the block is on the screen 
	    if (!positiveRangeIntersection(winStart, winEnd, cur->tStart,  cur->tStart + cur->size))
		{
		cur = cur->next;
		continue;
		}
	    safef(buffer, sizeof buffer, "allInOne");
	    }

	if ((hel = hashLookup(qChromHash, buffer)) == NULL)
	    {
	    AllocVar(lf);
	    lf->isHalSnake = TRUE;
	    slAddHead(&lfList, lf);
	    lf->start = 0;
	    lf->end = 1000000000;
	    lf->grayIx = maxShade;
	    lf->name = cloneString(buffer);
	    lf->extra = cloneString(buffer);
	    lf->orientation = (cur->strand == '+') ? 1 : -1;
	    hashAdd(qChromHash, lf->name, lf);

	    // now figure out where the duplication bars go
	    struct hal_target_dupe_list_t* targetDupeBlocks = head->targetDupeBlocks;

	    if ((tg->visibility == tvPack) || (tg->visibility == tvFull))
		for(;targetDupeBlocks; targetDupeBlocks = targetDupeBlocks->next)
		    {
		    if ((tg->visibility == tvPack) ||
			((tg->visibility == tvFull) &&
			 (sameString(targetDupeBlocks->qChrom, cur->qChrom))))
			{
			struct hal_target_dupe_list_t* dupeList;
			AllocVar(dupeList);
			*dupeList = *targetDupeBlocks;
			slAddHead(&lf->dupeList, dupeList);
			// TODO: should clone the target_range structures
			// rather than copying them
			}
		    }
	    }
	else
	    {
	    lf = hel->val;
	    }

	struct snakeFeature  *sf;
	AllocVar(sf);
	slAddHead(&lf->components, sf);
	
	sf->start = cur->tStart;
	sf->end = cur->tStart + cur->size;
	sf->qStart = cur->qStart;
	sf->qEnd = cur->qStart + cur->size;
	sf->orientation = (cur->strand == '+') ? 1 : -1;
	sf->tSequence = cloneString(cur->tSequence);
	sf->qSequence = cloneString(cur->qSequence);
	if (sf->tSequence != NULL)
	    toUpperN(sf->tSequence, strlen(sf->tSequence));
	if (sf->qSequence != NULL)
	    toUpperN(sf->qSequence, strlen(sf->qSequence));
	sf->qName = cur->qChrom;

	cur = cur->next;
    }
    if (tg->visibility == tvFull)
	{
	for(lf=lfList; lf ; lf = lf->next)
	    {
	    slSort(&lf->components, snakeFeatureCmpQStart);
	    }
	}
    else if ((tg->visibility == tvPack) && (lfList != NULL))
	{
	assert(lfList->next == NULL);
	slSort(&lfList->components, snakeFeatureCmpTStart);
	}
    
    //halFreeBlocks(head);
    //halClose(handle, myThread);

    tg->items = lfList;
    }

out:
errCatchEnd(errCatch);
if (errCatch->gotError)
    {
    tg->networkErrMsg = cloneString(errCatch->message->string);
    tg->drawItems = bigDrawWarning;
    tg->totalHeight = bigWarnTotalHeight;
    }
errCatchFree(&errCatch);
}

void halSnakeDrawLeftLabels(struct track *tg, int seqStart, int seqEnd,
        struct hvGfx *hvg, int xOff, int yOff, int width, int height,
        boolean withCenterLabels, MgFont *font,
        Color color, enum trackVisibility vis)
/* Draw left label (shortLabel) in pack and dense modes. */
{
if ((vis == tvDense) ||  (vis == tvPack))
    {
    hvGfxSetClip(hvgSide, leftLabelX, yOff + tg->lineHeight, insideWidth, tg->height);
    hvGfxTextRight(hvgSide, leftLabelX, yOff + tg->lineHeight, leftLabelWidth-1, tg->lineHeight,
                   color, font, tg->shortLabel);
    hvGfxUnclip(hvgSide);
    }
}

void halSnakeMethods(struct track *tg, struct trackDb *tdb, 
	int wordCount, char *words[])
{
linkedFeaturesMethods(tg);
tg->canPack = tdb->canPack = TRUE;
tg->loadItems = halSnakeLoadItems;
tg->drawItems = snakeDraw;
tg->mapItemName = lfMapNameFromExtra;
tg->subType = lfSubChain;
//tg->extraUiData = (void *) chainCart;
tg->totalHeight = snakeHeight; 
tg->drawLeftLabels = halSnakeDrawLeftLabels;

tg->drawItemAt = snakeDrawAt;
tg->itemHeight = snakeItemHeight;
tg->nextItemButtonable = FALSE;
}
#endif  // USE_HAL

#ifdef NOTNOW

// from here down are routines to support the visualization of chains as snakes
// this code is currently BROKEN, and may be removed completely in the future

struct cartOptions
    {
    enum chainColorEnum chainColor; /*  ChromColors, ScoreColors, NoColors */
    int scoreFilter ; /* filter chains by score if > 0 */
    };

// mySQL code to read in chains

static void doQuery(struct sqlConnection *conn, char *fullName, 
			struct lm *lm, struct hash *hash, 
			int start, int end,  boolean isSplit, int chainId)
/* doQuery- check the database for chain elements between
 * 	start and end.  Use the passed hash to resolve chain
 * 	id's and place the elements into the right
 * 	linkedFeatures structure
 */
{
struct sqlResult *sr = NULL;
char **row;
struct linkedFeatures *lf;
struct snakeFeature *sf;
struct dyString *query = newDyString(1024);
char *force = "";

if (isSplit)
    force = "force index (bin)";

if (chainId == -1)
    sqlDyStringPrintf(query, 
	"select chainId,tStart,tEnd,qStart from %sLink %-s where ",
	fullName, force);
else
    sqlDyStringPrintf(query, 
	"select chainId, tStart,tEnd,qStart from %sLink where chainId=%d and ",
	fullName, chainId);
if (!isSplit)
    sqlDyStringPrintf(query, "tName='%s' and ", chromName);
hAddBinToQuery(start, end, query);
dyStringPrintf(query, "tStart<%u and tEnd>%u", end, start);
sr = sqlGetResult(conn, query->string);

/* Loop through making up simple features and adding them
 * to the corresponding linkedFeature. */
while ((row = sqlNextRow(sr)) != NULL)
    {
    lf = hashFindVal(hash, row[0]);
    if (lf != NULL)
	{
	struct chain *pChain = lf->extra;
	lmAllocVar(lm, sf);
	sf->start = sqlUnsigned(row[1]);
	sf->end = sqlUnsigned(row[2]);
	sf->qStart = sqlUnsigned(row[3]); 

	sf->qEnd = sf->qStart + (sf->end - sf->start);
	if ((pChain) && pChain->qStrand == '-')
	    {
	    int temp;

	    temp = sf->qStart;
	    sf->qStart = pChain->qSize - sf->qEnd;
	    sf->qEnd = pChain->qSize - temp;
	    }
	sf->orientation = lf->orientation;
	slAddHead(&lf->components, sf);
	}
    }
sqlFreeResult(&sr);
dyStringFree(&query);
}

static void fixItems(struct linkedFeatures *lf)
// put all chain blocks from a single query chromosome into one
// linkedFeatures structure
{
struct linkedFeatures *firstLf, *next;
struct snakeFeature  *sf,  *nextSf;

firstLf = lf;
for (;lf; lf = next)
    {
    next = lf->next;
    if (!sameString(firstLf->name, lf->name) && (lf->components != NULL))
	{
	slSort(&firstLf->components, snakeFeatureCmpQStart);
	firstLf = lf;
	}
    for (sf =  (struct snakeFeature *)lf->components; sf != NULL; sf = nextSf)
	{
	sf->qName = lf->name;
	sf->orientation = lf->orientation;
	nextSf = sf->next;
	if (firstLf != lf)
	    {
	    lf->components = NULL;
	    slAddHead(&firstLf->components, sf);
	    }
	}
    }

if (firstLf != NULL)
    {
    slSort(&firstLf->components, snakeFeatureCmpQStart);
    firstLf->next = 0;
    }
}


static void loadLinks(struct track *tg, int seqStart, int seqEnd,
         enum trackVisibility vis)
// load up the chain elements into linkedFeatures
{
int start, end, extra;
char fullName[64];
int maxOverLeft = 0, maxOverRight = 0;
int overLeft, overRight;
struct linkedFeatures *lf;
struct lm *lm;
struct hash *hash;	/* Hash of chain ids. */
struct sqlConnection *conn;
lm = lmInit(1024*4);
hash = newHash(0);
conn = hAllocConn(database);

/* Make up a hash of all linked features keyed by
 * id, which is held in the extras field.  */
for (lf = tg->items; lf != NULL; lf = lf->next)
    {
    char buf[256];
    struct chain *pChain = lf->extra;
    safef(buf, sizeof(buf), "%d", pChain->id);
    hashAdd(hash, buf, lf);
    overRight = lf->end - seqEnd;
    if (overRight > maxOverRight)
	maxOverRight = overRight;
    overLeft = seqStart - lf->start ;
    if (overLeft > maxOverLeft)
	maxOverLeft = overLeft;
    }

if (hash->size)
    {
    boolean isSplit = TRUE;
    /* Make up range query. */
    safef(fullName, sizeof fullName, "%s_%s", chromName, tg->table);
    if (!hTableExists(database, fullName))
	{
	strcpy(fullName, tg->table);
	isSplit = FALSE;
	}

    /* in dense mode we don't draw the lines 
     * so we don't need items off the screen 
     */
    if (vis == tvDense)
	doQuery(conn, fullName, lm,  hash, seqStart, seqEnd,  isSplit, -1);
    else
	{
	/* if chains extend beyond edge of window we need to get 
	 * elements that are off the screen
	 * in both directions so we know whether to draw
	 * one or two lines to the edge of the screen.
	 */
#define STARTSLOP	0
#define MULTIPLIER	10
#define MAXLOOK		100000
	extra = (STARTSLOP < maxOverLeft) ? STARTSLOP : maxOverLeft;
	start = seqStart - extra;
	extra = (STARTSLOP < maxOverRight) ? STARTSLOP : maxOverRight;
	end = seqEnd + extra;
	doQuery(conn, fullName, lm,  hash, start, end,  isSplit, -1);
	}
    }
hFreeConn(&conn);
}

void snakeLoadItems(struct track *tg)
// Load chains from a mySQL database
{
char *track = tg->table;
struct chain chain;
int rowOffset;
char **row;
struct sqlConnection *conn = hAllocConn(database);
struct sqlResult *sr = NULL;
struct linkedFeatures *list = NULL, *lf;
int qs;
char optionChr[128]; /* Option -  chromosome filter */
char *optionChrStr;
char extraWhere[128] ;
struct cartOptions *chainCart;
struct chain *pChain;

chainCart = (struct cartOptions *) tg->extraUiData;

safef( optionChr, sizeof(optionChr), "%s.chromFilter", tg->table);
optionChrStr = cartUsualString(cart, optionChr, "All");
int ourStart = winStart;
int ourEnd = winEnd;

// we're grabbing everything now.. we really should be 
// doing this as a preprocessing stage, rather than at run-time
ourStart = 0;
ourEnd = 500000000;
//ourStart = winStart;
//ourEnd = winEnd;
if (startsWith("chr",optionChrStr)) 
    {
    safef(extraWhere, sizeof(extraWhere), 
            "qName = \"%s\" and score > %d",optionChrStr, 
            chainCart->scoreFilter);
    sr = hRangeQuery(conn, track, chromName, ourStart, ourEnd, 
            extraWhere, &rowOffset);
    }
else
    {
    if (chainCart->scoreFilter > 0)
        {
        safef(extraWhere, sizeof(extraWhere), 
                "score > \"%d\"",chainCart->scoreFilter);
        sr = hRangeQuery(conn, track, chromName, ourStart, ourEnd, 
                extraWhere, &rowOffset);
        }
    else
        {
        safef(extraWhere, sizeof(extraWhere), " ");
        sr = hRangeQuery(conn, track, chromName, ourStart, ourEnd, 
                NULL, &rowOffset);
        }
    }
while ((row = sqlNextRow(sr)) != NULL)
    {
    chainHeadStaticLoad(row + rowOffset, &chain);
    AllocVar(pChain);
    *pChain = chain;
    AllocVar(lf);
    lf->start = lf->tallStart = chain.tStart;
    lf->end = lf->tallEnd = chain.tEnd;
    lf->grayIx = maxShade;
    if (chainCart->chainColor == chainColorScoreColors)
	{
	float normScore = sqlFloat((row+rowOffset)[11]);
	lf->grayIx = (int) ((float)maxShade * (normScore/100.0));
	if (lf->grayIx > (maxShade+1)) lf->grayIx = maxShade+1;
	lf->score = normScore;
	}
    else
	lf->score = chain.score;

    lf->filterColor = -1;

    if (chain.qStrand == '-')
	{
	lf->orientation = -1;
        qs = chain.qSize - chain.qEnd;
	}
    else
        {
	lf->orientation = 1;
	qs = chain.qStart;
	}
    char buffer[1024];
    safef(buffer, sizeof(buffer), "%s", chain.qName);
    lf->name = cloneString(buffer);
    lf->extra = pChain;
    slAddHead(&list, lf);
    }

/* Make sure this is sorted if in full mode. Sort by score when
 * coloring by score and in dense */
if (tg->visibility != tvDense)
    slSort(&list, linkedFeaturesCmpStart);
else if ((tg->visibility == tvDense) &&
	(chainCart->chainColor == chainColorScoreColors))
    slSort(&list, chainCmpScore);
else
    slReverse(&list);
tg->items = list;


/* Clean up. */
sqlFreeResult(&sr);
hFreeConn(&conn);

/* now load the items */
loadLinks(tg, ourStart, ourEnd, tg->visibility);
lf=tg->items;
fixItems(lf);
}	/*	chainLoadItems()	*/

void snakeMethods(struct track *tg, struct trackDb *tdb, 
	int wordCount, char *words[])
/* Fill in custom parts of alignment chains. */
{

struct cartOptions *chainCart;

AllocVar(chainCart);

boolean normScoreAvailable = chainDbNormScoreAvailable(tdb);

/*	what does the cart say about coloring option	*/
chainCart->chainColor = chainFetchColorOption(cart, tdb, FALSE);

chainCart->scoreFilter = cartUsualIntClosestToHome(cart, tdb,
	FALSE, SCORE_FILTER, 0);


linkedFeaturesMethods(tg);
tg->itemColor = lfChromColor;	/*	default coloring option */

/*	if normScore column is available, then allow coloring	*/
if (normScoreAvailable)
    {
    switch (chainCart->chainColor)
	{
	case (chainColorScoreColors):
	    tg->itemColor = chainScoreColor;
	    tg->colorShades = shadesOfGray;
	    break;
	case (chainColorNoColors):
	    setNoColor(tg);
	    break;
	default:
	case (chainColorChromColors):
	    break;
	}
    }
else
    {
    char option[128]; /* Option -  rainbow chromosome color */
    char *optionStr;	/* this old option was broken before */

    safef(option, sizeof(option), "%s.color", tg->table);
    optionStr = cartUsualString(cart, option, "on");
    if (differentWord("on",optionStr))
	{
	setNoColor(tg);
	chainCart->chainColor = chainColorNoColors;
	}
    else
	chainCart->chainColor = chainColorChromColors;
    }

tg->canPack = FALSE;
tg->loadItems = snakeLoadItems;
tg->drawItems = snakeDraw;
tg->mapItemName = lfMapNameFromExtra;
tg->subType = lfSubChain;
tg->extraUiData = (void *) chainCart;
tg->totalHeight = snakeHeight; 

tg->drawItemAt = snakeDrawAt;
tg->itemHeight = snakeItemHeight;
}

static Color chainScoreColor(struct track *tg, void *item, struct hvGfx *hvg)
{
struct linkedFeatures *lf = (struct linkedFeatures *)item;

return(tg->colorShades[lf->grayIx]);
}

static Color chainNoColor(struct track *tg, void *item, struct hvGfx *hvg)
{
return(tg->ixColor);
}

static void setNoColor(struct track *tg)
{
tg->itemColor = chainNoColor;
tg->color.r = 0;
tg->color.g = 0;
tg->color.b = 0;
tg->altColor.r = 127;
tg->altColor.g = 127;
tg->altColor.b = 127;
tg->ixColor = MG_BLACK;
tg->ixAltColor = MG_GRAY;
}
#endif
