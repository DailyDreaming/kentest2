/* rhMapInfo.c was originally generated by the autoSql program, which also 
 * generated rhMapInfo.h and rhMapInfo.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rhMapInfo.h"


void rhMapInfoStaticLoad(char **row, struct rhMapInfo *ret)
/* Load a row from rhMapInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->zfinId = row[1];
ret->linkageGp = row[2];
ret->position = sqlUnsigned(row[3]);
ret->distance = sqlUnsigned(row[4]);
ret->markerType = row[5];
ret->source = row[6];
ret->mapSite = row[7];
ret->leftPrimer = row[8];
ret->rightPrimer = row[9];
}

struct rhMapInfo *rhMapInfoLoad(char **row)
/* Load a rhMapInfo from row fetched with select * from rhMapInfo
 * from database.  Dispose of this with rhMapInfoFree(). */
{
struct rhMapInfo *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->zfinId = cloneString(row[1]);
ret->linkageGp = cloneString(row[2]);
ret->position = sqlUnsigned(row[3]);
ret->distance = sqlUnsigned(row[4]);
ret->markerType = cloneString(row[5]);
ret->source = cloneString(row[6]);
ret->mapSite = cloneString(row[7]);
ret->leftPrimer = cloneString(row[8]);
ret->rightPrimer = cloneString(row[9]);
return ret;
}

struct rhMapInfo *rhMapInfoLoadAll(char *fileName) 
/* Load all rhMapInfo from a whitespace-separated file.
 * Dispose of this with rhMapInfoFreeList(). */
{
struct rhMapInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[10];

while (lineFileRow(lf, row))
    {
    el = rhMapInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rhMapInfo *rhMapInfoLoadAllByChar(char *fileName, char chopper) 
/* Load all rhMapInfo from a chopper separated file.
 * Dispose of this with rhMapInfoFreeList(). */
{
struct rhMapInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[10];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = rhMapInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rhMapInfo *rhMapInfoCommaIn(char **pS, struct rhMapInfo *ret)
/* Create a rhMapInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rhMapInfo */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->zfinId = sqlStringComma(&s);
ret->linkageGp = sqlStringComma(&s);
ret->position = sqlUnsignedComma(&s);
ret->distance = sqlUnsignedComma(&s);
ret->markerType = sqlStringComma(&s);
ret->source = sqlStringComma(&s);
ret->mapSite = sqlStringComma(&s);
ret->leftPrimer = sqlStringComma(&s);
ret->rightPrimer = sqlStringComma(&s);
*pS = s;
return ret;
}

void rhMapInfoFree(struct rhMapInfo **pEl)
/* Free a single dynamically allocated rhMapInfo such as created
 * with rhMapInfoLoad(). */
{
struct rhMapInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->zfinId);
freeMem(el->linkageGp);
freeMem(el->markerType);
freeMem(el->source);
freeMem(el->mapSite);
freeMem(el->leftPrimer);
freeMem(el->rightPrimer);
freez(pEl);
}

void rhMapInfoFreeList(struct rhMapInfo **pList)
/* Free a list of dynamically allocated rhMapInfo's */
{
struct rhMapInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rhMapInfoFree(&el);
    }
*pList = NULL;
}

void rhMapInfoOutput(struct rhMapInfo *el, FILE *f, char sep, char lastSep) 
/* Print out rhMapInfo.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->zfinId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->linkageGp);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->position);
fputc(sep,f);
fprintf(f, "%u", el->distance);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->markerType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->source);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mapSite);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->leftPrimer);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rightPrimer);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

