/* vgPrbAliAll.h was originally generated by the autoSql program, which also 
 * generated vgPrbAliAll.c and vgPrbAliAll.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef VGPRBALIALL_H
#define VGPRBALIALL_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define VGPRBALIALL_NUM_COLS 3

struct vgPrbAliAll
/* Define Probe Alignment status for track vgAllProbes */
    {
    struct vgPrbAliAll *next;  /* Next in singly linked list. */
    char *db;	/* assembly */
    int vgPrb;	/* vgPrb id */
    char *status;	/* new,ali,none */
    };

void vgPrbAliAllStaticLoad(char **row, struct vgPrbAliAll *ret);
/* Load a row from vgPrbAliAll table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct vgPrbAliAll *vgPrbAliAllLoad(char **row);
/* Load a vgPrbAliAll from row fetched with select * from vgPrbAliAll
 * from database.  Dispose of this with vgPrbAliAllFree(). */

struct vgPrbAliAll *vgPrbAliAllLoadAll(char *fileName);
/* Load all vgPrbAliAll from whitespace-separated file.
 * Dispose of this with vgPrbAliAllFreeList(). */

struct vgPrbAliAll *vgPrbAliAllLoadAllByChar(char *fileName, char chopper);
/* Load all vgPrbAliAll from chopper separated file.
 * Dispose of this with vgPrbAliAllFreeList(). */

#define vgPrbAliAllLoadAllByTab(a) vgPrbAliAllLoadAllByChar(a, '\t');
/* Load all vgPrbAliAll from tab separated file.
 * Dispose of this with vgPrbAliAllFreeList(). */

struct vgPrbAliAll *vgPrbAliAllLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all vgPrbAliAll from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with vgPrbAliAllFreeList(). */

void vgPrbAliAllSaveToDb(struct sqlConnection *conn, struct vgPrbAliAll *el, char *tableName, int updateSize);
/* Save vgPrbAliAll as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */

struct vgPrbAliAll *vgPrbAliAllCommaIn(char **pS, struct vgPrbAliAll *ret);
/* Create a vgPrbAliAll out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new vgPrbAliAll */

void vgPrbAliAllFree(struct vgPrbAliAll **pEl);
/* Free a single dynamically allocated vgPrbAliAll such as created
 * with vgPrbAliAllLoad(). */

void vgPrbAliAllFreeList(struct vgPrbAliAll **pList);
/* Free a list of dynamically allocated vgPrbAliAll's */

void vgPrbAliAllOutput(struct vgPrbAliAll *el, FILE *f, char sep, char lastSep);
/* Print out vgPrbAliAll.  Separate fields with sep. Follow last field with lastSep. */

#define vgPrbAliAllTabOut(el,f) vgPrbAliAllOutput(el,f,'\t','\n');
/* Print out vgPrbAliAll as a line in a tab-separated file. */

#define vgPrbAliAllCommaOut(el,f) vgPrbAliAllOutput(el,f,',',',');
/* Print out vgPrbAliAll as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* VGPRBALIALL_H */
