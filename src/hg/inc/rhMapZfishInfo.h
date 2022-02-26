/* rhMapZfishInfo.h was originally generated by the autoSql program, which also 
 * generated rhMapZfishInfo.c and rhMapZfishInfo.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2007 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef RHMAPZFISHINFO_H
#define RHMAPZFISHINFO_H

#define RHMAPZFISHINFO_NUM_COLS 10

struct rhMapZfishInfo
/* Zebrafish Radiation Hybrid map information */
    {
    struct rhMapZfishInfo *next;  /* Next in singly linked list. */
    char *name;	/* Name of Radiation Hybrid (RH) map marker */
    char *zfinId;	/* ZFIN ID for the marker */
    char *linkageGp;	/* Linkage group to which the marker was mapped */
    unsigned position;	/* Position number in RH map for this linkage group */
    unsigned distance;	/* Distance from the top of linkage group (cR) */
    char *markerType;	/* Type of marker */
    char *source;	/* Source of marker */
    char *mapSite;	/* Institution that mapped the marker */
    char *leftPrimer;	/* Forward primer sequence */
    char *rightPrimer;	/* Reverse primer sequence */
    };

void rhMapZfishInfoStaticLoad(char **row, struct rhMapZfishInfo *ret);
/* Load a row from rhMapZfishInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct rhMapZfishInfo *rhMapZfishInfoLoad(char **row);
/* Load a rhMapZfishInfo from row fetched with select * from rhMapZfishInfo
 * from database.  Dispose of this with rhMapZfishInfoFree(). */

struct rhMapZfishInfo *rhMapZfishInfoLoadAll(char *fileName);
/* Load all rhMapZfishInfo from whitespace-separated file.
 * Dispose of this with rhMapZfishInfoFreeList(). */

struct rhMapZfishInfo *rhMapZfishInfoLoadAllByChar(char *fileName, char chopper);
/* Load all rhMapZfishInfo from chopper separated file.
 * Dispose of this with rhMapZfishInfoFreeList(). */

#define rhMapZfishInfoLoadAllByTab(a) rhMapZfishInfoLoadAllByChar(a, '\t');
/* Load all rhMapZfishInfo from tab separated file.
 * Dispose of this with rhMapZfishInfoFreeList(). */

struct rhMapZfishInfo *rhMapZfishInfoCommaIn(char **pS, struct rhMapZfishInfo *ret);
/* Create a rhMapZfishInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rhMapZfishInfo */

void rhMapZfishInfoFree(struct rhMapZfishInfo **pEl);
/* Free a single dynamically allocated rhMapZfishInfo such as created
 * with rhMapZfishInfoLoad(). */

void rhMapZfishInfoFreeList(struct rhMapZfishInfo **pList);
/* Free a list of dynamically allocated rhMapZfishInfo's */

void rhMapZfishInfoOutput(struct rhMapZfishInfo *el, FILE *f, char sep, char lastSep);
/* Print out rhMapZfishInfo.  Separate fields with sep. Follow last field with lastSep. */

#define rhMapZfishInfoTabOut(el,f) rhMapZfishInfoOutput(el,f,'\t','\n');
/* Print out rhMapZfishInfo as a line in a tab-separated file. */

#define rhMapZfishInfoCommaOut(el,f) rhMapZfishInfoOutput(el,f,',',',');
/* Print out rhMapZfishInfo as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* RHMAPZFISHINFO_H */

