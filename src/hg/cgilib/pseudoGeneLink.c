/* pseudoGeneLink.c was originally generated by the autoSql program, which also 
 * generated pseudoGeneLink.h and pseudoGeneLink.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "pseudoGeneLink.h"


struct pseudoGeneLink *pseudoGeneLinkLoad(char **row)
/* Load a pseudoGeneLink from row fetched with select * from pseudoGeneLink
 * from database.  Dispose of this with pseudoGeneLinkFree(). */
{
struct pseudoGeneLink *ret;

AllocVar(ret);
ret->blockCount = sqlSigned(row[9]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->thickStart = sqlUnsigned(row[6]);
ret->thickEnd = sqlUnsigned(row[7]);
ret->reserved = sqlUnsigned(row[8]);
{
int sizeOne;
sqlSignedDynamicArray(row[10], &ret->blockSizes, &sizeOne);
assert(sizeOne == ret->blockCount);
}
{
int sizeOne;
sqlSignedDynamicArray(row[11], &ret->chromStarts, &sizeOne);
assert(sizeOne == ret->blockCount);
}
ret->trfRatio = atof(row[12]);
ret->type = cloneString(row[13]);
ret->axtScore = sqlSigned(row[14]);
ret->gChrom = cloneString(row[15]);
ret->gStart = sqlSigned(row[16]);
ret->gEnd = sqlSigned(row[17]);
strcpy(ret->gStrand, row[18]);
ret->exonCount = sqlUnsigned(row[19]);
ret->geneOverlap = sqlUnsigned(row[20]);
ret->polyA = sqlUnsigned(row[21]);
ret->polyAstart = sqlSigned(row[22]);
ret->exonCover = sqlSigned(row[23]);
ret->intronCount = sqlUnsigned(row[24]);
ret->bestAliCount = sqlUnsigned(row[25]);
ret->matches = sqlUnsigned(row[26]);
ret->qSize = sqlUnsigned(row[27]);
ret->qEnd = sqlUnsigned(row[28]);
ret->tReps = sqlUnsigned(row[29]);
ret->overlapRhesus = sqlSigned(row[30]);
ret->overlapMouse = sqlSigned(row[31]);
ret->coverage = sqlUnsigned(row[32]);
ret->label = sqlSigned(row[33]);
ret->milliBad = sqlUnsigned(row[34]);
ret->oldScore = sqlUnsigned(row[35]);
ret->oldIntronCount = sqlSigned(row[36]);
ret->processedIntrons = sqlSigned(row[37]);
ret->conservedSpliceSites = sqlSigned(row[38]);
ret->maxOverlap = sqlSigned(row[39]);
ret->refSeq = cloneString(row[40]);
ret->rStart = sqlSigned(row[41]);
ret->rEnd = sqlSigned(row[42]);
ret->mgc = cloneString(row[43]);
ret->mStart = sqlSigned(row[44]);
ret->mEnd = sqlSigned(row[45]);
ret->kgName = cloneString(row[46]);
ret->kStart = sqlSigned(row[47]);
ret->kEnd = sqlSigned(row[48]);
ret->overName = cloneString(row[49]);
ret->overStart = sqlSigned(row[50]);
ret->overEnd = sqlSigned(row[51]);
strcpy(ret->overStrand, row[52]);
ret->overlapDog = sqlSigned(row[53]);
ret->posConf = atof(row[54]);
ret->polyAlen = sqlUnsigned(row[55]);
return ret;
}

struct pseudoGeneLink *pseudoGeneLinkLoadAll(char *fileName) 
/* Load all pseudoGeneLink from a whitespace-separated file.
 * Dispose of this with pseudoGeneLinkFreeList(). */
{
struct pseudoGeneLink *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[56];

while (lineFileRow(lf, row))
    {
    el = pseudoGeneLinkLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pseudoGeneLink *pseudoGeneLinkLoadAllByChar(char *fileName, char chopper) 
/* Load all pseudoGeneLink from a chopper separated file.
 * Dispose of this with pseudoGeneLinkFreeList(). */
{
struct pseudoGeneLink *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[56];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = pseudoGeneLinkLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pseudoGeneLink *pseudoGeneLinkCommaIn(char **pS, struct pseudoGeneLink *ret)
/* Create a pseudoGeneLink out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pseudoGeneLink */
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
ret->thickStart = sqlUnsignedComma(&s);
ret->thickEnd = sqlUnsignedComma(&s);
ret->reserved = sqlUnsignedComma(&s);
ret->blockCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->blockSizes, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->blockSizes[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->chromStarts, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->chromStarts[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->trfRatio = sqlFloatComma(&s);
ret->type = sqlStringComma(&s);
ret->axtScore = sqlSignedComma(&s);
ret->gChrom = sqlStringComma(&s);
ret->gStart = sqlSignedComma(&s);
ret->gEnd = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->gStrand, sizeof(ret->gStrand));
ret->exonCount = sqlUnsignedComma(&s);
ret->geneOverlap = sqlUnsignedComma(&s);
ret->polyA = sqlUnsignedComma(&s);
ret->polyAstart = sqlSignedComma(&s);
ret->exonCover = sqlSignedComma(&s);
ret->intronCount = sqlUnsignedComma(&s);
ret->bestAliCount = sqlUnsignedComma(&s);
ret->matches = sqlUnsignedComma(&s);
ret->qSize = sqlUnsignedComma(&s);
ret->qEnd = sqlUnsignedComma(&s);
ret->tReps = sqlUnsignedComma(&s);
ret->overlapRhesus = sqlSignedComma(&s);
ret->overlapMouse = sqlSignedComma(&s);
ret->coverage = sqlUnsignedComma(&s);
ret->label = sqlSignedComma(&s);
ret->milliBad = sqlUnsignedComma(&s);
ret->oldScore = sqlUnsignedComma(&s);
ret->oldIntronCount = sqlSignedComma(&s);
ret->processedIntrons = sqlSignedComma(&s);
ret->conservedSpliceSites = sqlSignedComma(&s);
ret->maxOverlap = sqlSignedComma(&s);
ret->refSeq = sqlStringComma(&s);
ret->rStart = sqlSignedComma(&s);
ret->rEnd = sqlSignedComma(&s);
ret->mgc = sqlStringComma(&s);
ret->mStart = sqlSignedComma(&s);
ret->mEnd = sqlSignedComma(&s);
ret->kgName = sqlStringComma(&s);
ret->kStart = sqlSignedComma(&s);
ret->kEnd = sqlSignedComma(&s);
ret->overName = sqlStringComma(&s);
ret->overStart = sqlSignedComma(&s);
ret->overEnd = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->overStrand, sizeof(ret->overStrand));
ret->overlapDog = sqlSignedComma(&s);
ret->posConf = sqlFloatComma(&s);
ret->polyAlen = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void pseudoGeneLinkFree(struct pseudoGeneLink **pEl)
/* Free a single dynamically allocated pseudoGeneLink such as created
 * with pseudoGeneLinkLoad(). */
{
struct pseudoGeneLink *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->blockSizes);
freeMem(el->chromStarts);
freeMem(el->type);
freeMem(el->gChrom);
freeMem(el->refSeq);
freeMem(el->mgc);
freeMem(el->kgName);
freeMem(el->overName);
freez(pEl);
}

void pseudoGeneLinkFreeList(struct pseudoGeneLink **pList)
/* Free a list of dynamically allocated pseudoGeneLink's */
{
struct pseudoGeneLink *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pseudoGeneLinkFree(&el);
    }
*pList = NULL;
}

void pseudoGeneLinkOutput(struct pseudoGeneLink *el, FILE *f, char sep, char lastSep) 
/* Print out pseudoGeneLink.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->thickStart);
fputc(sep,f);
fprintf(f, "%u", el->thickEnd);
fputc(sep,f);
fprintf(f, "%u", el->reserved);
fputc(sep,f);
fprintf(f, "%d", el->blockCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%d", el->blockSizes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%d", el->chromStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
fprintf(f, "%g", el->trfRatio);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->axtScore);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->gStart);
fputc(sep,f);
fprintf(f, "%d", el->gEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->exonCount);
fputc(sep,f);
fprintf(f, "%u", el->geneOverlap);
fputc(sep,f);
fprintf(f, "%u", el->polyA);
fputc(sep,f);
fprintf(f, "%d", el->polyAstart);
fputc(sep,f);
fprintf(f, "%d", el->exonCover);
fputc(sep,f);
fprintf(f, "%u", el->intronCount);
fputc(sep,f);
fprintf(f, "%u", el->bestAliCount);
fputc(sep,f);
fprintf(f, "%u", el->matches);
fputc(sep,f);
fprintf(f, "%u", el->qSize);
fputc(sep,f);
fprintf(f, "%u", el->qEnd);
fputc(sep,f);
fprintf(f, "%u", el->tReps);
fputc(sep,f);
fprintf(f, "%d", el->overlapRhesus);
fputc(sep,f);
fprintf(f, "%d", el->overlapMouse);
fputc(sep,f);
fprintf(f, "%u", el->coverage);
fputc(sep,f);
fprintf(f, "%d", el->label);
fputc(sep,f);
fprintf(f, "%u", el->milliBad);
fputc(sep,f);
fprintf(f, "%u", el->oldScore);
fputc(sep,f);
fprintf(f, "%d", el->oldIntronCount);
fputc(sep,f);
fprintf(f, "%d", el->processedIntrons);
fputc(sep,f);
fprintf(f, "%d", el->conservedSpliceSites);
fputc(sep,f);
fprintf(f, "%d", el->maxOverlap);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->refSeq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->rStart);
fputc(sep,f);
fprintf(f, "%d", el->rEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mgc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->mStart);
fputc(sep,f);
fprintf(f, "%d", el->mEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->kgName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->kStart);
fputc(sep,f);
fprintf(f, "%d", el->kEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->overName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->overStart);
fputc(sep,f);
fprintf(f, "%d", el->overEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->overStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->overlapDog);
fputc(sep,f);
fprintf(f, "%g", el->posConf);
fputc(sep,f);
fprintf(f, "%u", el->polyAlen);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

