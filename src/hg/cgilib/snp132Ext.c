/* snp132Ext.c was originally generated by the autoSql program, which also 
 * generated snp132Ext.h and snp132Ext.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "snp132Ext.h"


struct snp132Ext *snp132ExtLoad(char **row)
/* Load a snp132Ext from row fetched with select * from snp132Ext
 * from database.  Dispose of this with snp132ExtFree(). */
{
struct snp132Ext *ret;

AllocVar(ret);
ret->submitterCount = sqlSigned(row[18]);
ret->alleleFreqCount = sqlSigned(row[20]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
ret->strand = cloneString(row[5]);
ret->refNCBI = cloneString(row[6]);
ret->refUCSC = cloneString(row[7]);
ret->observed = cloneString(row[8]);
ret->molType = cloneString(row[9]);
ret->class = cloneString(row[10]);
ret->valid = cloneString(row[11]);
ret->avHet = sqlFloat(row[12]);
ret->avHetSE = sqlFloat(row[13]);
ret->func = cloneString(row[14]);
ret->locType = cloneString(row[15]);
ret->weight = sqlUnsigned(row[16]);
ret->exceptions = cloneString(row[17]);
{
int sizeOne;
sqlStringDynamicArray(row[19], &ret->submitters, &sizeOne);
assert(sizeOne == ret->submitterCount);
}
{
int sizeOne;
sqlStringDynamicArray(row[21], &ret->alleles, &sizeOne);
assert(sizeOne == ret->alleleFreqCount);
}
{
int sizeOne;
sqlFloatDynamicArray(row[22], &ret->alleleNs, &sizeOne);
assert(sizeOne == ret->alleleFreqCount);
}
{
int sizeOne;
sqlFloatDynamicArray(row[23], &ret->alleleFreqs, &sizeOne);
assert(sizeOne == ret->alleleFreqCount);
}
ret->bitfields = cloneString(row[24]);
return ret;
}

struct snp132Ext *snp132ExtLoadAll(char *fileName) 
/* Load all snp132Ext from a whitespace-separated file.
 * Dispose of this with snp132ExtFreeList(). */
{
struct snp132Ext *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[25];

while (lineFileRow(lf, row))
    {
    el = snp132ExtLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snp132Ext *snp132ExtLoadAllByChar(char *fileName, char chopper) 
/* Load all snp132Ext from a chopper separated file.
 * Dispose of this with snp132ExtFreeList(). */
{
struct snp132Ext *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[25];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = snp132ExtLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snp132Ext *snp132ExtCommaIn(char **pS, struct snp132Ext *ret)
/* Create a snp132Ext out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snp132Ext */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
ret->strand = sqlStringComma(&s);
ret->refNCBI = sqlStringComma(&s);
ret->refUCSC = sqlStringComma(&s);
ret->observed = sqlStringComma(&s);
ret->molType = sqlStringComma(&s);
ret->class = sqlStringComma(&s);
ret->valid = sqlStringComma(&s);
ret->avHet = sqlFloatComma(&s);
ret->avHetSE = sqlFloatComma(&s);
ret->func = sqlStringComma(&s);
ret->locType = sqlStringComma(&s);
ret->weight = sqlUnsignedComma(&s);
ret->exceptions = sqlStringComma(&s);
ret->submitterCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->submitters, ret->submitterCount);
for (i=0; i<ret->submitterCount; ++i)
    {
    ret->submitters[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->alleleFreqCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->alleles, ret->alleleFreqCount);
for (i=0; i<ret->alleleFreqCount; ++i)
    {
    ret->alleles[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->alleleNs, ret->alleleFreqCount);
for (i=0; i<ret->alleleFreqCount; ++i)
    {
    ret->alleleNs[i] = sqlFloatComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->alleleFreqs, ret->alleleFreqCount);
for (i=0; i<ret->alleleFreqCount; ++i)
    {
    ret->alleleFreqs[i] = sqlFloatComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->bitfields = sqlStringComma(&s);
*pS = s;
return ret;
}

void snp132ExtFree(struct snp132Ext **pEl)
/* Free a single dynamically allocated snp132Ext such as created
 * with snp132ExtLoad(). */
{
struct snp132Ext *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->strand);
freeMem(el->refNCBI);
freeMem(el->refUCSC);
freeMem(el->observed);
freeMem(el->molType);
freeMem(el->class);
freeMem(el->valid);
freeMem(el->func);
freeMem(el->locType);
freeMem(el->exceptions);
/* All strings in submitters are allocated at once, so only need to free first. */
if (el->submitters != NULL)
    freeMem(el->submitters[0]);
freeMem(el->submitters);
/* All strings in alleles are allocated at once, so only need to free first. */
if (el->alleles != NULL)
    freeMem(el->alleles[0]);
freeMem(el->alleles);
freeMem(el->alleleNs);
freeMem(el->alleleFreqs);
freeMem(el->bitfields);
freez(pEl);
}

void snp132ExtFreeList(struct snp132Ext **pList)
/* Free a list of dynamically allocated snp132Ext's */
{
struct snp132Ext *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    snp132ExtFree(&el);
    }
*pList = NULL;
}

void snp132ExtOutput(struct snp132Ext *el, FILE *f, char sep, char lastSep) 
/* Print out snp132Ext.  Separate fields with sep. Follow last field with lastSep. */
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
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->refNCBI);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->refUCSC);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->observed);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->molType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->class);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->valid);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%g", el->avHet);
fputc(sep,f);
fprintf(f, "%g", el->avHetSE);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->func);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->locType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->weight);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->exceptions);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->submitterCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->submitterCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->submitters[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
fprintf(f, "%d", el->alleleFreqCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->alleleFreqCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->alleles[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->alleleFreqCount; ++i)
    {
    fprintf(f, "%g", el->alleleNs[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->alleleFreqCount; ++i)
    {
    fprintf(f, "%g", el->alleleFreqs[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->bitfields);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

