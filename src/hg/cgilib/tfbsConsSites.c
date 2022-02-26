/* tfbsConsSites.c was originally generated by the autoSql program, which also 
 * generated tfbsConsSites.h and tfbsConsSites.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "tfbsConsSites.h"


void tfbsConsSitesStaticLoad(char **row, struct tfbsConsSites *ret)
/* Load a row from tfbsConsSites table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->zScore = atof(row[6]);
}

struct tfbsConsSites *tfbsConsSitesLoad(char **row)
/* Load a tfbsConsSites from row fetched with select * from tfbsConsSites
 * from database.  Dispose of this with tfbsConsSitesFree(). */
{
struct tfbsConsSites *ret;

AllocVar(ret);
ret->name = cloneString(row[3]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->zScore = atof(row[6]);
return ret;
}

struct tfbsConsSites *tfbsConsSitesLoadAll(char *fileName) 
/* Load all tfbsConsSites from a whitespace-separated file.
 * Dispose of this with tfbsConsSitesFreeList(). */
{
struct tfbsConsSites *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = tfbsConsSitesLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct tfbsConsSites *tfbsConsSitesLoadAllByChar(char *fileName, char chopper) 
/* Load all tfbsConsSites from a chopper separated file.
 * Dispose of this with tfbsConsSitesFreeList(). */
{
struct tfbsConsSites *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = tfbsConsSitesLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct tfbsConsSites *tfbsConsSitesCommaIn(char **pS, struct tfbsConsSites *ret)
/* Create a tfbsConsSites out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new tfbsConsSites */
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
ret->zScore = sqlFloatComma(&s);
*pS = s;
return ret;
}

void tfbsConsSitesFree(struct tfbsConsSites **pEl)
/* Free a single dynamically allocated tfbsConsSites such as created
 * with tfbsConsSitesLoad(). */
{
struct tfbsConsSites *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void tfbsConsSitesFreeList(struct tfbsConsSites **pList)
/* Free a list of dynamically allocated tfbsConsSites's */
{
struct tfbsConsSites *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    tfbsConsSitesFree(&el);
    }
*pList = NULL;
}

void tfbsConsSitesOutput(struct tfbsConsSites *el, FILE *f, char sep, char lastSep) 
/* Print out tfbsConsSites.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%g", el->zScore);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

