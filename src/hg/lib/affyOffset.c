/* affyOffset.c was originally generated by the autoSql program, which also 
 * generated affyOffset.h and affyOffset.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "affyOffset.h"


void affyOffsetStaticLoad(char **row, struct affyOffset *ret)
/* Load a row from affyOffset table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
ret->piece = row[0];
ret->tStart = sqlUnsigned(row[1]);
}

struct affyOffset *affyOffsetLoad(char **row)
/* Load a affyOffset from row fetched with select * from affyOffset
 * from database.  Dispose of this with affyOffsetFree(). */
{
struct affyOffset *ret;

AllocVar(ret);
ret->piece = cloneString(row[0]);
ret->tStart = sqlUnsigned(row[1]);
return ret;
}

struct affyOffset *affyOffsetLoadAll(char *fileName) 
/* Load all affyOffset from a tab-separated file.
 * Dispose of this with affyOffsetFreeList(). */
{
struct affyOffset *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = affyOffsetLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct affyOffset *affyOffsetCommaIn(char **pS, struct affyOffset *ret)
/* Create a affyOffset out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new affyOffset */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->piece = sqlStringComma(&s);
ret->tStart = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void affyOffsetFree(struct affyOffset **pEl)
/* Free a single dynamically allocated affyOffset such as created
 * with affyOffsetLoad(). */
{
struct affyOffset *el;

if ((el = *pEl) == NULL) return;
freeMem(el->piece);
freez(pEl);
}

void affyOffsetFreeList(struct affyOffset **pList)
/* Free a list of dynamically allocated affyOffset's */
{
struct affyOffset *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    affyOffsetFree(&el);
    }
*pList = NULL;
}

void affyOffsetOutput(struct affyOffset *el, FILE *f, char sep, char lastSep) 
/* Print out affyOffset.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->piece);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->tStart);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

