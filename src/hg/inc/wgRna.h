/* wgRna.h was originally generated by the autoSql program, which also 
 * generated wgRna.c and wgRna.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2004 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef WGRNA_H
#define WGRNA_H

#define WGRNA_NUM_COLS 10

struct wgRna
/* CD and H/ACA Box snoRNAs and microRNAs from Weber and Griffiths-Jones */
    {
    struct wgRna *next;  /* Next in singly linked list. */
    short bin;	/* Bin number for browser speedup */
    char *chrom;	/* chromosome */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item */
    unsigned score;	/* Score from 0-1000 (bed6 compat.) */
    char strand[2];	/* + or - (bed6 compat.) */
    unsigned thickStart;	/* start of thick region */
    unsigned thickEnd;	/* start of thick region */
    char *type;	/* RNA type */
    };

void wgRnaStaticLoad(char **row, struct wgRna *ret);
/* Load a row from wgRna table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct wgRna *wgRnaLoad(char **row);
/* Load a wgRna from row fetched with select * from wgRna
 * from database.  Dispose of this with wgRnaFree(). */

struct wgRna *wgRnaLoadAll(char *fileName);
/* Load all wgRna from whitespace-separated file.
 * Dispose of this with wgRnaFreeList(). */

struct wgRna *wgRnaLoadAllByChar(char *fileName, char chopper);
/* Load all wgRna from chopper separated file.
 * Dispose of this with wgRnaFreeList(). */

#define wgRnaLoadAllByTab(a) wgRnaLoadAllByChar(a, '\t');
/* Load all wgRna from tab separated file.
 * Dispose of this with wgRnaFreeList(). */

struct wgRna *wgRnaCommaIn(char **pS, struct wgRna *ret);
/* Create a wgRna out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wgRna */

void wgRnaFree(struct wgRna **pEl);
/* Free a single dynamically allocated wgRna such as created
 * with wgRnaLoad(). */

void wgRnaFreeList(struct wgRna **pList);
/* Free a list of dynamically allocated wgRna's */

void wgRnaOutput(struct wgRna *el, FILE *f, char sep, char lastSep);
/* Print out wgRna.  Separate fields with sep. Follow last field with lastSep. */

#define wgRnaTabOut(el,f) wgRnaOutput(el,f,'\t','\n');
/* Print out wgRna as a line in a tab-separated file. */

#define wgRnaCommaOut(el,f) wgRnaOutput(el,f,',',',');
/* Print out wgRna as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* WGRNA_H */

