/* parSpec.c was originally generated by the autoSql program, which also 
 * generated parSpec.h and parSpec.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "parSpec.h"


struct parSpec *parSpecLoad(char **row)
/* Load a parSpec from row fetched with select * from parSpec
 * from database.  Dispose of this with parSpecFree(). */
{
struct parSpec *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->chromA = cloneString(row[1]);
ret->startA = sqlSigned(row[2]);
ret->endA = sqlSigned(row[3]);
ret->chromB = cloneString(row[4]);
ret->startB = sqlSigned(row[5]);
ret->endB = sqlSigned(row[6]);
return ret;
}

struct parSpec *parSpecLoadAll(char *fileName) 
/* Load all parSpec from a whitespace-separated file.
 * Dispose of this with parSpecFreeList(). */
{
struct parSpec *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = parSpecLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct parSpec *parSpecLoadAllByChar(char *fileName, char chopper) 
/* Load all parSpec from a chopper separated file.
 * Dispose of this with parSpecFreeList(). */
{
struct parSpec *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = parSpecLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct parSpec *parSpecCommaIn(char **pS, struct parSpec *ret)
/* Create a parSpec out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new parSpec */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->chromA = sqlStringComma(&s);
ret->startA = sqlSignedComma(&s);
ret->endA = sqlSignedComma(&s);
ret->chromB = sqlStringComma(&s);
ret->startB = sqlSignedComma(&s);
ret->endB = sqlSignedComma(&s);
*pS = s;
return ret;
}

void parSpecFree(struct parSpec **pEl)
/* Free a single dynamically allocated parSpec such as created
 * with parSpecLoad(). */
{
struct parSpec *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->chromA);
freeMem(el->chromB);
freez(pEl);
}

void parSpecFreeList(struct parSpec **pList)
/* Free a list of dynamically allocated parSpec's */
{
struct parSpec *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    parSpecFree(&el);
    }
*pList = NULL;
}

void parSpecOutput(struct parSpec *el, FILE *f, char sep, char lastSep) 
/* Print out parSpec.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chromA);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->startA);
fputc(sep,f);
fprintf(f, "%d", el->endA);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chromB);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->startB);
fputc(sep,f);
fprintf(f, "%d", el->endB);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

