/* mafSummary.h was originally generated by the autoSql program, which also 
 * generated mafSummary.c and mafSummary.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef MAFSUMMARY_H
#define MAFSUMMARY_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define MAFSUMMARY_NUM_COLS 7

struct mafSummary
/* Positions and scores for alignment blocks */
    {
    struct mafSummary *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *src;	/* Sequence name or database of alignment */
    float score;	/* Floating point score. */
    char leftStatus[2];	/* Gap/break annotation for preceding block */
    char rightStatus[2];	/* Gap/break annotation for following block */
    };

void mafSummaryStaticLoad(char **row, struct mafSummary *ret);
/* Load a row from mafSummary table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct mafSummary *mafSummaryLoad(char **row);
/* Load a mafSummary from row fetched with select * from mafSummary
 * from database.  Dispose of this with mafSummaryFree(). */

struct mafSummary *mafSummaryLoadAll(char *fileName);
/* Load all mafSummary from whitespace-separated file.
 * Dispose of this with mafSummaryFreeList(). */

struct mafSummary *mafSummaryLoadAllByChar(char *fileName, char chopper);
/* Load all mafSummary from chopper separated file.
 * Dispose of this with mafSummaryFreeList(). */

#define mafSummaryLoadAllByTab(a) mafSummaryLoadAllByChar(a, '\t');
/* Load all mafSummary from tab separated file.
 * Dispose of this with mafSummaryFreeList(). */

struct mafSummary *mafSummaryLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all mafSummary from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with mafSummaryFreeList(). */

void mafSummarySaveToDb(struct sqlConnection *conn, struct mafSummary *el, char *tableName, int updateSize);
/* Save mafSummary as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */

struct mafSummary *mafSummaryCommaIn(char **pS, struct mafSummary *ret);
/* Create a mafSummary out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new mafSummary */

void mafSummaryFree(struct mafSummary **pEl);
/* Free a single dynamically allocated mafSummary such as created
 * with mafSummaryLoad(). */

void mafSummaryFreeList(struct mafSummary **pList);
/* Free a list of dynamically allocated mafSummary's */

void mafSummaryOutput(struct mafSummary *el, FILE *f, char sep, char lastSep);
/* Print out mafSummary.  Separate fields with sep. Follow last field with lastSep. */

#define mafSummaryTabOut(el,f) mafSummaryOutput(el,f,'\t','\n');
/* Print out mafSummary as a line in a tab-separated file. */

#define mafSummaryCommaOut(el,f) mafSummaryOutput(el,f,',',',');
/* Print out mafSummary as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

void mafSummaryTableCreate(struct sqlConnection *conn, char *tableName, int indexSize);
/* Create a summary table with the given name. */

struct mafSummary *mafSummaryMiniLoad(char **row);
/* Load a mafSummary from row fetched with select * from mafSummary
 * from database, except dummy in the {left,right}Status fields.
 * For use on earlier version of mafSummary tables.
 * Dispose of this with mafSummaryFree(). */

#endif /* MAFSUMMARY_H */
