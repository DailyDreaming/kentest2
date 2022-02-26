/* bigWarn -- shared handlers for displaying big/udc warn/error messages */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hgTracks.h"
#include "container.h"
#include "bigWarn.h"
#include "hgConfig.h"

static int bigWarnNumLines(char *errMsg)
/* Count number of lines in err msg */
{
int n = countChars(errMsg, '\n');
int sl = strlen(errMsg);
if ((sl > 0) && (errMsg[sl-1]!='\n'))
    ++n;
if (n == 0)
    n = 1;
return n;
}


char *bigWarnReformat(char *errMsg)
/* Return a copy of the re-formatted error message,
 * such as breaking longer lines */
{
if (errMsg == NULL)
    return cloneString("");
/* convert ". " to ".\n" to break long lines. */
char *result = cloneString(errMsg);
char *nl = result;
while ((nl = strchr(nl,'.')))
    {
    ++nl;
    if (nl[0] == ' ')
	{
	nl[0] = '\n';
	++nl;
	}
    }
return result;
}

void bigDrawWarning(struct track *tg, int seqStart, int seqEnd, struct hvGfx *hvg,
                 int xOff, int yOff, int width, MgFont *font, Color color,
                 enum trackVisibility vis)
/* Draw the network error message, for the first window only. 
 * Use the entire screen space across all windows. */
{
if (currentWindow != windows)
    return;

// put the error message into the log
if (sameString(cfgOptionDefault("bigWarn", "on"), "on"))
    fprintf(stderr, "bigWarn: %s\n", tg->networkErrMsg);

int clipXBak, clipYBak, clipWidthBak, clipHeightBak;
hvGfxGetClip(hvg, &clipXBak, &clipYBak, &clipWidthBak, &clipHeightBak);
hvGfxUnclip(hvg);
hvGfxSetClip(hvg, fullInsideX, yOff, fullInsideWidth, tg->height);
xOff = fullInsideX;
width = fullInsideWidth;

char message[1024];
Color yellow = hvGfxFindRgb(hvg, &undefinedYellowColor);
char *errMsg = bigWarnReformat(tg->networkErrMsg);
char *nl = errMsg;
int sl = strlen(errMsg);
// in some cases, cannot use tg-> values, so recalc local equivalents. 
int heightPer = tl.fontHeight;        /* Height per item line minus border. */
int lineHeight = heightPer+4;         /* Height per item line including border. */ 
if (lineHeight > tg->height)
    lineHeight = tg->height;
int n = bigWarnNumLines(errMsg);      /* Lines of warning text */
int m = tg->height / lineHeight;      /* Lines of text space available */
if (m < 1) 
    m = 1;
// make yellow background to draw user's attention to the err msg

if (!sameOk(parentContainerType(tg), "multiWig")) // multiWig knows full parent height
    hvGfxBox(hvg, xOff, yOff, width, tg->height, yellow);
// leading blank lines if any
int bl = (m-n)/2;   
int i;
for(i=0;i<bl;++i)
    yOff += lineHeight;
int l = 0;
while (TRUE)
    {
    char *msg = nl;
    nl = strchr(nl,'\n');
    if (nl)
	nl[0] = 0;
    if (nl || ((sl > 0) && (errMsg[sl-1]!='\n')))
	{  
	safef(message, sizeof(message), "%s", msg);
	hvGfxTextCentered(hvg, xOff, yOff, width, lineHeight, MG_BLACK, font, message);
	yOff += lineHeight;
        ++l;
        if (l > n)
	    break;
        if (l > m)
	    break;
	}
    if (!nl)
	break;
    ++nl;
    }
freeMem(errMsg);
hvGfxUnclip(hvg);
hvGfxSetClip(hvg, clipXBak, clipYBak, clipWidthBak, clipHeightBak);
}

int bigWarnTotalHeight(struct track *tg, enum trackVisibility vis)
/* Return total height. Called before and after drawItems.
 * Must set the following variables: height, lineHeight, heightPer. */
{
if (tg->height == 0)  // since this gets called more that once, but we should calculate the same thing each time, don't re-do it.
    {    
    tg->heightPer = tl.fontHeight;        /* Height per item line minus border. */
    tg->lineHeight = tg->heightPer+4;     /* Height per item line including border. */ 

    /* count lines in reformated warning msg */
    char *errMsg = bigWarnReformat(tg->networkErrMsg);
    int n = bigWarnNumLines(errMsg);
    freeMem(errMsg);

    tg->height = tg->lineHeight * n; /* Total height - must be set. */

    }
return tg->height;
}
