/* encodeRegionInfo.h was originally generated by the autoSql program, which also 
 * generated encodeRegionInfo.c and encodeRegionInfo.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2008 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef ENCODEREGIONINFO_H
#define ENCODEREGIONINFO_H

#define ENCODEREGIONINFO_NUM_COLS 2

struct encodeRegionInfo
/* Descriptive, assembly-independent information about ENCODE regions */
    {
    struct encodeRegionInfo *next;  /* Next in singly linked list. */
    char *name;	/* Name of region */
    char *descr;	/* Description (gene region, random pick, etc.) */
    };

void encodeRegionInfoStaticLoad(char **row, struct encodeRegionInfo *ret);
/* Load a row from encodeRegionInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct encodeRegionInfo *encodeRegionInfoLoad(char **row);
/* Load a encodeRegionInfo from row fetched with select * from encodeRegionInfo
 * from database.  Dispose of this with encodeRegionInfoFree(). */

struct encodeRegionInfo *encodeRegionInfoLoadAll(char *fileName);
/* Load all encodeRegionInfo from whitespace-separated file.
 * Dispose of this with encodeRegionInfoFreeList(). */

struct encodeRegionInfo *encodeRegionInfoLoadAllByChar(char *fileName, char chopper);
/* Load all encodeRegionInfo from chopper separated file.
 * Dispose of this with encodeRegionInfoFreeList(). */

#define encodeRegionInfoLoadAllByTab(a) encodeRegionInfoLoadAllByChar(a, '\t');
/* Load all encodeRegionInfo from tab separated file.
 * Dispose of this with encodeRegionInfoFreeList(). */

struct encodeRegionInfo *encodeRegionInfoCommaIn(char **pS, struct encodeRegionInfo *ret);
/* Create a encodeRegionInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodeRegionInfo */

void encodeRegionInfoFree(struct encodeRegionInfo **pEl);
/* Free a single dynamically allocated encodeRegionInfo such as created
 * with encodeRegionInfoLoad(). */

void encodeRegionInfoFreeList(struct encodeRegionInfo **pList);
/* Free a list of dynamically allocated encodeRegionInfo's */

void encodeRegionInfoOutput(struct encodeRegionInfo *el, FILE *f, char sep, char lastSep);
/* Print out encodeRegionInfo.  Separate fields with sep. Follow last field with lastSep. */

#define encodeRegionInfoTabOut(el,f) encodeRegionInfoOutput(el,f,'\t','\n');
/* Print out encodeRegionInfo as a line in a tab-separated file. */

#define encodeRegionInfoCommaOut(el,f) encodeRegionInfoOutput(el,f,',',',');
/* Print out encodeRegionInfo as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

char *getEncodeRegionDescr(char *name);
/* Get descriptive text for a region */

#endif /* ENCODEREGIONINFO_H */


