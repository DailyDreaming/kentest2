/* dbDb.h was originally generated by the autoSql program, which also 
 * generated dbDb.c and dbDb.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2004, 2016 The Regents of the University of California
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef DBDB_H
#define DBDB_H

#define DBDB_NUM_COLS 14

extern char *dbDbCommaSepFieldNames;

struct dbDb
/* Description of annotation database */
    {
    struct dbDb *next;  /* Next in singly linked list. */
    char *name;	/* Short name of database.  'hg8' or the like */
    char *description;	/* Short description - 'Aug. 8, 2001' or the like */
    char *nibPath;	/* Path to packed sequence files */
    char *organism;	/* Common name of organism - first letter capitalized */
    char *defaultPos;	/* Default starting position */
    int active;	/* Flag indicating whether this db is in active use */
    int orderKey;	/* Int used to control display order within a genome */
    char *genome;	/* Unifying genome collection to which an assembly belongs */
    char *scientificName;	/* Genus and species of the organism; e.g. Homo sapiens */
    char *htmlPath;	/* path in /gbdb for assembly description */
    signed char hgNearOk;	/* Have hgNear for this? */
    signed char hgPbOk;	/* Have pbTracks for this? */
    char *sourceName;	/* Source build/release/version of the assembly */
    int taxId;	/* NCBI Taxonomy ID for genome */
    };

void dbDbStaticLoad(char **row, struct dbDb *ret);
/* Load a row from dbDb table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct dbDb *dbDbLoad(char **row);
/* Load a dbDb from row fetched with select * from dbDb
 * from database.  Dispose of this with dbDbFree(). */

struct dbDb *dbDbLoadAll(char *fileName);
/* Load all dbDb from whitespace-separated file.
 * Dispose of this with dbDbFreeList(). */

struct dbDb *dbDbLoadAllByChar(char *fileName, char chopper);
/* Load all dbDb from chopper separated file.
 * Dispose of this with dbDbFreeList(). */

#define dbDbLoadAllByTab(a) dbDbLoadAllByChar(a, '\t');
/* Load all dbDb from tab separated file.
 * Dispose of this with dbDbFreeList(). */

struct dbDb *dbDbCommaIn(char **pS, struct dbDb *ret);
/* Create a dbDb out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new dbDb */

void dbDbFree(struct dbDb **pEl);
/* Free a single dynamically allocated dbDb such as created
 * with dbDbLoad(). */

void dbDbFreeList(struct dbDb **pList);
/* Free a list of dynamically allocated dbDb's */

void dbDbOutput(struct dbDb *el, FILE *f, char sep, char lastSep);
/* Print out dbDb.  Separate fields with sep. Follow last field with lastSep. */

#define dbDbTabOut(el,f) dbDbOutput(el,f,'\t','\n');
/* Print out dbDb as a line in a tab-separated file. */

#define dbDbCommaOut(el,f) dbDbOutput(el,f,',',',');
/* Print out dbDb as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* DBDB_H */

