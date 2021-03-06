/* rnaSecStr.c was originally generated by the autoSql program, which also 
 * generated rnaSecStr.h and rnaSecStr.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rnaSecStr.h"


struct rnaSecStr *rnaSecStrLoad(char **row)
/* Load a rnaSecStr from row fetched with select * from rnaSecStr
 * from database. Ignores conf column if present.  Dispose of this with rnaSecStrFree(). */
{
struct rnaSecStr *ret;

AllocVar(ret);
ret->size = sqlUnsigned(row[6]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->secStr = cloneString(row[7]);
return ret;
}

struct rnaSecStr *rnaSecStrLoadConf(char **row)
/* Load a rnaSecStr from row fetched with select * from rnaSecStr
 * from database. Loads conf column.  Dispose of this with rnaSecStrFree(). */
{
struct rnaSecStr *ret;

AllocVar(ret);
ret->size = sqlUnsigned(row[6]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->secStr = cloneString(row[7]);
{
int sizeOne;
sqlDoubleDynamicArray(row[8], &ret->conf, &sizeOne);
assert(sizeOne == ret->size);
}
return ret;
}

struct rnaSecStr *rnaSecStrLoadAll(char *fileName) 
/* Load all rnaSecStr from a whitespace-separated file.
 * Dispose of this with rnaSecStrFreeList(). */
{
struct rnaSecStr *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileRow(lf, row))
    {
    el = rnaSecStrLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rnaSecStr *rnaSecStrLoadAllByChar(char *fileName, char chopper) 
/* Load all rnaSecStr from a chopper separated file.
 * Dispose of this with rnaSecStrFreeList(). */
{
struct rnaSecStr *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = rnaSecStrLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rnaSecStr *rnaSecStrCommaIn(char **pS, struct rnaSecStr *ret)
/* Create a rnaSecStr out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rnaSecStr */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->size = sqlUnsignedComma(&s);
ret->secStr = sqlStringComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->conf, ret->size);
for (i=0; i<ret->size; ++i)
    {
    ret->conf[i] = sqlDoubleComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void rnaSecStrFree(struct rnaSecStr **pEl)
/* Free a single dynamically allocated rnaSecStr such as created
 * with rnaSecStrLoad(). */
{
struct rnaSecStr *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->secStr);
freeMem(el->conf);
freez(pEl);
}

void rnaSecStrFreeList(struct rnaSecStr **pList)
/* Free a list of dynamically allocated rnaSecStr's */
{
struct rnaSecStr *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rnaSecStrFree(&el);
    }
*pList = NULL;
}

void rnaSecStrOutput(struct rnaSecStr *el, FILE *f, char sep, char lastSep) 
/* Print out rnaSecStr.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->size);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->secStr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->size; ++i)
    {
    fprintf(f, "%g", el->conf[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

