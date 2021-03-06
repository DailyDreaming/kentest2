/* mrnaMisMatch.c was originally generated by the autoSql program, which also 
 * generated mrnaMisMatch.h and mrnaMisMatch.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "mrnaMisMatch.h"


struct mrnaMisMatch *mrnaMisMatchLoad(char **row)
/* Load a mrnaMisMatch from row fetched with select * from mrnaMisMatch
 * from database.  Dispose of this with mrnaMisMatchFree(). */
{
struct mrnaMisMatch *ret;

AllocVar(ret);
ret->misMatchCount = sqlSigned(row[3]);
ret->snpCount = sqlSigned(row[9]);
ret->name = cloneString(row[0]);
ret->mrnaBase = row[1][0];
ret->mrnaLoc = sqlSigned(row[2]);
ret->bases = cloneString(row[4]);
{
int sizeOne;
sqlStringDynamicArray(row[5], &ret->chroms, &sizeOne);
assert(sizeOne == ret->misMatchCount);
}
{
int sizeOne;
sqlUnsignedDynamicArray(row[6], &ret->tStarts, &sizeOne);
assert(sizeOne == ret->misMatchCount);
}
ret->strands = cloneString(row[7]);
{
int sizeOne;
sqlUnsignedDynamicArray(row[8], &ret->loci, &sizeOne);
assert(sizeOne == ret->misMatchCount);
}
{
int sizeOne;
sqlStringDynamicArray(row[10], &ret->snps, &sizeOne);
assert(sizeOne == ret->snpCount);
}
return ret;
}

struct mrnaMisMatch *mrnaMisMatchLoadAll(char *fileName) 
/* Load all mrnaMisMatch from a whitespace-separated file.
 * Dispose of this with mrnaMisMatchFreeList(). */
{
struct mrnaMisMatch *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileRow(lf, row))
    {
    el = mrnaMisMatchLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct mrnaMisMatch *mrnaMisMatchLoadAllByChar(char *fileName, char chopper) 
/* Load all mrnaMisMatch from a chopper separated file.
 * Dispose of this with mrnaMisMatchFreeList(). */
{
struct mrnaMisMatch *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = mrnaMisMatchLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct mrnaMisMatch *mrnaMisMatchCommaIn(char **pS, struct mrnaMisMatch *ret)
/* Create a mrnaMisMatch out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new mrnaMisMatch */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
sqlFixedStringComma(&s, &(ret->mrnaBase), sizeof(ret->mrnaBase));
ret->mrnaLoc = sqlSignedComma(&s);
ret->misMatchCount = sqlSignedComma(&s);
ret->bases = sqlStringComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->chroms, ret->misMatchCount);
for (i=0; i<ret->misMatchCount; ++i)
    {
    ret->chroms[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->tStarts, ret->misMatchCount);
for (i=0; i<ret->misMatchCount; ++i)
    {
    ret->tStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->strands = sqlStringComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->loci, ret->misMatchCount);
for (i=0; i<ret->misMatchCount; ++i)
    {
    ret->loci[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->snpCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->snps, ret->snpCount);
for (i=0; i<ret->snpCount; ++i)
    {
    ret->snps[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void mrnaMisMatchFree(struct mrnaMisMatch **pEl)
/* Free a single dynamically allocated mrnaMisMatch such as created
 * with mrnaMisMatchLoad(). */
{
struct mrnaMisMatch *el;
int i;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->bases);
for (i = 0 ; i < el->misMatchCount; i++)
    if (el->chroms[i] != NULL)
        freeMem(el->chroms[i]);
freeMem(el->chroms);
freeMem(el->tStarts);
freeMem(el->strands);
freeMem(el->loci);
for (i = 0 ; i < el->misMatchCount; i++)
    if (el->snps[i] != NULL)
        freeMem(el->snps[i]);
freeMem(el->snps);
freez(pEl);
}

void mrnaMisMatchFreeList(struct mrnaMisMatch **pList)
/* Free a list of dynamically allocated mrnaMisMatch's */
{
struct mrnaMisMatch *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    mrnaMisMatchFree(&el);
    }
*pList = NULL;
}

void mrnaMisMatchOutput(struct mrnaMisMatch *el, FILE *f, char sep, char lastSep) 
/* Print out mrnaMisMatch.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->mrnaBase);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->mrnaLoc);
fputc(sep,f);
fprintf(f, "%d", el->misMatchCount);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->bases);
if (sep == ',') fputc('"',f);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->misMatchCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->chroms[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->misMatchCount; ++i)
    {
    fprintf(f, "%u", el->tStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strands);
if (sep == ',') fputc('"',f);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->misMatchCount; ++i)
    {
    fprintf(f, "%u", el->loci[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
fprintf(f, "%d", el->snpCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->misMatchCount; i++)
    {
    if (sep == ',') fputc('"',f);
//        printf("bad snp %s i=%d cnt=%d\n",el->name, i, el->snpCount);
//    assert(el->snps[i] != NULL);
    if(el->snps[i] != NULL)
        {
        fprintf(f, "%s", el->snps[i]);
        }
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

