/* Stuff to handle chromGraph tracks, especially custom ones. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "trackDb.h"
#include "chromGraph.h"
#include "customTrack.h"

#include "hgGenome.h"


boolean isChromGraph(struct trackDb *track)
/* Return TRUE if it's a chromGraph track */
{
if (track && track->type)
    return startsWithWord("chromGraph", track->type);
else
    return FALSE;
}


