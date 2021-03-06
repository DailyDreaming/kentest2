/* recombRateMouse.h was originally generated by the autoSql program, which also 
 * generated recombRateMouse.c and recombRateMouse.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2003 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef RECOMBRATEMOUSE_H
#define RECOMBRATEMOUSE_H

#define RECOMBRATEMOUSE_NUM_COLS 6

struct recombRateMouse
/* Describes the recombination rate in 5Mb intervals based on genetic maps */
    {
    struct recombRateMouse *next;  /* Next in singly linked list. */
    char *chrom;	/* chromosome number */
    unsigned chromStart;	/* Start position in genoSeq */
    unsigned chromEnd;	/* End position in genoSeq */
    char *name;	/* Constant string recombRate */
    float wiAvg;	/* Calculated WI genetic map recombination rate */
    float mgdAvg;	/* Calculated MGD genetic map recombination rate */
    };

void recombRateMouseStaticLoad(char **row, struct recombRateMouse *ret);
/* Load a row from recombRateMouse table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct recombRateMouse *recombRateMouseLoad(char **row);
/* Load a recombRateMouse from row fetched with select * from recombRateMouse
 * from database.  Dispose of this with recombRateMouseFree(). */

struct recombRateMouse *recombRateMouseLoadAll(char *fileName);
/* Load all recombRateMouse from whitespace-separated file.
 * Dispose of this with recombRateMouseFreeList(). */

struct recombRateMouse *recombRateMouseLoadAllByChar(char *fileName, char chopper);
/* Load all recombRateMouse from chopper separated file.
 * Dispose of this with recombRateMouseFreeList(). */

#define recombRateMouseLoadAllByTab(a) recombRateMouseLoadAllByChar(a, '\t');
/* Load all recombRateMouse from tab separated file.
 * Dispose of this with recombRateMouseFreeList(). */

struct recombRateMouse *recombRateMouseCommaIn(char **pS, struct recombRateMouse *ret);
/* Create a recombRateMouse out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new recombRateMouse */

void recombRateMouseFree(struct recombRateMouse **pEl);
/* Free a single dynamically allocated recombRateMouse such as created
 * with recombRateMouseLoad(). */

void recombRateMouseFreeList(struct recombRateMouse **pList);
/* Free a list of dynamically allocated recombRateMouse's */

void recombRateMouseOutput(struct recombRateMouse *el, FILE *f, char sep, char lastSep);
/* Print out recombRateMouse.  Separate fields with sep. Follow last field with lastSep. */

#define recombRateMouseTabOut(el,f) recombRateMouseOutput(el,f,'\t','\n');
/* Print out recombRateMouse as a line in a tab-separated file. */

#define recombRateMouseCommaOut(el,f) recombRateMouseOutput(el,f,',',',');
/* Print out recombRateMouse as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* RECOMBRATEMOUSE_H */

