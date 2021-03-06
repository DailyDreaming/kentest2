/* alleleFreqs.h was originally generated by the autoSql program, which also 
 * generated alleleFreqs.c and alleleFreqs.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef ALLELEFREQS_H
#define ALLELEFREQS_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define ALLELEFREQS_NUM_COLS 16

struct alleleFreqs
/* Allele Frequencies from HapMap */
    {
    struct alleleFreqs *next;  /* Next in singly linked list. */
    char *rsId;	/*  rs			rs2104604 */
    char *chrom;	/*  chrom			Chr1 */
    int chromStart;	/*  pos			101809619 */
    char strand;	/*  strand		+ */
    char *assembly;	/*  build			ncbi_b34 */
    char *center;	/*  center		bcm */
    char *protLSID;	/* protLSID */
    char *assayLSID;	/* assayLSID */
    char *panelLSID;	/* panelLSID */
    char majAllele;	/*  major_allele		G */
    int majCount;	/*  major_allele_count	120 */
    float majFreq;	/*  major_allele_freq	1 */
    char minAllele;	/*  minor_allele		T */
    int minCount;	/*  minor_allele_count	0 */
    float minFreq;	/*  minor_allele_freq	0 */
    int total;	/*  total			120 */
    };

void alleleFreqsStaticLoad(char **row, struct alleleFreqs *ret);
/* Load a row from alleleFreqs table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct alleleFreqs *alleleFreqsLoad(char **row);
/* Load a alleleFreqs from row fetched with select * from alleleFreqs
 * from database.  Dispose of this with alleleFreqsFree(). */

struct alleleFreqs *alleleFreqsLoadAll(char *fileName);
/* Load all alleleFreqs from whitespace-separated file.
 * Dispose of this with alleleFreqsFreeList(). */

struct alleleFreqs *alleleFreqsLoadAllByChar(char *fileName, char chopper);
/* Load all alleleFreqs from chopper separated file.
 * Dispose of this with alleleFreqsFreeList(). */

#define alleleFreqsLoadAllByTab(a) alleleFreqsLoadAllByChar(a, '\t');
/* Load all alleleFreqs from tab separated file.
 * Dispose of this with alleleFreqsFreeList(). */

struct alleleFreqs *alleleFreqsLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all alleleFreqs from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with alleleFreqsFreeList(). */

void alleleFreqsSaveToDb(struct sqlConnection *conn, struct alleleFreqs *el, char *tableName, int updateSize);
/* Save alleleFreqs as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */

struct alleleFreqs *alleleFreqsCommaIn(char **pS, struct alleleFreqs *ret);
/* Create a alleleFreqs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new alleleFreqs */

void alleleFreqsFree(struct alleleFreqs **pEl);
/* Free a single dynamically allocated alleleFreqs such as created
 * with alleleFreqsLoad(). */

void alleleFreqsFreeList(struct alleleFreqs **pList);
/* Free a list of dynamically allocated alleleFreqs's */

void alleleFreqsOutput(struct alleleFreqs *el, FILE *f, char sep, char lastSep);
/* Print out alleleFreqs.  Separate fields with sep. Follow last field with lastSep. */

#define alleleFreqsTabOut(el,f) alleleFreqsOutput(el,f,'\t','\n');
/* Print out alleleFreqs as a line in a tab-separated file. */

#define alleleFreqsCommaOut(el,f) alleleFreqsOutput(el,f,',',',');
/* Print out alleleFreqs as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* ALLELEFREQS_H */

