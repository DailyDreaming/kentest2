/* cnpRedon.h was originally generated by the autoSql program, which also 
 * generated cnpRedon.c and cnpRedon.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2006 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef CNPHURLES_H
#define CNPHURLES_H

#define CNPHURLES_NUM_COLS 6

struct cnpRedon
/* CNP data from Redon lab */
    {
    struct cnpRedon *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Reference SNP identifier or Affy SNP name */
    float score;	/* Score */
    char strand[2];	/* Strand */
    };

void cnpRedonStaticLoad(char **row, struct cnpRedon *ret);
/* Load a row from cnpRedon table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct cnpRedon *cnpRedonLoad(char **row);
/* Load a cnpRedon from row fetched with select * from cnpRedon
 * from database.  Dispose of this with cnpRedonFree(). */

struct cnpRedon *cnpRedonLoadAll(char *fileName);
/* Load all cnpRedon from whitespace-separated file.
 * Dispose of this with cnpRedonFreeList(). */

struct cnpRedon *cnpRedonLoadAllByChar(char *fileName, char chopper);
/* Load all cnpRedon from chopper separated file.
 * Dispose of this with cnpRedonFreeList(). */

#define cnpRedonLoadAllByTab(a) cnpRedonLoadAllByChar(a, '\t');
/* Load all cnpRedon from tab separated file.
 * Dispose of this with cnpRedonFreeList(). */

struct cnpRedon *cnpRedonCommaIn(char **pS, struct cnpRedon *ret);
/* Create a cnpRedon out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cnpRedon */

void cnpRedonFree(struct cnpRedon **pEl);
/* Free a single dynamically allocated cnpRedon such as created
 * with cnpRedonLoad(). */

void cnpRedonFreeList(struct cnpRedon **pList);
/* Free a list of dynamically allocated cnpRedon's */

void cnpRedonOutput(struct cnpRedon *el, FILE *f, char sep, char lastSep);
/* Print out cnpRedon.  Separate fields with sep. Follow last field with lastSep. */

#define cnpRedonTabOut(el,f) cnpRedonOutput(el,f,'\t','\n');
/* Print out cnpRedon as a line in a tab-separated file. */

#define cnpRedonCommaOut(el,f) cnpRedonOutput(el,f,',',',');
/* Print out cnpRedon as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* CNPHURLES_H */

