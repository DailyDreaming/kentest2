/* rmskAlign.c was originally generated by the autoSql program, which also 
 * generated rmskAlign.h and rmskAlign.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rmskAlign.h"


void rmskAlignStaticLoad(char **row, struct rmskAlign *ret)
/* Load a row from rmskAlign table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->swScore = sqlUnsigned(row[0]);
ret->milliDiv = sqlUnsigned(row[1]);
ret->milliDel = sqlUnsigned(row[2]);
ret->milliIns = sqlUnsigned(row[3]);
ret->genoName = row[4];
ret->genoStart = sqlUnsigned(row[5]);
ret->genoEnd = sqlUnsigned(row[6]);
ret->genoLeft = sqlSigned(row[7]);
safecpy(ret->strand, sizeof(ret->strand), row[8]);
ret->repName = row[9];
ret->repClass = row[10];
ret->repFamily = row[11];
ret->repStart = sqlSigned(row[12]);
ret->repEnd = sqlUnsigned(row[13]);
ret->repLeft = sqlSigned(row[14]);
ret->id = sqlUnsigned(row[15]);
ret->alignment = row[16];
}

struct rmskAlign *rmskAlignLoad(char **row)
/* Load a rmskAlign from row fetched with select * from rmskAlign
 * from database.  Dispose of this with rmskAlignFree(). */
{
struct rmskAlign *ret;

AllocVar(ret);
ret->swScore = sqlUnsigned(row[0]);
ret->milliDiv = sqlUnsigned(row[1]);
ret->milliDel = sqlUnsigned(row[2]);
ret->milliIns = sqlUnsigned(row[3]);
ret->genoName = cloneString(row[4]);
ret->genoStart = sqlUnsigned(row[5]);
ret->genoEnd = sqlUnsigned(row[6]);
ret->genoLeft = sqlSigned(row[7]);
safecpy(ret->strand, sizeof(ret->strand), row[8]);
ret->repName = cloneString(row[9]);
ret->repClass = cloneString(row[10]);
ret->repFamily = cloneString(row[11]);
ret->repStart = sqlSigned(row[12]);
ret->repEnd = sqlUnsigned(row[13]);
ret->repLeft = sqlSigned(row[14]);
ret->id = sqlUnsigned(row[15]);
ret->alignment = cloneString(row[16]);
return ret;
}

struct rmskAlign *rmskAlignLoadAll(char *fileName) 
/* Load all rmskAlign from a whitespace-separated file.
 * Dispose of this with rmskAlignFreeList(). */
{
struct rmskAlign *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileRow(lf, row))
    {
    el = rmskAlignLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rmskAlign *rmskAlignLoadAllByChar(char *fileName, char chopper) 
/* Load all rmskAlign from a chopper separated file.
 * Dispose of this with rmskAlignFreeList(). */
{
struct rmskAlign *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = rmskAlignLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rmskAlign *rmskAlignCommaIn(char **pS, struct rmskAlign *ret)
/* Create a rmskAlign out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rmskAlign */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->swScore = sqlUnsignedComma(&s);
ret->milliDiv = sqlUnsignedComma(&s);
ret->milliDel = sqlUnsignedComma(&s);
ret->milliIns = sqlUnsignedComma(&s);
ret->genoName = sqlStringComma(&s);
ret->genoStart = sqlUnsignedComma(&s);
ret->genoEnd = sqlUnsignedComma(&s);
ret->genoLeft = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->repName = sqlStringComma(&s);
ret->repClass = sqlStringComma(&s);
ret->repFamily = sqlStringComma(&s);
ret->repStart = sqlSignedComma(&s);
ret->repEnd = sqlUnsignedComma(&s);
ret->repLeft = sqlSignedComma(&s);
ret->id = sqlUnsignedComma(&s);
ret->alignment = sqlStringComma(&s);
*pS = s;
return ret;
}

void rmskAlignFree(struct rmskAlign **pEl)
/* Free a single dynamically allocated rmskAlign such as created
 * with rmskAlignLoad(). */
{
struct rmskAlign *el;

if ((el = *pEl) == NULL) return;
freeMem(el->genoName);
freeMem(el->repName);
freeMem(el->repClass);
freeMem(el->repFamily);
freeMem(el->alignment);
freez(pEl);
}

void rmskAlignFreeList(struct rmskAlign **pList)
/* Free a list of dynamically allocated rmskAlign's */
{
struct rmskAlign *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rmskAlignFree(&el);
    }
*pList = NULL;
}

void rmskAlignOutput(struct rmskAlign *el, FILE *f, char sep, char lastSep) 
/* Print out rmskAlign.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%u", el->swScore);
fputc(sep,f);
fprintf(f, "%u", el->milliDiv);
fputc(sep,f);
fprintf(f, "%u", el->milliDel);
fputc(sep,f);
fprintf(f, "%u", el->milliIns);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genoName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->genoStart);
fputc(sep,f);
fprintf(f, "%u", el->genoEnd);
fputc(sep,f);
fprintf(f, "%d", el->genoLeft);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repClass);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repFamily);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->repStart);
fputc(sep,f);
fprintf(f, "%u", el->repEnd);
fputc(sep,f);
fprintf(f, "%d", el->repLeft);
fputc(sep,f);
fprintf(f, "%u", el->id);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->alignment);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

