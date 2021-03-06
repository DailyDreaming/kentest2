/* itemDetailsHtml.h was originally generated by the autoSql program, which also 
 * generated itemDetailsHtml.c and itemDetailsHtml.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2012 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef ITEMDETAILSHTML_H
#define ITEMDETAILSHTML_H

#define ITEMDETAILSHTML_NUM_COLS 2

struct itemDetailsHtml
/* table that allows adding html to details page for various items */
    {
    struct itemDetailsHtml *next;  /* Next in singly linked list. */
    char *name;	/* Name of item */
    char *html;	/* HTML fragment to include */
    };

void itemDetailsHtmlStaticLoad(char **row, struct itemDetailsHtml *ret);
/* Load a row from itemDetailsHtml table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct itemDetailsHtml *itemDetailsHtmlLoad(char **row);
/* Load a itemDetailsHtml from row fetched with select * from itemDetailsHtml
 * from database.  Dispose of this with itemDetailsHtmlFree(). */

struct itemDetailsHtml *itemDetailsHtmlLoadAll(char *fileName);
/* Load all itemDetailsHtml from whitespace-separated file.
 * Dispose of this with itemDetailsHtmlFreeList(). */

struct itemDetailsHtml *itemDetailsHtmlLoadAllByChar(char *fileName, char chopper);
/* Load all itemDetailsHtml from chopper separated file.
 * Dispose of this with itemDetailsHtmlFreeList(). */

#define itemDetailsHtmlLoadAllByTab(a) itemDetailsHtmlLoadAllByChar(a, '\t');
/* Load all itemDetailsHtml from tab separated file.
 * Dispose of this with itemDetailsHtmlFreeList(). */

struct itemDetailsHtml *itemDetailsHtmlCommaIn(char **pS, struct itemDetailsHtml *ret);
/* Create a itemDetailsHtml out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new itemDetailsHtml */

void itemDetailsHtmlFree(struct itemDetailsHtml **pEl);
/* Free a single dynamically allocated itemDetailsHtml such as created
 * with itemDetailsHtmlLoad(). */

void itemDetailsHtmlFreeList(struct itemDetailsHtml **pList);
/* Free a list of dynamically allocated itemDetailsHtml's */

void itemDetailsHtmlOutput(struct itemDetailsHtml *el, FILE *f, char sep, char lastSep);
/* Print out itemDetailsHtml.  Separate fields with sep. Follow last field with lastSep. */

#define itemDetailsHtmlTabOut(el,f) itemDetailsHtmlOutput(el,f,'\t','\n');
/* Print out itemDetailsHtml as a line in a tab-separated file. */

#define itemDetailsHtmlCommaOut(el,f) itemDetailsHtmlOutput(el,f,',',',');
/* Print out itemDetailsHtml as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* ITEMDETAILSHTML_H */

