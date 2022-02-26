/* wigCommon.h - common items to the two graphing types of tracks
 *	type wig and type bedGraph
 */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#ifndef WIGCOMMON_H
#define WIGCOMMON_H


/*	wigCartOptions structure - to carry cart options from wigMethods
 *	to all the other methods via the track->extraUiData pointer
 */
struct wigCartOptions
    {
    enum wiggleGridOptEnum horizontalGrid;	/*  grid lines, ON/OFF */
    enum wiggleGraphOptEnum lineBar;		/*  Line or Bar chart */
    enum wiggleScaleOptEnum autoScale;		/*  autoScale on */
    enum wiggleWindowingEnum windowingFunction;	/*  max,mean,min */
    enum wiggleSmoothingEnum smoothingWindow;	/*  N: [1:15] */
    enum wiggleYLineMarkEnum yLineOnOff;	/*  OFF/ON	*/
    enum wiggleAlwaysZeroEnum alwaysZero;	/*  OFF/ON	*/
    enum wiggleTransformFuncEnum transformFunc;	/*  NONE/LOG	*/
    double minY;	/*	from trackDb.ra words, the absolute minimum */
    double maxY;	/*	from trackDb.ra words, the absolute maximum */
    int maxHeight;	/*	maximum pixels height from trackDb	*/
    int defaultHeight;	/*	requested height from cart	*/
    int minHeight;	/*	minimum pixels height from trackDb	*/
    double yLineMark;	/*	user requested line at y = */
    char *colorTrack;   /*	Track to use for coloring wiggle track. */
    int graphColumn;	/*	column to be graphing (bedGraph tracks)	*/
    boolean bedGraph;	/*	is this a bedGraph track ?	*/
    boolean isMultiWig;	/*      If true it's a multi-wig. */
    enum wiggleAggregateFunctionEnum aggregateFunction;	/*  NONE/TRANSPARENT/STACKED	*/
    boolean doNegative; /*      should we negate the values */
    };

struct wigCartOptions *wigCartOptionsNew(struct cart *cart, struct trackDb *tdb, int wordCount, char *words[]);
/* Create a wigCartOptions from cart contents and tdb. */

struct preDrawContainer
/* A preDraw array with a little bit of extra info */
    {
    struct preDrawContainer *nextPlaceholder;   /* Refactoring code so not in list really. */
    struct preDrawElement *preDraw;     /* Array of values. */
    int preDrawSize;		/* Size of preDraw */
    int preDrawZero;		/* Offset from start of predraw array to data requested.  We
                                 * get more because of smoothing */
    int width;			/* Passed in width, number of pixels to display without smooth */
    double graphUpperLimit, graphLowerLimit; /* limits to the smoothed value */
    boolean skipAutoscale;      /* multiWig does its own autoscale */
    };

struct preDrawElement
    {
    double	max;	/*	maximum value seen for this point	*/
    double	min;	/*	minimum value seen for this point	*/
    unsigned long long	count;	/* number of datum at this point */
    double	sumData;	/*	sum of all values at this point	*/
    double  sumSquares;	/* sum of (values squared) at this point */
    double  plotValue;	/*	raw data to plot	*/
    double  smooth;	/*	smooth data values	*/
    };

struct bedGraphItem
/* A bedGraph track item. */
    {
    struct bedGraphItem *next;
    int start, end;	/* Start/end in chrom coordinates. */
    char *name;		/* Common name */
    float dataValue;	/* data value from bed table graphColumn	*/
    double graphUpperLimit;	/* filled in by DrawItems	*/
    double graphLowerLimit;	/* filled in by DrawItems	*/
    };

typedef void (*WigVerticalLineVirtual)(void *image, int x, int y, int height, Color color);
/* Draw a vertical line somewhere, maybe a regular image, maybe a transparent one. */

struct pixelCountBin
/* A structure used to keep track of how many pixels are drawn in a set of bins. */
    {
    unsigned binCount;
    unsigned binSize;
    unsigned long *bins;
    };

struct wigGraphOutput
/* Info on where to draw a wig - may be pretty indirect in transparent case. */
   {
   WigVerticalLineVirtual vLine;
   void *image;	    /* Some type in reality that goes with vLine. */
   int xOff, yOff;  /* Where to offset output within image. */
   double *yOffsets; /* if not NULL, points to yOffsets for stacked bars */
   int numTrack;   /* the index of this track */
   struct pixelCountBin *pixelBins;
   };

struct wigGraphOutput *wigGraphOutputStack(int xOff, int yOff, int width, int numTracks,  struct hvGfx *image);
/* Get appropriate wigGraphOutput for non-transparent stacked rendering */

struct wigGraphOutput *wigGraphOutputSolid(int xOff, int yOff, struct hvGfx *image);
/* Get appropriate wigGraphOutput for non-transparent rendering */

/*	source to these routines is in wigTrack.c	*/

struct preDrawContainer *initPreDrawContainer(int width);
/*	initialize a preDraw array of size width	*/

void preDrawWindowFunction(struct preDrawElement *preDraw, int preDrawSize,
	enum wiggleWindowingEnum windowingFunction,
	enum wiggleTransformFuncEnum transformFunc,
	boolean doNegative);
/*	apply windowing function to the values in preDraw array	*/

void preDrawSmoothing(struct preDrawElement *preDraw, int preDrawSize,
    enum wiggleSmoothingEnum smoothingWindow);
/*	apply smoothing function to preDraw array	*/

double preDrawLimits(struct preDrawElement *preDraw, int preDrawZero,
    int width, double *overallUpperLimit, double *overallLowerLimit);
/*	scan preDraw array and determine graph limits */

double preDrawAutoScale(struct preDrawElement *preDraw, int preDrawZero,
    int width, enum wiggleScaleOptEnum autoScale,
    enum wiggleWindowingEnum windowingFunction,
    double *graphUpperLimit, double *graphLowerLimit,
    double *epsilon, int lineHeight,
    double maxY, double minY, enum wiggleAlwaysZeroEnum alwaysZero);
/*	if autoScaling, scan preDraw array and determine limits */

void drawZeroLine(enum trackVisibility vis,
    enum wiggleGridOptEnum horizontalGrid,
    double graphUpperLimit, double graphLowerLimit,
    struct hvGfx *hvg, int xOff, int yOff, int width, int lineHeight);
/*	draw a line at y=0 on the graph	*/

void drawArbitraryYLine(enum trackVisibility vis,
    enum wiggleGridOptEnum horizontalGrid,
    double graphUpperLimit, double graphLowerLimit,
    struct hvGfx *hvg, int xOff, int yOff, int width, int lineHeight,
    double yLineMark, double graphRange, enum wiggleYLineMarkEnum yLineOnOff);
/*	draw a line at y=yLineMark on the graph	*/

void wigMapSelf(struct track *tg, struct hvGfx *hvg, int seqStart, int seqEnd,
    int xOff, int yOff, int width);
/*	if self mapping, create the mapping box	*/

void wigLeftLabels(struct track *tg, int seqStart, int seqEnd,
	struct hvGfx *hvg, int xOff, int yOff, int width, int height,
	boolean withCenterLabels, MgFont *font, Color color,
	enum trackVisibility vis);
/*	drawing left labels	*/

char *wigNameCallback(struct track *tg, void *item);
/* Return name of wig level track. */

void wigFindItemLimits(void *items,
    double *graphUpperLimit, double *graphLowerLimit);
/*	find upper and lower limits of graphed items (wigItem)	*/

void wigMultiRegionGraphLimits(struct track *tg);
/* Set common graphLimits across all windows */

void wigPreDrawPredraw(struct track *tg, int seqStart, int seqEnd,
                    struct hvGfx *hvg, int xOff, int yOff, int width,
                    MgFont *font, Color color, enum trackVisibility vis,
                    struct preDrawContainer *preContainer, int preDrawZero,
                    int preDrawSize, double *retGraphUpperLimit, double *retGraphLowerLimit);
/* Figure out graph limits after running windowingFunction and smoothing if needed. */

void wigDrawPredraw(struct track *tg, int seqStart, int seqEnd,
	struct hvGfx *hvg, int xOff, int yOff, int width,
	MgFont *font, Color color, enum trackVisibility vis, struct preDrawContainer *preDrawContainer,
	int preDrawZero, int preDrawSize, double graphUpperLimit, double graphLowerLimit);
/* Draw once we've figured out predraw. */

void wigLeftAxisLabels(struct track *tg, int seqStart, int seqEnd,
	struct hvGfx *hvg, int xOff, int yOff, int width, int height,
	boolean withCenterLabels, MgFont *font, Color color,
	enum trackVisibility vis, char *shortLabel, double graphUpperLimit, double graphLowerLimit,
	boolean showNumbers);
/* Draw labels on left for a wiggle-type track. */

double wiggleLogish(double x);
/* Return log-like transform without singularity at 0. */

/******************  in source file bedGraph.c ************************/
void wigBedGraphFindItemLimits(void *items,
    double *graphUpperLimit, double *graphLowerLimit);
/*	find upper and lower limits of graphed items (bedGraphItem)	*/

int normalizeCount(struct preDrawElement *el, double countFactor,
    double minVal, double maxVal, double sumData, double sumSquares);
/* Normalize statistics to be based on an integer number of valid bases.
 * Integer value is the smallest integer not less than countFactor. */
#endif /* WIGCOMMON_H */
