/* sample.h was originally generated by the autoSql program, which also 
 * generated sample.c and sample.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2003 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef SAMPLE_H
#define SAMPLE_H

#ifndef JKSQL_H 
#include "jksql.h"
#endif 


struct sample
/* Any track that has samples to display as y-values (first 6 fields are bed6) */
    {
    struct sample *next;  /* Next in singly linked list. */
    char *chrom;	/* Human chromosome or FPC contig */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item */
    unsigned score;	/* Score from 0-1000 */
    char strand[3];	/* # + or - */
    unsigned sampleCount;	/* number of samples total */
    unsigned *samplePosition;	/* bases relative to chromStart (x-values) */
    int *sampleHeight;	/* the height each pixel is drawn to [0,1000] */
    };

struct sample *sampleLoad(char **row);
/* Load a sample from row fetched with select * from sample
 * from database.  Dispose of this with sampleFree(). */

struct sample *sampleLoadAll(char *fileName);
/* Load all sample from a tab-separated file.
 * Dispose of this with sampleFreeList(). */

struct sample *sampleLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all sample from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with sampleFreeList(). */

struct sample *sampleCommaIn(char **pS, struct sample *ret);
/* Create a sample out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new sample */

void sampleFree(struct sample **pEl);
/* Free a single dynamically allocated sample such as created
 * with sampleLoad(). */

void sampleFreeList(struct sample **pList);
/* Free a list of dynamically allocated sample's */

void sampleOutput(struct sample *el, FILE *f, char sep, char lastSep);
/* Print out sample.  Separate fields with sep. Follow last field with lastSep. */

#define sampleTabOut(el,f) sampleOutput(el,f,'\t','\n');
/* Print out sample as a line in a tab-separated file. */

#define sampleCommaOut(el,f) sampleOutput(el,f,',',',');
/* Print out sample as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SAMPLE_H */

