/* genomicSuperDups.h was originally generated by the autoSql program, which also 
 * generated genomicSuperDups.c and genomicSuperDups.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2002 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef GENOMICSUPERDUPS_H
#define GENOMICSUPERDUPS_H

struct genomicSuperDups
/* Summary of large genomic Duplications (>1KB >90% similar) */
    {
    struct genomicSuperDups *next;  /* Next in singly linked list. */
    char *chrom;	/* Human chromosome or FPC contig */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Other chromosome involved */
    unsigned score;	/* Score from 900-1000.  1000 is best */
    char strand[2];	/* Value should be + or - */
    char *otherChrom;	/* Other Human chromosome or FPC contig */
    unsigned otherStart;	/* Start in other  sequence */
    unsigned otherEnd;	/* End in other  sequence */
    unsigned otherSize;	/* Total size of other sequence */
    unsigned uid;	/* unique id */
    unsigned posBasesHit;	/* HitPositive UnCovered */
    char *testResult;	/* HitPositive (yes or no) UnCovered (covered=0) */
    char *verdict;	/* Real or Allele */
    char *chits;	/* dup positive coverage */
    char *ccov;	/* clone coverage celera screened */
    char *alignfile;	/* alignment file path */
    unsigned alignL;	/* spaces/positions in alignment */
    unsigned indelN;	/* number of indels */
    unsigned indelS;	/* indel spaces */
    unsigned alignB;	/* bases Aligned */
    unsigned matchB;	/* aligned bases that match */
    unsigned mismatchB;	/* aligned bases that do not match */
    unsigned transitionsB;	/* number of transitions */
    unsigned transversionsB;	/* number of transversions */
    float fracMatch;	/* fraction of matching bases */
    float fracMatchIndel;	/* fraction of matching bases with indels */
    float jcK;	/* K-value calculated with Jukes-Cantor */
    float k2K;	/* Kimura K */
    };

void genomicSuperDupsStaticLoad(char **row, struct genomicSuperDups *ret);
/* Load a row from genomicSuperDups table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct genomicSuperDups *genomicSuperDupsLoad(char **row);
/* Load a genomicSuperDups from row fetched with select * from genomicSuperDups
 * from database.  Dispose of this with genomicSuperDupsFree(). */

struct genomicSuperDups *genomicSuperDupsLoadAll(char *fileName);
/* Load all genomicSuperDups from a tab-separated file.
 * Dispose of this with genomicSuperDupsFreeList(). */

struct genomicSuperDups *genomicSuperDupsLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all genomicSuperDups from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with genomicSuperDupsFreeList(). */

struct genomicSuperDups *genomicSuperDupsCommaIn(char **pS, struct genomicSuperDups *ret);
/* Create a genomicSuperDups out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new genomicSuperDups */

void genomicSuperDupsFree(struct genomicSuperDups **pEl);
/* Free a single dynamically allocated genomicSuperDups such as created
 * with genomicSuperDupsLoad(). */

void genomicSuperDupsFreeList(struct genomicSuperDups **pList);
/* Free a list of dynamically allocated genomicSuperDups's */

void genomicSuperDupsOutput(struct genomicSuperDups *el, FILE *f, char sep, char lastSep);
/* Print out genomicSuperDups.  Separate fields with sep. Follow last field with lastSep. */

#define genomicSuperDupsTabOut(el,f) genomicSuperDupsOutput(el,f,'\t','\n');
/* Print out genomicSuperDups as a line in a tab-separated file. */

#define genomicSuperDupsCommaOut(el,f) genomicSuperDupsOutput(el,f,',',',');
/* Print out genomicSuperDups as a comma separated list including final comma. */

#endif /* GENOMICSUPERDUPS_H */

