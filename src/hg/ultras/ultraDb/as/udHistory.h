/* udHistory.h was originally generated by the autoSql program, which also 
 * generated udHistory.c and udHistory.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2004 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef UDHISTORY_H
#define UDHISTORY_H

#define UDHISTORY_NUM_COLS 4

struct udHistory
/* Records alterations made to ultra database */
    {
    struct udHistory *next;  /* Next in singly linked list. */
    char *who;	/* Name of user who made change */
    char *program;	/* Name of program that made change */
    char *time;	/* When alteration was made */
    char *changes;	/* CGI-ENCODED string describing changes */
    };

void udHistoryStaticLoad(char **row, struct udHistory *ret);
/* Load a row from udHistory table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct udHistory *udHistoryLoad(char **row);
/* Load a udHistory from row fetched with select * from udHistory
 * from database.  Dispose of this with udHistoryFree(). */

struct udHistory *udHistoryLoadAll(char *fileName);
/* Load all udHistory from whitespace-separated file.
 * Dispose of this with udHistoryFreeList(). */

struct udHistory *udHistoryLoadAllByChar(char *fileName, char chopper);
/* Load all udHistory from chopper separated file.
 * Dispose of this with udHistoryFreeList(). */

#define udHistoryLoadAllByTab(a) udHistoryLoadAllByChar(a, '\t');
/* Load all udHistory from tab separated file.
 * Dispose of this with udHistoryFreeList(). */

struct udHistory *udHistoryCommaIn(char **pS, struct udHistory *ret);
/* Create a udHistory out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new udHistory */

void udHistoryFree(struct udHistory **pEl);
/* Free a single dynamically allocated udHistory such as created
 * with udHistoryLoad(). */

void udHistoryFreeList(struct udHistory **pList);
/* Free a list of dynamically allocated udHistory's */

void udHistoryOutput(struct udHistory *el, FILE *f, char sep, char lastSep);
/* Print out udHistory.  Separate fields with sep. Follow last field with lastSep. */

#define udHistoryTabOut(el,f) udHistoryOutput(el,f,'\t','\n');
/* Print out udHistory as a line in a tab-separated file. */

#define udHistoryCommaOut(el,f) udHistoryOutput(el,f,',',',');
/* Print out udHistory as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* UDHISTORY_H */

