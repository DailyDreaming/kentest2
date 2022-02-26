/* txCluster.h was originally generated by the autoSql program, which also 
 * generated txCluster.c and txCluster.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef TXCLUSTER_H
#define TXCLUSTER_H

#define TXCLUSTER_NUM_COLS 8

struct txCluster
/* A cluster of transcripts or coding regions */
    {
    struct txCluster *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome (or contig) */
    int chromStart;	/* Zero based start within chromosome. */
    int chromEnd;	/* End coordinate, non inclusive. */
    char *name;	/* Cluster name */
    int score;	/* BED score - 0-1000.  0 if not used. */
    char strand[2];	/* Strand - either plus or minus */
    int txCount;	/* Count of transcripts */
    char **txArray;	/* Array of transcripts */
    };

struct txCluster *txClusterLoad(char **row);
/* Load a txCluster from row fetched with select * from txCluster
 * from database.  Dispose of this with txClusterFree(). */

struct txCluster *txClusterLoadAll(char *fileName);
/* Load all txCluster from whitespace-separated file.
 * Dispose of this with txClusterFreeList(). */

struct txCluster *txClusterLoadAllByChar(char *fileName, char chopper);
/* Load all txCluster from chopper separated file.
 * Dispose of this with txClusterFreeList(). */

#define txClusterLoadAllByTab(a) txClusterLoadAllByChar(a, '\t');
/* Load all txCluster from tab separated file.
 * Dispose of this with txClusterFreeList(). */

struct txCluster *txClusterCommaIn(char **pS, struct txCluster *ret);
/* Create a txCluster out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new txCluster */

void txClusterFree(struct txCluster **pEl);
/* Free a single dynamically allocated txCluster such as created
 * with txClusterLoad(). */

void txClusterFreeList(struct txCluster **pList);
/* Free a list of dynamically allocated txCluster's */

void txClusterOutput(struct txCluster *el, FILE *f, char sep, char lastSep);
/* Print out txCluster.  Separate fields with sep. Follow last field with lastSep. */

#define txClusterTabOut(el,f) txClusterOutput(el,f,'\t','\n');
/* Print out txCluster as a line in a tab-separated file. */

#define txClusterCommaOut(el,f) txClusterOutput(el,f,',',',');
/* Print out txCluster as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* TXCLUSTER_H */

