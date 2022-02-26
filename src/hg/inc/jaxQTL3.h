/* jaxQTL3.h was originally generated by the autoSql program, which also 
 * generated jaxQTL3.c and jaxQTL3.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2007 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef JAXQTL3_H
#define JAXQTL3_H

#define JAXQTL3_NUM_COLS 14

struct jaxQTL3
/* Quantitative Trait Loci from Jackson Lab / Mouse Genome Informatics */
    {
    struct jaxQTL3 *next;  /* Next in singly linked list. */
    char *chrom;	/* chromosome */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item */
    unsigned score;	/* Score from 0-1000 (bed6 compat.) */
    char strand[2];	/* + or - (bed6 compat.) */
    unsigned thickStart;	/* start of thick region */
    unsigned thickEnd;	/* start of thick region */
    char *marker;	/* MIT SSLP Marker w/highest correlation */
    char *mgiID;	/* MGI ID */
    char *description;	/* MGI description */
    float cMscore;	/* cM position of marker associated with peak LOD score */
    char *flank1;	/* flanking marker 1 */
    char *flank2;	/* flanking marker 2 */
    };

void jaxQTL3StaticLoad(char **row, struct jaxQTL3 *ret);
/* Load a row from jaxQTL3 table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct jaxQTL3 *jaxQTL3Load(char **row);
/* Load a jaxQTL3 from row fetched with select * from jaxQTL3
 * from database.  Dispose of this with jaxQTL3Free(). */

struct jaxQTL3 *jaxQTL3LoadAll(char *fileName);
/* Load all jaxQTL3 from whitespace-separated file.
 * Dispose of this with jaxQTL3FreeList(). */

struct jaxQTL3 *jaxQTL3LoadAllByChar(char *fileName, char chopper);
/* Load all jaxQTL3 from chopper separated file.
 * Dispose of this with jaxQTL3FreeList(). */

#define jaxQTL3LoadAllByTab(a) jaxQTL3LoadAllByChar(a, '\t');
/* Load all jaxQTL3 from tab separated file.
 * Dispose of this with jaxQTL3FreeList(). */

struct jaxQTL3 *jaxQTL3CommaIn(char **pS, struct jaxQTL3 *ret);
/* Create a jaxQTL3 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new jaxQTL3 */

void jaxQTL3Free(struct jaxQTL3 **pEl);
/* Free a single dynamically allocated jaxQTL3 such as created
 * with jaxQTL3Load(). */

void jaxQTL3FreeList(struct jaxQTL3 **pList);
/* Free a list of dynamically allocated jaxQTL3's */

void jaxQTL3Output(struct jaxQTL3 *el, FILE *f, char sep, char lastSep);
/* Print out jaxQTL3.  Separate fields with sep. Follow last field with lastSep. */

#define jaxQTL3TabOut(el,f) jaxQTL3Output(el,f,'\t','\n');
/* Print out jaxQTL3 as a line in a tab-separated file. */

#define jaxQTL3CommaOut(el,f) jaxQTL3Output(el,f,',',',');
/* Print out jaxQTL3 as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* JAXQTL3_H */
