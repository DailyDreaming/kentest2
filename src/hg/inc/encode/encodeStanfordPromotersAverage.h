/* encodeStanfordPromotersAverage.h was originally generated by the autoSql program, which also 
 * generated encodeStanfordPromotersAverage.c and encodeStanfordPromotersAverage.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2008 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef ENCODESTANFORDPROMOTERSAVERAGE_H
#define ENCODESTANFORDPROMOTERSAVERAGE_H

#define ENCODESTANFORDPROMOTERSAVERAGE_NUM_COLS 12

struct encodeStanfordPromotersAverage
/* Stanford Promoter Activity in ENCODE Regions, average over all cell types (bed 9+) */
    {
    struct encodeStanfordPromotersAverage *next;  /* Next in singly linked list. */
    char *chrom;	/* chromosome */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* accession of mRNA used to predict the promoter */
    unsigned score;	/* Score from 0-1000 */
    char strand[2];	/* + or - */
    unsigned thickStart;	/* Placeholder for BED9 format -- same as chromStart */
    unsigned thickEnd;	/* Placeholder for BED9 format -- same as chromEnd */
    unsigned reserved;	/* Used as itemRgb */
    char *geneModel;	/* Gene model ID; same ID may have multiple predicted promoters */
    char *description;	/* Gene description */
    float normLog2Ratio;	/* Normalized and log2 transformed Luciferase Renilla Ratio */
    };

void encodeStanfordPromotersAverageStaticLoad(char **row, struct encodeStanfordPromotersAverage *ret);
/* Load a row from encodeStanfordPromotersAverage table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct encodeStanfordPromotersAverage *encodeStanfordPromotersAverageLoad(char **row);
/* Load a encodeStanfordPromotersAverage from row fetched with select * from encodeStanfordPromotersAverage
 * from database.  Dispose of this with encodeStanfordPromotersAverageFree(). */

struct encodeStanfordPromotersAverage *encodeStanfordPromotersAverageLoadAll(char *fileName);
/* Load all encodeStanfordPromotersAverage from whitespace-separated file.
 * Dispose of this with encodeStanfordPromotersAverageFreeList(). */

struct encodeStanfordPromotersAverage *encodeStanfordPromotersAverageLoadAllByChar(char *fileName, char chopper);
/* Load all encodeStanfordPromotersAverage from chopper separated file.
 * Dispose of this with encodeStanfordPromotersAverageFreeList(). */

#define encodeStanfordPromotersAverageLoadAllByTab(a) encodeStanfordPromotersAverageLoadAllByChar(a, '\t');
/* Load all encodeStanfordPromotersAverage from tab separated file.
 * Dispose of this with encodeStanfordPromotersAverageFreeList(). */

struct encodeStanfordPromotersAverage *encodeStanfordPromotersAverageCommaIn(char **pS, struct encodeStanfordPromotersAverage *ret);
/* Create a encodeStanfordPromotersAverage out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodeStanfordPromotersAverage */

void encodeStanfordPromotersAverageFree(struct encodeStanfordPromotersAverage **pEl);
/* Free a single dynamically allocated encodeStanfordPromotersAverage such as created
 * with encodeStanfordPromotersAverageLoad(). */

void encodeStanfordPromotersAverageFreeList(struct encodeStanfordPromotersAverage **pList);
/* Free a list of dynamically allocated encodeStanfordPromotersAverage's */

void encodeStanfordPromotersAverageOutput(struct encodeStanfordPromotersAverage *el, FILE *f, char sep, char lastSep);
/* Print out encodeStanfordPromotersAverage.  Separate fields with sep. Follow last field with lastSep. */

#define encodeStanfordPromotersAverageTabOut(el,f) encodeStanfordPromotersAverageOutput(el,f,'\t','\n');
/* Print out encodeStanfordPromotersAverage as a line in a tab-separated file. */

#define encodeStanfordPromotersAverageCommaOut(el,f) encodeStanfordPromotersAverageOutput(el,f,',',',');
/* Print out encodeStanfordPromotersAverage as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* ENCODESTANFORDPROMOTERSAVERAGE_H */

