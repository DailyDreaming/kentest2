/* txInfo.h was originally generated by the autoSql program, which also 
 * generated txInfo.c and txInfo.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2007 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef TXINFO_H
#define TXINFO_H

#define TXINFO_NUM_COLS 23

struct txInfo
/* Various bits of information about a transcript from the txGraph/txCds system (aka KG3) */
    {
    struct txInfo *next;  /* Next in singly linked list. */
    char *name;	/* Name of transcript */
    char *category;	/* coding/nearCoding/noncoding for now */
    char *sourceAcc;	/* Accession of genbank transcript patterned on (may be refSeq) */
    unsigned char isRefSeq;	/* Is a refSeq */
    int sourceSize;	/* Number of bases in source, excluding poly-A tail. */
    double aliCoverage;	/* Fraction of bases in source aligning. */
    double aliIdRatio;	/* matching/total bases in alignment */
    int genoMapCount;	/* Number of times source aligns in genome. */
    int exonCount;	/* Number of exons (excludes gaps from frame shift/stops) */
    int orfSize;	/* Size of ORF */
    double cdsScore;	/* Score of best CDS according to txCdsPredict */
    unsigned char startComplete;	/* Starts with ATG */
    unsigned char endComplete;	/* Ends with stop codon */
    unsigned char nonsenseMediatedDecay;	/* If true, is a nonsense mediated decay candidate. */
    unsigned char retainedIntron;	/* True if has a retained intron compared to overlapping transcripts */
    int bleedIntoIntron;	/* If nonzero number of bases start or end of tx bleeds into intron */
    int strangeSplice;	/* Count of splice sites not gt/ag, gc/ag, or at/ac */
    int atacIntrons;	/* Count of number of at/ac introns */
    unsigned char cdsSingleInIntron;	/* True if CDS is single exon and in intron of other transcript. */
    unsigned char cdsSingleInUtr3;	/* True if CDS is single exon and in 3' UTR of other transcript. */
    unsigned char selenocysteine;	/* If true TGA codes for selenocysteine */
    unsigned char genomicFrameShift;	/* True if genomic version has frame shift we cut out */
    unsigned char genomicStop;	/* True if genomic version has stop codon we cut out */
    };

void txInfoStaticLoad(char **row, struct txInfo *ret);
/* Load a row from txInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct txInfo *txInfoLoad(char **row);
/* Load a txInfo from row fetched with select * from txInfo
 * from database.  Dispose of this with txInfoFree(). */

struct txInfo *txInfoLoadAll(char *fileName);
/* Load all txInfo from whitespace-separated file.
 * Dispose of this with txInfoFreeList(). */

struct txInfo *txInfoLoadAllByChar(char *fileName, char chopper);
/* Load all txInfo from chopper separated file.
 * Dispose of this with txInfoFreeList(). */

#define txInfoLoadAllByTab(a) txInfoLoadAllByChar(a, '\t');
/* Load all txInfo from tab separated file.
 * Dispose of this with txInfoFreeList(). */

struct txInfo *txInfoCommaIn(char **pS, struct txInfo *ret);
/* Create a txInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new txInfo */

void txInfoFree(struct txInfo **pEl);
/* Free a single dynamically allocated txInfo such as created
 * with txInfoLoad(). */

void txInfoFreeList(struct txInfo **pList);
/* Free a list of dynamically allocated txInfo's */

void txInfoOutput(struct txInfo *el, FILE *f, char sep, char lastSep);
/* Print out txInfo.  Separate fields with sep. Follow last field with lastSep. */

#define txInfoTabOut(el,f) txInfoOutput(el,f,'\t','\n');
/* Print out txInfo as a line in a tab-separated file. */

#define txInfoCommaOut(el,f) txInfoOutput(el,f,',',',');
/* Print out txInfo as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

double txInfoCodingScore(struct txInfo *info, boolean boostRefSeq);
/* Return coding score for info.  This is just the cdsScore score,
 * plus another 1000 if it's a refSeq.  1000 is quite a bit for a
 * cdsScore score, equivalent to another 1000 bases in the coding region. */

#endif /* TXINFO_H */

