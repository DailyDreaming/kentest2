/* rnaFold.c was originally generated by the autoSql program, which also 
 * generated rnaFold.h and rnaFold.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rnaFold.h"


void rnaFoldStaticLoad(char **row, struct rnaFold *ret)
/* Load a row from rnaFold table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->seq = row[1];
ret->fold = row[2];
ret->energy = atof(row[3]);
}

struct rnaFold *rnaFoldLoad(char **row)
/* Load a rnaFold from row fetched with select * from rnaFold
 * from database.  Dispose of this with rnaFoldFree(). */
{
struct rnaFold *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->seq = cloneString(row[1]);
ret->fold = cloneString(row[2]);
ret->energy = atof(row[3]);
return ret;
}

struct rnaFold *rnaFoldLoadAll(char *fileName) 
/* Load all rnaFold from a whitespace-separated file.
 * Dispose of this with rnaFoldFreeList(). */
{
struct rnaFold *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = rnaFoldLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rnaFold *rnaFoldLoadAllByChar(char *fileName, char chopper) 
/* Load all rnaFold from a chopper separated file.
 * Dispose of this with rnaFoldFreeList(). */
{
struct rnaFold *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = rnaFoldLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rnaFold *rnaFoldCommaIn(char **pS, struct rnaFold *ret)
/* Create a rnaFold out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rnaFold */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->seq = sqlStringComma(&s);
ret->fold = sqlStringComma(&s);
ret->energy = sqlFloatComma(&s);
*pS = s;
return ret;
}

void rnaFoldFree(struct rnaFold **pEl)
/* Free a single dynamically allocated rnaFold such as created
 * with rnaFoldLoad(). */
{
struct rnaFold *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->seq);
freeMem(el->fold);
freez(pEl);
}

void rnaFoldFreeList(struct rnaFold **pList)
/* Free a list of dynamically allocated rnaFold's */
{
struct rnaFold *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rnaFoldFree(&el);
    }
*pList = NULL;
}

void rnaFoldOutput(struct rnaFold *el, FILE *f, char sep, char lastSep) 
/* Print out rnaFold.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->seq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->fold);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->energy);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

