/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef RETROGENE_H
#define RETROGENE_H

#include "common.h"
#include "linefile.h"
#include "portable.h"
#include "memalloc.h"
#include "localmem.h"
#include "obscure.h"
#include "dystring.h"
#include "hash.h"
#include "jksql.h"
#include "chromColors.h"
#include "hdb.h"
#include "hgTracks.h"
#include "ucscRetroInfo.h"
#include "retroMrnaInfo.h"
#endif /* RETROGENE_H */

#define retroInfoTblSetting    "ucscRetroInfo"

extern char *protDbName;               /* Name of proteome database for this genome. */
struct linkedFeatures *lfFromRetroGene(struct ucscRetroInfo *pg);
/* Return a linked feature from a retroGene. */

void lfRetroGene(struct track *tg);
/* Load the items in one custom track - just move beds in
 * window... */

void lookupRetroNames(struct track *tg);
/* This converts the accession to a gene name where possible. */

void loadRetroGene(struct track *tg);
/* Load up RetroGenes. */

void retroGeneMethods(struct track *tg);
/* Make track of retroGenes from bed */


