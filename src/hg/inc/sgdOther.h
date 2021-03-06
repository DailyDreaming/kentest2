/* sgdOther.h was originally generated by the autoSql program, which also 
 * generated sgdOther.c and sgdOther.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2003 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef SGDOTHER_H
#define SGDOTHER_H

#define SGDOTHER_NUM_COLS 7

struct sgdOther
/* Features other than coding genes from yeast genome database */
    {
    struct sgdOther *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome in chrNN format */
    int chromStart;	/* Start (zero based) */
    int chromEnd;	/* End (non-inclusive) */
    char *name;	/* Feature name */
    int score;	/* Always 0 */
    char strand[2];	/* Strand: +, - or . */
    char *type;	/* Feature type */
    };

void sgdOtherStaticLoad(char **row, struct sgdOther *ret);
/* Load a row from sgdOther table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct sgdOther *sgdOtherLoad(char **row);
/* Load a sgdOther from row fetched with select * from sgdOther
 * from database.  Dispose of this with sgdOtherFree(). */

struct sgdOther *sgdOtherLoadAll(char *fileName);
/* Load all sgdOther from whitespace-separated file.
 * Dispose of this with sgdOtherFreeList(). */

struct sgdOther *sgdOtherLoadAllByChar(char *fileName, char chopper);
/* Load all sgdOther from chopper separated file.
 * Dispose of this with sgdOtherFreeList(). */

#define sgdOtherLoadAllByTab(a) sgdOtherLoadAllByChar(a, '\t');
/* Load all sgdOther from tab separated file.
 * Dispose of this with sgdOtherFreeList(). */

struct sgdOther *sgdOtherCommaIn(char **pS, struct sgdOther *ret);
/* Create a sgdOther out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new sgdOther */

void sgdOtherFree(struct sgdOther **pEl);
/* Free a single dynamically allocated sgdOther such as created
 * with sgdOtherLoad(). */

void sgdOtherFreeList(struct sgdOther **pList);
/* Free a list of dynamically allocated sgdOther's */

void sgdOtherOutput(struct sgdOther *el, FILE *f, char sep, char lastSep);
/* Print out sgdOther.  Separate fields with sep. Follow last field with lastSep. */

#define sgdOtherTabOut(el,f) sgdOtherOutput(el,f,'\t','\n');
/* Print out sgdOther as a line in a tab-separated file. */

#define sgdOtherCommaOut(el,f) sgdOtherOutput(el,f,',',',');
/* Print out sgdOther as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SGDOTHER_H */

