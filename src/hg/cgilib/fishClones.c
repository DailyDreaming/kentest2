/* fishClones.c was originally generated by the autoSql program, which also 
 * generated fishClones.h and fishClones.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "jksql.h"
#include "fishClones.h"


struct fishClones *fishClonesLoad(char **row)
/* Load a fishClones from row fetched with select * from fishClones
 * from database.  Dispose of this with fishClonesFree(). */
{
struct fishClones *ret;
int sizeOne;

AllocVar(ret);
ret->placeCount = sqlUnsigned(row[5]);
ret->accCount = sqlUnsigned(row[10]);
ret->stsCount = sqlUnsigned(row[12]);
ret->beCount = sqlUnsigned(row[14]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
sqlStringDynamicArray(row[6], &ret->bandStarts, &sizeOne);
assert(sizeOne == ret->placeCount);
sqlStringDynamicArray(row[7], &ret->bandEnds, &sizeOne);
assert(sizeOne == ret->placeCount);
sqlStringDynamicArray(row[8], &ret->labs, &sizeOne);
assert(sizeOne == ret->placeCount);
ret->placeType = cloneString(row[9]);
sqlStringDynamicArray(row[11], &ret->accNames, &sizeOne);
assert(sizeOne == ret->accCount);
sqlStringDynamicArray(row[13], &ret->stsNames, &sizeOne);
assert(sizeOne == ret->stsCount);
sqlStringDynamicArray(row[15], &ret->beNames, &sizeOne);
assert(sizeOne == ret->beCount);
return ret;
}

struct fishClones *fishClonesLoadAll(char *fileName) 
/* Load all fishClones from a tab-separated file.
 * Dispose of this with fishClonesFreeList(). */
{
struct fishClones *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[16];

while (lineFileRow(lf, row))
    {
    el = fishClonesLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct fishClones *fishClonesCommaIn(char **pS, struct fishClones *ret)
/* Create a fishClones out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new fishClones */
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
ret->placeCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->bandStarts, ret->placeCount);
for (i=0; i<ret->placeCount; ++i)
    {
    ret->bandStarts[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->bandEnds, ret->placeCount);
for (i=0; i<ret->placeCount; ++i)
    {
    ret->bandEnds[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->labs, ret->placeCount);
for (i=0; i<ret->placeCount; ++i)
    {
    ret->labs[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->placeType = sqlStringComma(&s);
ret->accCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->accNames, ret->accCount);
for (i=0; i<ret->accCount; ++i)
    {
    ret->accNames[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->stsCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->stsNames, ret->stsCount);
for (i=0; i<ret->stsCount; ++i)
    {
    ret->stsNames[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->beCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->beNames, ret->beCount);
for (i=0; i<ret->beCount; ++i)
    {
    ret->beNames[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void fishClonesFree(struct fishClones **pEl)
/* Free a single dynamically allocated fishClones such as created
 * with fishClonesLoad(). */
{
struct fishClones *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
/* All strings in bandStarts are allocated at once, so only need to free first. */
if (el->bandStarts != NULL)
    freeMem(el->bandStarts[0]);
freeMem(el->bandStarts);
/* All strings in bandEnds are allocated at once, so only need to free first. */
if (el->bandEnds != NULL)
    freeMem(el->bandEnds[0]);
freeMem(el->bandEnds);
/* All strings in labs are allocated at once, so only need to free first. */
if (el->labs != NULL)
    freeMem(el->labs[0]);
freeMem(el->labs);
freeMem(el->placeType);
/* All strings in accNames are allocated at once, so only need to free first. */
if (el->accNames != NULL)
    freeMem(el->accNames[0]);
freeMem(el->accNames);
/* All strings in stsNames are allocated at once, so only need to free first. */
if (el->stsNames != NULL)
    freeMem(el->stsNames[0]);
freeMem(el->stsNames);
/* All strings in beNames are allocated at once, so only need to free first. */
if (el->beNames != NULL)
    freeMem(el->beNames[0]);
freeMem(el->beNames);
freez(pEl);
}

void fishClonesFreeList(struct fishClones **pList)
/* Free a list of dynamically allocated fishClones's */
{
struct fishClones *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    fishClonesFree(&el);
    }
*pList = NULL;
}

void fishClonesOutput(struct fishClones *el, FILE *f, char sep, char lastSep) 
/* Print out fishClones.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->placeCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->placeCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->bandStarts[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->placeCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->bandEnds[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->placeCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->labs[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->placeType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->accCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->accCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->accNames[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->stsCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->stsCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->stsNames[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->beCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->beCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->beNames[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

