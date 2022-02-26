/**** Lowe lab declarations ***/

/* Copyright (C) 2008 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

Color gbGeneColor(struct track *tg, void *item, struct hvGfx *hvg);
void archaeaGeneMethods(struct track *tg);
void gbGeneMethods(struct track *tg);
Color tigeGeneColor(struct track *tg, void *item, struct hvGfx *hvg);
void tigrGeneMethods(struct track *tg);
char *llBlastPName(struct track *tg, void *item);
void llBlastPMethods(struct track *tg);
void codeBlastMethods(struct track *tg);
void tigrOperonMethods(struct track *tg);
void rnaGenesMethods(struct track *tg);
void sargassoSeaMethods(struct track *tg);
void rnaHybridizationMethods(struct track *tg);

