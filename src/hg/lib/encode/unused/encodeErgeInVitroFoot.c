/* encodeErgeInVitroFoot.c was originally generated by the autoSql program, which also 
 * generated encodeErgeInVitroFoot.h and encodeErgeInVitroFoot.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2012 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "encode/encodeErgeInVitroFoot.h"


struct encodeErgeInVitroFoot *encodeErgeInVitroFootLoad(char **row)
/* Load a encodeErgeInVitroFoot from row fetched with select * from encodeErgeInVitroFoot
 * from database.  Dispose of this with encodeErgeInVitroFootFree(). */
{
struct encodeErgeInVitroFoot *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->blockCount = sqlUnsigned(row[9]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->thickStart = sqlUnsigned(row[6]);
ret->thickEnd = sqlUnsigned(row[7]);
ret->reserved = sqlUnsigned(row[8]);
sqlUnsignedDynamicArray(row[10], &ret->blockSizes, &sizeOne);
assert(sizeOne == ret->blockCount);
sqlUnsignedDynamicArray(row[11], &ret->chromStarts, &sizeOne);
assert(sizeOne == ret->blockCount);
ret->Id = cloneString(row[12]);
ret->color = cloneString(row[13]);
return ret;
}

struct encodeErgeInVitroFoot *encodeErgeInVitroFootLoadAll(char *fileName) 
/* Load all encodeErgeInVitroFoot from a whitespace-separated file.
 * Dispose of this with encodeErgeInVitroFootFreeList(). */
{
struct encodeErgeInVitroFoot *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileRow(lf, row))
    {
    el = encodeErgeInVitroFootLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeErgeInVitroFoot *encodeErgeInVitroFootLoadAllByChar(char *fileName, char chopper) 
/* Load all encodeErgeInVitroFoot from a chopper separated file.
 * Dispose of this with encodeErgeInVitroFootFreeList(). */
{
struct encodeErgeInVitroFoot *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = encodeErgeInVitroFootLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeErgeInVitroFoot *encodeErgeInVitroFootCommaIn(char **pS, struct encodeErgeInVitroFoot *ret)
/* Create a encodeErgeInVitroFoot out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodeErgeInVitroFoot */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->thickStart = sqlUnsignedComma(&s);
ret->thickEnd = sqlUnsignedComma(&s);
ret->reserved = sqlUnsignedComma(&s);
ret->blockCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->blockSizes, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->blockSizes[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->chromStarts, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->chromStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->Id = sqlStringComma(&s);
ret->color = sqlStringComma(&s);
*pS = s;
return ret;
}

void encodeErgeInVitroFootFree(struct encodeErgeInVitroFoot **pEl)
/* Free a single dynamically allocated encodeErgeInVitroFoot such as created
 * with encodeErgeInVitroFootLoad(). */
{
struct encodeErgeInVitroFoot *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->blockSizes);
freeMem(el->chromStarts);
freeMem(el->Id);
freeMem(el->color);
freez(pEl);
}

void encodeErgeInVitroFootFreeList(struct encodeErgeInVitroFoot **pList)
/* Free a list of dynamically allocated encodeErgeInVitroFoot's */
{
struct encodeErgeInVitroFoot *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    encodeErgeInVitroFootFree(&el);
    }
*pList = NULL;
}

void encodeErgeInVitroFootOutput(struct encodeErgeInVitroFoot *el, FILE *f, char sep, char lastSep) 
/* Print out encodeErgeInVitroFoot.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
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
fprintf(f, "%u", el->thickStart);
fputc(sep,f);
fprintf(f, "%u", el->thickEnd);
fputc(sep,f);
fprintf(f, "%u", el->reserved);
fputc(sep,f);
fprintf(f, "%u", el->blockCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%u", el->blockSizes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%u", el->chromStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->Id);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->color);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

