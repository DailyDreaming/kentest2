/* wiggle - stuff to process wiggle tracks. 
 * Much of this is lifted from hgText/hgWigText.c */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "dystring.h"
#include "portable.h"
#include "obscure.h"
#include "jksql.h"
#include "cheapcgi.h"
#include "cart.h"
#include "web.h"
#include "bed.h"
#include "hdb.h"
#include "hui.h"
#include "hgColors.h"
#include "trackDb.h"
#include "customTrack.h"
#include "wiggle.h"
#include "correlate.h"

#include "hgGenome.h"



boolean isWiggle(char *db, char *table)
/* Return TRUE if db.table is a wiggle. */
{
boolean typeWiggle = FALSE;

if (db != NULL && table != NULL)
    {
    if (isCustomTrack(table))
	{
	struct customTrack *ct = lookupCt(table);
	if (ct->wiggle)
	    typeWiggle = TRUE;
	}
    else
	{
	struct hTableInfo *hti = maybeGetHti(db, table);
	typeWiggle = (hti != NULL && HTI_IS_WIGGLE);
	}
    }
return(typeWiggle);
}


struct wiggleDataStream *wigChromRawStats(char *chrom)
/* Fetch stats for wig data in chrom.  
 * Returns a wiggleDataStream, free it with wiggleDataStreamFree() */
{
char splitTableOrFileName[HDB_MAX_TABLE_STRING];
struct customTrack *ct = NULL;
boolean isCustom = FALSE;
struct wiggleDataStream *wds = NULL;
int operations = wigFetchRawStats;
char *table = curTable;

/* ct, isCustom, wds are set here */
if (isCustomTrack(table)) 
    { 
    ct = lookupCt(table); 
    isCustom = TRUE; 
    if (! ct->wiggle) 
	errAbort("called to work on a custom track '%s' that isn't wiggle data ?", table); 
 
    if (ct->dbTrack) 
	safef(splitTableOrFileName, sizeof splitTableOrFileName, "%s", ct->dbTableName); 
    else 
	safef(splitTableOrFileName, sizeof splitTableOrFileName, "%s", ct->wigFile); 
    } 

wds = wiggleDataStreamNew(); 

wds->setChromConstraint(wds, chrom);

if (isCustom)
    {
    if (ct->dbTrack)
	wds->getData(wds, CUSTOM_TRASH, splitTableOrFileName, operations);
    else
	wds->getData(wds, NULL, splitTableOrFileName, operations);
    }
else
    {
    if (hFindSplitTable(database, chrom, table, splitTableOrFileName, sizeof splitTableOrFileName, NULL))
	{
	wds->getData(wds, database, splitTableOrFileName, operations);
	}
    else
	errAbort("Track %s not found", table);
    }
return wds;
}



