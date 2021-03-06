/* encodeIndels.h was originally generated by the autoSql program, which also 
 * generated encodeIndels.c and encodeIndels.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2008 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef ENCODEINDELS_H
#define ENCODEINDELS_H

#define ENCODEINDELS_NUM_COLS 15

struct encodeIndels
/* ENCODE Deletion and Insertion Polymorphisms from NHGRI */
    {
    struct encodeIndels *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Trace sequence */
    unsigned score;	/* Quality score  */
    char strand[2];	/* Always + */
    unsigned thickStart;	/* Start position in chromosome */
    unsigned thickEnd;	/* End position in chromosome */
    unsigned reserved;	/* Reserved */
    char *traceName;	/* Name of trace */
    char *traceId;	/* Trace Id (always integer) */
    unsigned tracePos;	/* Position in trace */
    char traceStrand[2];	/* Value should be + or - */
    char *variant;	/* Variant sequence */
    char *reference;	/* Reference sequence */
    };

void encodeIndelsStaticLoad(char **row, struct encodeIndels *ret);
/* Load a row from encodeIndels table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct encodeIndels *encodeIndelsLoad(char **row);
/* Load a encodeIndels from row fetched with select * from encodeIndels
 * from database.  Dispose of this with encodeIndelsFree(). */

struct encodeIndels *encodeIndelsLoadAll(char *fileName);
/* Load all encodeIndels from whitespace-separated file.
 * Dispose of this with encodeIndelsFreeList(). */

struct encodeIndels *encodeIndelsLoadAllByChar(char *fileName, char chopper);
/* Load all encodeIndels from chopper separated file.
 * Dispose of this with encodeIndelsFreeList(). */

#define encodeIndelsLoadAllByTab(a) encodeIndelsLoadAllByChar(a, '\t');
/* Load all encodeIndels from tab separated file.
 * Dispose of this with encodeIndelsFreeList(). */

struct encodeIndels *encodeIndelsCommaIn(char **pS, struct encodeIndels *ret);
/* Create a encodeIndels out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodeIndels */

void encodeIndelsFree(struct encodeIndels **pEl);
/* Free a single dynamically allocated encodeIndels such as created
 * with encodeIndelsLoad(). */

void encodeIndelsFreeList(struct encodeIndels **pList);
/* Free a list of dynamically allocated encodeIndels's */

void encodeIndelsOutput(struct encodeIndels *el, FILE *f, char sep, char lastSep);
/* Print out encodeIndels.  Separate fields with sep. Follow last field with lastSep. */

#define encodeIndelsTabOut(el,f) encodeIndelsOutput(el,f,'\t','\n');
/* Print out encodeIndels as a line in a tab-separated file. */

#define encodeIndelsCommaOut(el,f) encodeIndelsOutput(el,f,',',',');
/* Print out encodeIndels as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* ENCODEINDELS_H */

