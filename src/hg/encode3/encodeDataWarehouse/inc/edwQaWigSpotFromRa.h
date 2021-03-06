/* Parser to read in a edwQaWigSpot from a ra file where tags in ra file correspond to fields in a
 * struct. This program was generated by raToStructGen. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef EDWQAWIGSPOTFROMRA_H

struct raToStructReader *edwQaWigSpotRaReader();
/* Make a raToStructReader for edwBamFile */

struct edwQaWigSpot *edwQaWigSpotFromNextRa(struct lineFile *lf, struct raToStructReader *reader);
/* Return next stanza put into an edwQaWigSpot. */

struct edwQaWigSpot *edwQaWigSpotLoadRa(char *fileName);
/* Return list of all edwQaWigSpot in ra file. */

struct edwQaWigSpot *edwQaWigSpotOneFromRa(char *fileName);
/* Return edwQaWigSpot in file and insist there be exactly one record. */

#endif /* EDWQAWIGSPOTFROMRA_H */
