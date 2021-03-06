/* minGeneInfo.h was originally generated by the autoSql program, which also 
 * generated minGeneInfo.c and minGeneInfo.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2005 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef MINGENEINFO_H
#define MINGENEINFO_H

#define MINGENEINFO_NUM_COLS 8

struct minGeneInfo
/* Auxilliary info about a gene (less than the knownInfo) */
    {
    struct minGeneInfo *next;  /* Next in singly linked list. */
    char *name;	/* gene accession */
    char *gene;	/* gene name */
    char *product;	/* gene product */
    char *note;	/* gene note */
    char *protein;	/* gene protein */
    char *gi;	/* gene genbank id */
    char *ec;	/* ec number  */
    char *entrezGene;	/* entrez gene id */
    };

void minGeneInfoStaticLoad(char **row, struct minGeneInfo *ret);
/* Load a row from minGeneInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct minGeneInfo *minGeneInfoLoad(char **row);
/* Load a minGeneInfo from row fetched with select * from minGeneInfo
 * from database.  Dispose of this with minGeneInfoFree(). */

struct minGeneInfo *minGeneInfoLoadAll(char *fileName);
/* Load all minGeneInfo from whitespace-separated file.
 * Dispose of this with minGeneInfoFreeList(). */

struct minGeneInfo *minGeneInfoLoadAllByChar(char *fileName, char chopper);
/* Load all minGeneInfo from chopper separated file.
 * Dispose of this with minGeneInfoFreeList(). */

#define minGeneInfoLoadAllByTab(a) minGeneInfoLoadAllByChar(a, '\t');
/* Load all minGeneInfo from tab separated file.
 * Dispose of this with minGeneInfoFreeList(). */

struct minGeneInfo *minGeneInfoCommaIn(char **pS, struct minGeneInfo *ret);
/* Create a minGeneInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new minGeneInfo */

void minGeneInfoFree(struct minGeneInfo **pEl);
/* Free a single dynamically allocated minGeneInfo such as created
 * with minGeneInfoLoad(). */

void minGeneInfoFreeList(struct minGeneInfo **pList);
/* Free a list of dynamically allocated minGeneInfo's */

void minGeneInfoOutput(struct minGeneInfo *el, FILE *f, char sep, char lastSep);
/* Print out minGeneInfo.  Separate fields with sep. Follow last field with lastSep. */

#define minGeneInfoTabOut(el,f) minGeneInfoOutput(el,f,'\t','\n');
/* Print out minGeneInfo as a line in a tab-separated file. */

#define minGeneInfoCommaOut(el,f) minGeneInfoOutput(el,f,',',',');
/* Print out minGeneInfo as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* MINGENEINFO_H */

