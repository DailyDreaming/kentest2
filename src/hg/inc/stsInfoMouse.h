/* stsInfoMouse.h was originally generated by the autoSql program, which also 
 * generated stsInfoMouse.c and stsInfoMouse.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2002 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef STSINFOMOUSE_H
#define STSINFOMOUSE_H

struct stsInfoMouse
/* Constant STS marker information */
    {
    struct stsInfoMouse *next;  /* Next in singly linked list. */
    unsigned identNo;	/* UCSC identification number */
    char *name;	/* Official UCSC name */
    unsigned MGIPrimerID;	/* sts Primer's MGI ID or 0 if N/A */
    char *primerName;	/* sts Primer's name */
    char *primerSymbol;	/* sts Primer's symbol */
    char *primer1;	/* primer1 sequence */
    char *primer2;	/* primer2 sequence */
    char *distance;	/* Length of STS sequence */
    unsigned sequence;	/* Whether the full sequence is available (1) or not (0) for STS */
    unsigned MGIMarkerID;	/* sts Marker's MGI ID or 0 if N/A */
    char *stsMarkerSymbol;	/* symbol of  STS marker */
    char *Chr;	/* Chromosome in Genetic map */
    float geneticPos;	/* Position in Genetic map. -2 if N/A, -1 if syntenic  */
    char *stsMarkerName;	/* Name of sts Marker. */
    unsigned LocusLinkID;	/* Locuslink Id, 0 if N/A */
    };

void stsInfoMouseStaticLoad(char **row, struct stsInfoMouse *ret);
/* Load a row from stsInfoMouse table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct stsInfoMouse *stsInfoMouseLoad(char **row);
/* Load a stsInfoMouse from row fetched with select * from stsInfoMouse
 * from database.  Dispose of this with stsInfoMouseFree(). */

struct stsInfoMouse *stsInfoMouseLoadAll(char *fileName);
/* Load all stsInfoMouse from a tab-separated file.
 * Dispose of this with stsInfoMouseFreeList(). */

struct stsInfoMouse *stsInfoMouseLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all stsInfoMouse from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with stsInfoMouseFreeList(). */

struct stsInfoMouse *stsInfoMouseCommaIn(char **pS, struct stsInfoMouse *ret);
/* Create a stsInfoMouse out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new stsInfoMouse */

void stsInfoMouseFree(struct stsInfoMouse **pEl);
/* Free a single dynamically allocated stsInfoMouse such as created
 * with stsInfoMouseLoad(). */

void stsInfoMouseFreeList(struct stsInfoMouse **pList);
/* Free a list of dynamically allocated stsInfoMouse's */

void stsInfoMouseOutput(struct stsInfoMouse *el, FILE *f, char sep, char lastSep);
/* Print out stsInfoMouse.  Separate fields with sep. Follow last field with lastSep. */

#define stsInfoMouseTabOut(el,f) stsInfoMouseOutput(el,f,'\t','\n');
/* Print out stsInfoMouse as a line in a tab-separated file. */

#define stsInfoMouseCommaOut(el,f) stsInfoMouseOutput(el,f,',',',');
/* Print out stsInfoMouse as a comma separated list including final comma. */

#endif /* STSINFOMOUSE_H */

