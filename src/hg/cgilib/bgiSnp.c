/* bgiSnp.c was originally generated by the autoSql program, which also 
 * generated bgiSnp.h and bgiSnp.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "bgiSnp.h"


void bgiSnpStaticLoad(char **row, struct bgiSnp *ret)
/* Load a row from bgiSnp table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
strcpy(ret->snpType, row[4]);
ret->readStart = sqlUnsigned(row[5]);
ret->readEnd = sqlUnsigned(row[6]);
ret->qualChr = sqlUnsigned(row[7]);
ret->qualReads = sqlUnsigned(row[8]);
ret->snpSeq = row[9];
ret->readName = row[10];
strcpy(ret->readDir, row[11]);
strcpy(ret->inBroiler, row[12]);
strcpy(ret->inLayer, row[13]);
strcpy(ret->inSilkie, row[14]);
ret->primerL = row[15];
ret->primerR = row[16];
strcpy(ret->questionM, row[17]);
ret->extra = row[18];
}

struct bgiSnp *bgiSnpLoad(char **row)
/* Load a bgiSnp from row fetched with select * from bgiSnp
 * from database.  Dispose of this with bgiSnpFree(). */
{
struct bgiSnp *ret;
AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
strcpy(ret->snpType, row[4]);
ret->readStart = sqlUnsigned(row[5]);
ret->readEnd = sqlUnsigned(row[6]);
ret->qualChr = sqlUnsigned(row[7]);
ret->qualReads = sqlUnsigned(row[8]);
ret->snpSeq = cloneString(row[9]);
ret->readName = cloneString(row[10]);
strcpy(ret->readDir, row[11]);
strcpy(ret->inBroiler, row[12]);
strcpy(ret->inLayer, row[13]);
strcpy(ret->inSilkie, row[14]);
ret->primerL = cloneString(row[15]);
ret->primerR = cloneString(row[16]);
strcpy(ret->questionM, row[17]);
ret->extra = cloneString(row[18]);
return ret;
}

struct bgiSnp *bgiSnpLoadAll(char *fileName) 
/* Load all bgiSnp from a whitespace-separated file.
 * Dispose of this with bgiSnpFreeList(). */
{
struct bgiSnp *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[19];

while (lineFileRow(lf, row))
    {
    el = bgiSnpLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bgiSnp *bgiSnpLoadAllByChar(char *fileName, char chopper) 
/* Load all bgiSnp from a chopper separated file.
 * Dispose of this with bgiSnpFreeList(). */
{
struct bgiSnp *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[19];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = bgiSnpLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bgiSnp *bgiSnpCommaIn(char **pS, struct bgiSnp *ret)
/* Create a bgiSnp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bgiSnp */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
sqlFixedStringComma(&s, ret->snpType, sizeof(ret->snpType));
ret->readStart = sqlUnsignedComma(&s);
ret->readEnd = sqlUnsignedComma(&s);
ret->qualChr = sqlUnsignedComma(&s);
ret->qualReads = sqlUnsignedComma(&s);
ret->snpSeq = sqlStringComma(&s);
ret->readName = sqlStringComma(&s);
sqlFixedStringComma(&s, ret->readDir, sizeof(ret->readDir));
sqlFixedStringComma(&s, ret->inBroiler, sizeof(ret->inBroiler));
sqlFixedStringComma(&s, ret->inLayer, sizeof(ret->inLayer));
sqlFixedStringComma(&s, ret->inSilkie, sizeof(ret->inSilkie));
ret->primerL = sqlStringComma(&s);
ret->primerR = sqlStringComma(&s);
sqlFixedStringComma(&s, ret->questionM, sizeof(ret->questionM));
ret->extra = sqlStringComma(&s);
*pS = s;
return ret;
}

void bgiSnpFree(struct bgiSnp **pEl)
/* Free a single dynamically allocated bgiSnp such as created
 * with bgiSnpLoad(). */
{
struct bgiSnp *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->snpSeq);
freeMem(el->readName);
freeMem(el->primerL);
freeMem(el->primerR);
freeMem(el->extra);
freez(pEl);
}

void bgiSnpFreeList(struct bgiSnp **pList)
/* Free a list of dynamically allocated bgiSnp's */
{
struct bgiSnp *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bgiSnpFree(&el);
    }
*pList = NULL;
}

void bgiSnpOutput(struct bgiSnp *el, FILE *f, char sep, char lastSep) 
/* Print out bgiSnp.  Separate fields with sep. Follow last field with lastSep. */
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
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->snpType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->readStart);
fputc(sep,f);
fprintf(f, "%u", el->readEnd);
fputc(sep,f);
fprintf(f, "%u", el->qualChr);
fputc(sep,f);
fprintf(f, "%u", el->qualReads);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->snpSeq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->readName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->readDir);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->inBroiler);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->inLayer);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->inSilkie);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->primerL);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->primerR);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->questionM);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->extra);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

