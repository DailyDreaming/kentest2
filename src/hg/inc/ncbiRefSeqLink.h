/* ncbiRefSeqLink.h was originally generated by the autoSql program, which also 
 * generated ncbiRefSeqLink.c and ncbiRefSeqLink.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef NCBIREFSEQLINK_H
#define NCBIREFSEQLINK_H

#define NCBIREFSEQLINK_NUM_COLS 19

extern char *ncbiRefSeqLinkCommaSepFieldNames;

struct ncbiRefSeqLink
/* Metadata for NCBI RefSeq tracks */
    {
    struct ncbiRefSeqLink *next;  /* Next in singly linked list. */
    char *id;	/* id for this gene or curated item */
    char *status;	/* Inferred, Model, Predicted, Provisional, Reviewed, Validated, Unknown */
    char *name;	/* gene name */
    char *product;	/* product */
    char *mrnaAcc;	/* transcript_id */
    char *protAcc;	/* protein_id */
    char *locusLinkId;	/* locus link identifier, from Dbxref */
    char *omimId;	/* OMIM identifier, from Dbxref */
    char *hgnc;	/* HGNC identifier, from Dbxref */
    char *genbank;	/* genbank identifier from Dbxref */
    char *pseudo;	/* 'true' if pseudo gene, or n/a */
    char *gbkey;	/* genbank key: Gene, mRNA, ncRNA, rRNA, tRNA, etc... */
    char *source;	/* source: RefSeq, tRNAscan-SE, Gnomon, Curated Genomic, BestRefSeq */
    char *gene_biotype;	/* bio type: protein_coding, pseudogene, C_region, J_segment_pseudogene, other */
    char *gene_synonym;	/* list of synonym names */
    char *ncrna_class;	/* type of RNA: miRNA, lncRNA, snoRNA, etc... */
    char *note;	/* other notes from genbank record */
    char *description;	/* description from rna gbff record via gbProcess */
    char *externalId;	/* for outside URL link, WormBase, FlyBase, RGD, SGD, etc... from Dbxref */
    };

void ncbiRefSeqLinkStaticLoad(char **row, struct ncbiRefSeqLink *ret);
/* Load a row from ncbiRefSeqLink table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct ncbiRefSeqLink *ncbiRefSeqLinkLoad(char **row);
/* Load a ncbiRefSeqLink from row fetched with select * from ncbiRefSeqLink
 * from database.  Dispose of this with ncbiRefSeqLinkFree(). */

struct ncbiRefSeqLink *ncbiRefSeqLinkLoadAll(char *fileName);
/* Load all ncbiRefSeqLink from whitespace-separated file.
 * Dispose of this with ncbiRefSeqLinkFreeList(). */

struct ncbiRefSeqLink *ncbiRefSeqLinkLoadAllByChar(char *fileName, char chopper);
/* Load all ncbiRefSeqLink from chopper separated file.
 * Dispose of this with ncbiRefSeqLinkFreeList(). */

#define ncbiRefSeqLinkLoadAllByTab(a) ncbiRefSeqLinkLoadAllByChar(a, '\t');
/* Load all ncbiRefSeqLink from tab separated file.
 * Dispose of this with ncbiRefSeqLinkFreeList(). */

struct ncbiRefSeqLink *ncbiRefSeqLinkCommaIn(char **pS, struct ncbiRefSeqLink *ret);
/* Create a ncbiRefSeqLink out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ncbiRefSeqLink */

void ncbiRefSeqLinkFree(struct ncbiRefSeqLink **pEl);
/* Free a single dynamically allocated ncbiRefSeqLink such as created
 * with ncbiRefSeqLinkLoad(). */

void ncbiRefSeqLinkFreeList(struct ncbiRefSeqLink **pList);
/* Free a list of dynamically allocated ncbiRefSeqLink's */

void ncbiRefSeqLinkOutput(struct ncbiRefSeqLink *el, FILE *f, char sep, char lastSep);
/* Print out ncbiRefSeqLink.  Separate fields with sep. Follow last field with lastSep. */

#define ncbiRefSeqLinkTabOut(el,f) ncbiRefSeqLinkOutput(el,f,'\t','\n');
/* Print out ncbiRefSeqLink as a line in a tab-separated file. */

#define ncbiRefSeqLinkCommaOut(el,f) ncbiRefSeqLinkOutput(el,f,',',',');
/* Print out ncbiRefSeqLink as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* NCBIREFSEQLINK_H */
