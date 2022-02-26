/* rmskAlign.h was originally generated by the autoSql program, which also 
 * generated rmskAlign.c and rmskAlign.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef RMSKALIGN_H
#define RMSKALIGN_H

#define RMSKALIGN_NUM_COLS 17

struct rmskAlign
/* RepeatMasker .align record */
    {
    struct rmskAlign *next;  /* Next in singly linked list. */
    unsigned swScore;	/* Smith Waterman alignment score */
    unsigned milliDiv;	/* Base mismatches in parts per thousand */
    unsigned milliDel;	/* Bases deleted in parts per thousand */
    unsigned milliIns;	/* Bases inserted in parts per thousand */
    char *genoName;	/* Genomic sequence name */
    unsigned genoStart;	/* Start in genomic sequence */
    unsigned genoEnd;	/* End in genomic sequence */
    int genoLeft;	/* -#bases after match in genomic sequence */
    char strand[2];	/* Relative orientation + or - */
    char *repName;	/* Name of repeat */
    char *repClass;	/* Class of repeat */
    char *repFamily;	/* Family of repeat (1-based) */
    int repStart;	/* Start in repeat sequence */
    unsigned repEnd;	/* End in repeat sequence */
    int repLeft;	/* -#bases after match in repeat sequence */
    unsigned id;	/* The ID of the hit. Used to link related fragments */
    char *alignment;	/* The alignment data stored as a single string */
    };

void rmskAlignStaticLoad(char **row, struct rmskAlign *ret);
/* Load a row from rmskAlign table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct rmskAlign *rmskAlignLoad(char **row);
/* Load a rmskAlign from row fetched with select * from rmskAlign
 * from database.  Dispose of this with rmskAlignFree(). */

struct rmskAlign *rmskAlignLoadAll(char *fileName);
/* Load all rmskAlign from whitespace-separated file.
 * Dispose of this with rmskAlignFreeList(). */

struct rmskAlign *rmskAlignLoadAllByChar(char *fileName, char chopper);
/* Load all rmskAlign from chopper separated file.
 * Dispose of this with rmskAlignFreeList(). */

#define rmskAlignLoadAllByTab(a) rmskAlignLoadAllByChar(a, '\t');
/* Load all rmskAlign from tab separated file.
 * Dispose of this with rmskAlignFreeList(). */

struct rmskAlign *rmskAlignCommaIn(char **pS, struct rmskAlign *ret);
/* Create a rmskAlign out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rmskAlign */

void rmskAlignFree(struct rmskAlign **pEl);
/* Free a single dynamically allocated rmskAlign such as created
 * with rmskAlignLoad(). */

void rmskAlignFreeList(struct rmskAlign **pList);
/* Free a list of dynamically allocated rmskAlign's */

void rmskAlignOutput(struct rmskAlign *el, FILE *f, char sep, char lastSep);
/* Print out rmskAlign.  Separate fields with sep. Follow last field with lastSep. */

#define rmskAlignTabOut(el,f) rmskAlignOutput(el,f,'\t','\n');
/* Print out rmskAlign as a line in a tab-separated file. */

#define rmskAlignCommaOut(el,f) rmskAlignOutput(el,f,',',',');
/* Print out rmskAlign as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* RMSKALIGN_H */

