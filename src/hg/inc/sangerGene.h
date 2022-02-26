/* sangerGene.h was originally generated by the autoSql program, which also 
 * generated sangerGene.c and sangerGene.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2004 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef SANGERGENE_H
#define SANGERGENE_H

#define SANGERGENE_NUM_COLS 11

struct sangerGene
/* GenePred table with proteinID field for WormBase Genes. */
    {
    struct sangerGene *next;  /* Next in singly linked list. */
    char *name;	/* Name of gene */
    char *chrom;	/* Chromosome name */
    char strand[2];	/* + or - for strand */
    unsigned txStart;	/* Transcription start position */
    unsigned txEnd;	/* Transcription end position */
    unsigned cdsStart;	/* Coding region start */
    unsigned cdsEnd;	/* Coding region end */
    unsigned exonCount;	/* Number of exons */
    unsigned *exonStarts;	/* Exon start positions */
    unsigned *exonEnds;	/* Exon end positions */
    char *proteinID;	/* Swiss-Prot protein ID */
    };

struct sangerGene *sangerGeneLoad(char **row);
/* Load a sangerGene from row fetched with select * from sangerGene
 * from database.  Dispose of this with sangerGeneFree(). */

struct sangerGene *sangerGeneLoadAll(char *fileName);
/* Load all sangerGene from whitespace-separated file.
 * Dispose of this with sangerGeneFreeList(). */

struct sangerGene *sangerGeneLoadAllByChar(char *fileName, char chopper);
/* Load all sangerGene from chopper separated file.
 * Dispose of this with sangerGeneFreeList(). */

#define sangerGeneLoadAllByTab(a) sangerGeneLoadAllByChar(a, '\t');
/* Load all sangerGene from tab separated file.
 * Dispose of this with sangerGeneFreeList(). */

struct sangerGene *sangerGeneCommaIn(char **pS, struct sangerGene *ret);
/* Create a sangerGene out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new sangerGene */

void sangerGeneFree(struct sangerGene **pEl);
/* Free a single dynamically allocated sangerGene such as created
 * with sangerGeneLoad(). */

void sangerGeneFreeList(struct sangerGene **pList);
/* Free a list of dynamically allocated sangerGene's */

void sangerGeneOutput(struct sangerGene *el, FILE *f, char sep, char lastSep);
/* Print out sangerGene.  Separate fields with sep. Follow last field with lastSep. */

#define sangerGeneTabOut(el,f) sangerGeneOutput(el,f,'\t','\n');
/* Print out sangerGene as a line in a tab-separated file. */

#define sangerGeneCommaOut(el,f) sangerGeneOutput(el,f,',',',');
/* Print out sangerGene as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SANGERGENE_H */

