/* pbStamp.h was originally generated by the autoSql program, which also 
 * generated pbStamp.c and pbStamp.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2003 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef PBSTAMP_H
#define PBSTAMP_H

#define PBSTAMP_NUM_COLS 9

struct pbStamp
/* Info needed for a Proteome Browser Stamp */
    {
    struct pbStamp *next;  /* Next in singly linked list. */
    char stampName[41];	/* Short Name of stamp */
    char stampTable[41];	/* Database table name of the stamp (distribution) data */
    char stampTitle[41];	/* Stamp Title to be displayed */
    int len;	/* number of x-y pairs */
    float xmin;	/* x minimum */
    float xmax;	/* x maximum */
    float ymin;	/* y minimum */
    float ymax;	/* y maximum */
    char *stampDesc;	/* Description of the stamp */
    };

void pbStampStaticLoad(char **row, struct pbStamp *ret);
/* Load a row from pbStamp table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct pbStamp *pbStampLoad(char **row);
/* Load a pbStamp from row fetched with select * from pbStamp
 * from database.  Dispose of this with pbStampFree(). */

struct pbStamp *pbStampLoadAll(char *fileName);
/* Load all pbStamp from whitespace-separated file.
 * Dispose of this with pbStampFreeList(). */

struct pbStamp *pbStampLoadAllByChar(char *fileName, char chopper);
/* Load all pbStamp from chopper separated file.
 * Dispose of this with pbStampFreeList(). */

#define pbStampLoadAllByTab(a) pbStampLoadAllByChar(a, '\t');
/* Load all pbStamp from tab separated file.
 * Dispose of this with pbStampFreeList(). */

struct pbStamp *pbStampCommaIn(char **pS, struct pbStamp *ret);
/* Create a pbStamp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pbStamp */

void pbStampFree(struct pbStamp **pEl);
/* Free a single dynamically allocated pbStamp such as created
 * with pbStampLoad(). */

void pbStampFreeList(struct pbStamp **pList);
/* Free a list of dynamically allocated pbStamp's */

void pbStampOutput(struct pbStamp *el, FILE *f, char sep, char lastSep);
/* Print out pbStamp.  Separate fields with sep. Follow last field with lastSep. */

#define pbStampTabOut(el,f) pbStampOutput(el,f,'\t','\n');
/* Print out pbStamp as a line in a tab-separated file. */

#define pbStampCommaOut(el,f) pbStampOutput(el,f,',',',');
/* Print out pbStamp as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* PBSTAMP_H */

