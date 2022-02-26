/* snp125Exceptions.h was originally generated by the autoSql program, which also 
 * generated snp125Exceptions.c and snp125Exceptions.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2006 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef SNP125EXCEPTIONS_H
#define SNP125EXCEPTIONS_H

#define SNP125EXCEPTIONS_NUM_COLS 5

struct snp125Exceptions
/* Annotations for snp125 data */
    {
    struct snp125Exceptions *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Reference SNP identifier or Affy SNP name */
    char *exception;	/* Exception found for this SNP */
    };

void snp125ExceptionsStaticLoad(char **row, struct snp125Exceptions *ret);
/* Load a row from snp125Exceptions table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct snp125Exceptions *snp125ExceptionsLoad(char **row);
/* Load a snp125Exceptions from row fetched with select * from snp125Exceptions
 * from database.  Dispose of this with snp125ExceptionsFree(). */

struct snp125Exceptions *snp125ExceptionsLoadAll(char *fileName);
/* Load all snp125Exceptions from whitespace-separated file.
 * Dispose of this with snp125ExceptionsFreeList(). */

struct snp125Exceptions *snp125ExceptionsLoadAllByChar(char *fileName, char chopper);
/* Load all snp125Exceptions from chopper separated file.
 * Dispose of this with snp125ExceptionsFreeList(). */

#define snp125ExceptionsLoadAllByTab(a) snp125ExceptionsLoadAllByChar(a, '\t');
/* Load all snp125Exceptions from tab separated file.
 * Dispose of this with snp125ExceptionsFreeList(). */

struct snp125Exceptions *snp125ExceptionsCommaIn(char **pS, struct snp125Exceptions *ret);
/* Create a snp125Exceptions out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snp125Exceptions */

void snp125ExceptionsFree(struct snp125Exceptions **pEl);
/* Free a single dynamically allocated snp125Exceptions such as created
 * with snp125ExceptionsLoad(). */

void snp125ExceptionsFreeList(struct snp125Exceptions **pList);
/* Free a list of dynamically allocated snp125Exceptions's */

void snp125ExceptionsOutput(struct snp125Exceptions *el, FILE *f, char sep, char lastSep);
/* Print out snp125Exceptions.  Separate fields with sep. Follow last field with lastSep. */

#define snp125ExceptionsTabOut(el,f) snp125ExceptionsOutput(el,f,'\t','\n');
/* Print out snp125Exceptions as a line in a tab-separated file. */

#define snp125ExceptionsCommaOut(el,f) snp125ExceptionsOutput(el,f,',',',');
/* Print out snp125Exceptions as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SNP125EXCEPTIONS_H */

void snp125ExceptionsTableCreate(struct sqlConnection *conn);
/* create a snp125Exceptions table */

