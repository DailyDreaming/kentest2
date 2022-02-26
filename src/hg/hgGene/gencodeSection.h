/* gencodeClick - click handling for GENCODE tracks */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#ifndef GENCODECLICK_H


bool isNewGencodeGene(struct trackDb *tdb);
/* is this a new-style gencode (>= V7) track, as indicated by
 * the presence of the wgEncodeGencodeVersion setting */

void doGencodeGene(struct trackDb *tdb, char *gencodeId);
/* Process click on a GENCODE annotation. */

#endif
