/* cds.h - code for coloring of bases, codons, or alignment differences. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef CDS_H
#define CDS_H

#ifndef HVGFX_H
#include "hvGfx.h"
#endif

#ifndef PSL_H
#include "psl.h"
#endif

#ifndef GENEPRED_H
#include "genePred.h"
#endif

#ifndef HGTRACKS_H
#include "hgTracks.h"
#endif

/* Definitions of cds colors for coding coloring display */
#define CDS_ERROR   0

#define CDS_ODD     1
#define	CDS_ODD_R	0x00
#define	CDS_ODD_G	0x00
#define	CDS_ODD_B	0x9e

#define CDS_EVEN    2
#define	CDS_EVEN_R	0x00
#define	CDS_EVEN_G	0x00
#define	CDS_EVEN_B	0xdc

#define CDS_START   3
#define	CDS_START_R	0x00
#define	CDS_START_G	0xf0
#define	CDS_START_B	0x00

#define CDS_STOP    4
#define	CDS_STOP_R	0xe1
#define	CDS_STOP_G	0x00
#define	CDS_STOP_B	0x00

#define CDS_SPLICE      5
#define	CDS_SPLICE_R	0xa0
#define	CDS_SPLICE_G	0xa0
#define	CDS_SPLICE_B	0xd9

#define CDS_PARTIAL_CODON	6
#define CDS_PARTIAL_CODON_R	0x0
#define CDS_PARTIAL_CODON_G 0xc0
#define CDS_PARTIAL_CODON_B	0xc0

#define CDS_QUERY_INSERTION   7
#define CDS_QUERY_INSERTION_R 220
#define CDS_QUERY_INSERTION_G 128
#define CDS_QUERY_INSERTION_B 0

#define CDS_QUERY_INSERTION_AT_END 8
#define CDS_QUERY_INSERTION_AT_END_R 146
#define CDS_QUERY_INSERTION_AT_END_G 20
#define CDS_QUERY_INSERTION_AT_END_B 255

#define CDS_POLY_A 9
#define CDS_POLY_A_R 0
#define CDS_POLY_A_G 210
#define CDS_POLY_A_B 0

#define CDS_ALT_START    10
#define CDS_ALT_START_R  0 
#define CDS_ALT_START_G  0 
#define CDS_ALT_START_B  128

#define CDS_SYN_PROT    11   /* yellow, protein seq change "synonymous" ie I->V , R->K etc */
#define CDS_SYN_PROT_R  255 
#define CDS_SYN_PROT_G  215 
#define CDS_SYN_PROT_B  0

#define CDS_SYN_BLEND    12  /* brown, protein seq part syn and part non-syn */
#define CDS_SYN_BLEND_R  100 
#define CDS_SYN_BLEND_G  50 
#define CDS_SYN_BLEND_B  0

// Ribosomal shift colors
#define CDS_RIBO_SLIP1    13  /* purple */
#define CDS_RIBO_SLIP1_R  250 
#define CDS_RIBO_SLIP1_G  50 
#define CDS_RIBO_SLIP1_B  255

#define CDS_RIBO_SLIP2    14  /* cyan */
#define CDS_RIBO_SLIP2_R  50 
#define CDS_RIBO_SLIP2_G  250 
#define CDS_RIBO_SLIP2_B  255

#define CDS_NUM_COLORS 15

Color getCdsColor(int index);
/* return color from index of types of colors */

enum baseColorDrawOpt baseColorGetDrawOpt(struct track *tg);
/* Determine what base/codon coloring option (if any) has been selected 
 * in trackDb/cart, and gate with zoom level. */


struct simpleFeature *baseColorCodonsFromGenePred(struct linkedFeatures *lf,
        struct genePred *gp, boolean colorStopStart, boolean codonNumbering);
/* Given an lf and the genePred from which the lf was constructed, 
 * return a list of simpleFeature elements, one per codon (or partial 
 * codon if the codon falls on a gap boundary. */

struct simpleFeature *baseColorCodonsFromPsl(struct linkedFeatures *lf, 
        struct psl *psl, int sizeMul, boolean isXeno, int maxShade,
        enum baseColorDrawOpt drawOpt, struct track *tg);
/* Given an lf and the psl from which the lf was constructed, 
 * return a list of simpleFeature elements, one per codon (or partial 
 * codon if the codon falls on a gap boundary.  sizeMul, isXeno and maxShade
 * are for defaulting to one-simpleFeature-per-exon if cds is not found. */


INLINE boolean baseColorCanDraw(struct track *tg)
/* baseColor uses tg->drawItems as a proxy for type of tg->items, which must be 
 * linkedFeatures or linkedFeaturesSeries.  Return TRUE if tg->drawItems
 * implies that tg->items is linkedFeatures or linkedFeaturesSeries, and from
 * a subtype supported by the baseColor code. */
{
return (tg->drawItems == linkedFeaturesDraw || tg->drawItems == linkedFeaturesAverageDense ||
	tg->drawItems == linkedFeaturesAverageDenseOrientEst ||
	tg->drawItems == linkedFeaturesSeriesDraw ||
	tg->drawItems == bamLinkedFeaturesDraw ||
	tg->drawItems == bamLinkedFeaturesSeriesDraw);

}

void baseColorInitTrack(struct hvGfx *hvg, struct track *tg);
/* Set up base coloring state (e.g. cache genomic sequence) for tg.
 * This must be called by tg->drawItems if baseColorDrawSetup is used 
 * in tg->drawItemAt, but note that this should be called only if
 * baseColorCanDraw(tg) (above). */

enum baseColorDrawOpt baseColorDrawSetup(struct hvGfx *hvg, struct track *tg,
			struct linkedFeatures *lf,
			struct dnaSeq **retMrnaSeq, struct psl **retPsl);
/* Returns the CDS coloring option, allocates colors if necessary, and 
 * returns the sequence and psl record for the given item if applicable. 
 * Note: even if base coloring is not enabled, this will return psl and 
 * mrna seq if query insert/polyA display is enabled.
 * baseColorInitTrack must be called before this (in tg->drawItems) --
 * this is meant to be called by tg->drawItemAt (i.e. linkedFeaturesDrawAt). */

void baseColorDrawItem(struct track *tg,  struct linkedFeatures *lf,
        int grayIx, struct hvGfx *hvg, int xOff, int y,
        double scale, MgFont *font, int s, int e, int heightPer,
        boolean zoomedToCodonLevel, struct dnaSeq *mrnaSeq, struct simpleFeature *sf, struct psl *psl,
	enum baseColorDrawOpt drawOpt,
        int maxPixels, int winStart, Color originalColor);
/*draw a box that is colored by the bases inside it and its
 * orientation. Stop codons are red, start are green, otherwise they
 * alternate light/dark blue colors. */

void baseColorOverdrawDiff(struct track *tg,  struct linkedFeatures *lf,
			   struct hvGfx *hvg, int xOff,
			   int y, double scale, int heightPer,
			   struct dnaSeq *mrnaSeq, struct psl *psl,
			   int winStart, enum baseColorDrawOpt drawOpt);
/* If we're drawing different bases/codons, and zoomed out past base/codon 
 * level, draw 1-pixel wide red lines only where bases/codons differ from 
 * genomic.  This tests drawing mode and zoom level but assumes that lf itself 
 * has been drawn already and we're not in dense mode etc. */

void baseColorOverdrawQInsert(struct track *tg,  struct linkedFeatures *lf,
			      struct hvGfx *hvg, int xOff,
			      int y, double scale, int heightPer,
			      struct dnaSeq *mrnaSeq, struct psl *psl,
			      int winStart, enum baseColorDrawOpt drawOpt,
			      boolean indelShowQInsert, boolean indelShowPolyA);
/* If applicable, draw 1-pixel wide orange lines for query insertions in the
 * middle of the query, 1-pixel wide purple lines for query insertions at the 
 * end of the query, and 1-pixel wide green (instead of purple) when a query 
 * insertion at the end is a valid poly-A tail. */

void baseColorDrawCleanup(struct linkedFeatures *lf, struct dnaSeq **pMrnaSeq,
			  struct psl **pPsl);
/* Free structures allocated just for base/cds coloring. */


struct simpleFeature *baseColorCodonsFromDna(int frame, int chromStart,
					     int chromEnd, struct dnaSeq *seq,
					     bool reverse);
/* Create list of codons from a DNA sequence */

void baseColorDrawRulerCodons(struct hvGfx *hvg, struct simpleFeature *sfList,
                double scale, int xOff, int y, int height, MgFont *font, 
                int winStart, int maxPixels, bool zoomedToText);
/* Draw amino acid translation of genomic sequence based on a list
   of codons. Used for browser ruler in full mode*/

void baseColorSetCdsBounds(struct linkedFeatures *lf, struct psl *psl,
                           struct track *tg);
/* set CDS bounds in linked features for a PSL.  Used when zoomed out too far
 * for codon or base coloring, but still want to render CDS bounds */

#endif /* CDS_H */
