/* txEdgeOrtho.c was originally generated by the autoSql program, which also 
 * generated txEdgeOrtho.h and txEdgeOrtho.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "txEdgeOrtho.h"


/* definitions for type column */
static char *values_type[] = {"exon", "intron", NULL};
static struct hash *valhash_type = NULL;

void txEdgeOrthoStaticLoad(char **row, struct txEdgeOrtho *ret)
/* Load a row from txEdgeOrtho table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlSigned(row[1]);
ret->chromEnd = sqlSigned(row[2]);
ret->name = row[3];
ret->score = sqlSigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
safecpy(ret->startType, sizeof(ret->startType), row[6]);
ret->type = sqlEnumParse(row[7], values_type, &valhash_type);
safecpy(ret->endType, sizeof(ret->endType), row[8]);
ret->mappedChrom = row[9];
ret->mappedStart = sqlSigned(row[10]);
ret->mappedEnd = sqlSigned(row[11]);
ret->txGraph = row[12];
ret->overlapScore = sqlSigned(row[13]);
safecpy(ret->orthoStrand, sizeof(ret->orthoStrand), row[14]);
safecpy(ret->orthoStartType, sizeof(ret->orthoStartType), row[15]);
safecpy(ret->orthoEndType, sizeof(ret->orthoEndType), row[16]);
ret->orthoStart = sqlSigned(row[17]);
ret->orthoEnd = sqlSigned(row[18]);
}

struct txEdgeOrtho *txEdgeOrthoLoad(char **row)
/* Load a txEdgeOrtho from row fetched with select * from txEdgeOrtho
 * from database.  Dispose of this with txEdgeOrthoFree(). */
{
struct txEdgeOrtho *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlSigned(row[1]);
ret->chromEnd = sqlSigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlSigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
safecpy(ret->startType, sizeof(ret->startType), row[6]);
ret->type = sqlEnumParse(row[7], values_type, &valhash_type);
safecpy(ret->endType, sizeof(ret->endType), row[8]);
ret->mappedChrom = cloneString(row[9]);
ret->mappedStart = sqlSigned(row[10]);
ret->mappedEnd = sqlSigned(row[11]);
ret->txGraph = cloneString(row[12]);
ret->overlapScore = sqlSigned(row[13]);
safecpy(ret->orthoStrand, sizeof(ret->orthoStrand), row[14]);
safecpy(ret->orthoStartType, sizeof(ret->orthoStartType), row[15]);
safecpy(ret->orthoEndType, sizeof(ret->orthoEndType), row[16]);
ret->orthoStart = sqlSigned(row[17]);
ret->orthoEnd = sqlSigned(row[18]);
return ret;
}

struct txEdgeOrtho *txEdgeOrthoLoadAll(char *fileName) 
/* Load all txEdgeOrtho from a whitespace-separated file.
 * Dispose of this with txEdgeOrthoFreeList(). */
{
struct txEdgeOrtho *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[19];

while (lineFileRow(lf, row))
    {
    el = txEdgeOrthoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct txEdgeOrtho *txEdgeOrthoLoadAllByChar(char *fileName, char chopper) 
/* Load all txEdgeOrtho from a chopper separated file.
 * Dispose of this with txEdgeOrthoFreeList(). */
{
struct txEdgeOrtho *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[19];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = txEdgeOrthoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct txEdgeOrtho *txEdgeOrthoCommaIn(char **pS, struct txEdgeOrtho *ret)
/* Create a txEdgeOrtho out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new txEdgeOrtho */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlSignedComma(&s);
ret->chromEnd = sqlSignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
sqlFixedStringComma(&s, ret->startType, sizeof(ret->startType));
ret->type = sqlEnumComma(&s, values_type, &valhash_type);
sqlFixedStringComma(&s, ret->endType, sizeof(ret->endType));
ret->mappedChrom = sqlStringComma(&s);
ret->mappedStart = sqlSignedComma(&s);
ret->mappedEnd = sqlSignedComma(&s);
ret->txGraph = sqlStringComma(&s);
ret->overlapScore = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->orthoStrand, sizeof(ret->orthoStrand));
sqlFixedStringComma(&s, ret->orthoStartType, sizeof(ret->orthoStartType));
sqlFixedStringComma(&s, ret->orthoEndType, sizeof(ret->orthoEndType));
ret->orthoStart = sqlSignedComma(&s);
ret->orthoEnd = sqlSignedComma(&s);
*pS = s;
return ret;
}

void txEdgeOrthoFree(struct txEdgeOrtho **pEl)
/* Free a single dynamically allocated txEdgeOrtho such as created
 * with txEdgeOrthoLoad(). */
{
struct txEdgeOrtho *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->mappedChrom);
freeMem(el->txGraph);
freez(pEl);
}

void txEdgeOrthoFreeList(struct txEdgeOrtho **pList)
/* Free a list of dynamically allocated txEdgeOrtho's */
{
struct txEdgeOrtho *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    txEdgeOrthoFree(&el);
    }
*pList = NULL;
}

void txEdgeOrthoOutput(struct txEdgeOrtho *el, FILE *f, char sep, char lastSep) 
/* Print out txEdgeOrtho.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->chromStart);
fputc(sep,f);
fprintf(f, "%d", el->chromEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->startType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
sqlEnumPrint(f, el->type, values_type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->endType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mappedChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->mappedStart);
fputc(sep,f);
fprintf(f, "%d", el->mappedEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->txGraph);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->overlapScore);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->orthoStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->orthoStartType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->orthoEndType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->orthoStart);
fputc(sep,f);
fprintf(f, "%d", el->orthoEnd);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

