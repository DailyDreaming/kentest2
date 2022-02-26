/* snpExtFile.h was originally generated by the autoSql program, which also 
 * generated snpExtFile.c and snpExtFile.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2006 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef SNPEXTFILE_H
#define SNPEXTFILE_H

#define SNPEXTFILE_NUM_COLS 3

struct snpExtFile
/* External (i.e. not in database) rs_fasta file info */
    {
    struct snpExtFile *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    char *path;	/* Full path of file */
    int size;	/* byte size of file */
    };

void snpExtFileStaticLoad(char **row, struct snpExtFile *ret);
/* Load a row from snpExtFile table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct snpExtFile *snpExtFileLoad(char **row);
/* Load a snpExtFile from row fetched with select * from snpExtFile
 * from database.  Dispose of this with snpExtFileFree(). */

struct snpExtFile *snpExtFileLoadAll(char *fileName);
/* Load all snpExtFile from whitespace-separated file.
 * Dispose of this with snpExtFileFreeList(). */

struct snpExtFile *snpExtFileLoadAllByChar(char *fileName, char chopper);
/* Load all snpExtFile from chopper separated file.
 * Dispose of this with snpExtFileFreeList(). */

#define snpExtFileLoadAllByTab(a) snpExtFileLoadAllByChar(a, '\t');
/* Load all snpExtFile from tab separated file.
 * Dispose of this with snpExtFileFreeList(). */

struct snpExtFile *snpExtFileCommaIn(char **pS, struct snpExtFile *ret);
/* Create a snpExtFile out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snpExtFile */

void snpExtFileFree(struct snpExtFile **pEl);
/* Free a single dynamically allocated snpExtFile such as created
 * with snpExtFileLoad(). */

void snpExtFileFreeList(struct snpExtFile **pList);
/* Free a list of dynamically allocated snpExtFile's */

void snpExtFileOutput(struct snpExtFile *el, FILE *f, char sep, char lastSep);
/* Print out snpExtFile.  Separate fields with sep. Follow last field with lastSep. */

#define snpExtFileTabOut(el,f) snpExtFileOutput(el,f,'\t','\n');
/* Print out snpExtFile as a line in a tab-separated file. */

#define snpExtFileCommaOut(el,f) snpExtFileOutput(el,f,',',',');
/* Print out snpExtFile as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SNPEXTFILE_H */

