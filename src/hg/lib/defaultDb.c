/* defaultDb.c was originally generated by the autoSql program, which also 
 * generated defaultDb.h and defaultDb.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "defaultDb.h"


void defaultDbStaticLoad(char **row, struct defaultDb *ret)
/* Load a row from defaultDb table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->genome = row[0];
ret->name = row[1];
}

struct defaultDb *defaultDbLoad(char **row)
/* Load a defaultDb from row fetched with select * from defaultDb
 * from database.  Dispose of this with defaultDbFree(). */
{
struct defaultDb *ret;

AllocVar(ret);
ret->genome = cloneString(row[0]);
ret->name = cloneString(row[1]);
return ret;
}

struct defaultDb *defaultDbLoadAll(char *fileName) 
/* Load all defaultDb from a tab-separated file.
 * Dispose of this with defaultDbFreeList(). */
{
struct defaultDb *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = defaultDbLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct defaultDb *defaultDbCommaIn(char **pS, struct defaultDb *ret)
/* Create a defaultDb out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new defaultDb */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->genome = sqlStringComma(&s);
ret->name = sqlStringComma(&s);
*pS = s;
return ret;
}

void defaultDbFree(struct defaultDb **pEl)
/* Free a single dynamically allocated defaultDb such as created
 * with defaultDbLoad(). */
{
struct defaultDb *el;

if ((el = *pEl) == NULL) return;
freeMem(el->genome);
freeMem(el->name);
freez(pEl);
}

void defaultDbFreeList(struct defaultDb **pList)
/* Free a list of dynamically allocated defaultDb's */
{
struct defaultDb *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    defaultDbFree(&el);
    }
*pList = NULL;
}

void defaultDbOutput(struct defaultDb *el, FILE *f, char sep, char lastSep) 
/* Print out defaultDb.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genome);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

