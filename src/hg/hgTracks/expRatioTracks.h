/* Copyright (C) 2008 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef EXPRATIO_TRACKS_H
#define EXPRATIO_TRACKS_H

void mapBoxHcTwoItems(struct hvGfx *hvg, int start, int end, int x, int y, int width, int height, 
		      char *track, char *item1, char *item2, char *statusLine);
/* Print out image map rectangle that would invoke the htc (human track click)
 * program. */

void illuminaProbesMethods(struct track *tg);

#endif
