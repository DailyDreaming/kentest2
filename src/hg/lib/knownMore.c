/* knownMore.c was originally generated by the autoSql program, which also 
 * generated knownMore.h and knownMore.sql.  This module links the database and the RAM 
 * representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "jksql.h"
#include "knownMore.h"


void knownMoreStaticLoad(char **row, struct knownMore *ret)
/* Load a row from knownMore table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->transId = row[1];
ret->geneId = row[2];
ret->gbGeneName = sqlUnsigned(row[3]);
ret->gbProductName = sqlUnsigned(row[4]);
ret->gbProteinAcc = row[5];
ret->gbNgi = row[6];
ret->gbPgi = row[7];
ret->omimId = sqlUnsigned(row[8]);
ret->omimName = row[9];
ret->hugoId = sqlUnsigned(row[10]);
ret->hugoSymbol = row[11];
ret->hugoName = row[12];
ret->hugoMap = row[13];
ret->pmId1 = sqlUnsigned(row[14]);
ret->pmId2 = sqlUnsigned(row[15]);
ret->refSeqAcc = row[16];
ret->aliases = row[17];
ret->locusLinkId = sqlUnsigned(row[18]);
ret->gdbId = row[19];
}

struct knownMore *knownMoreLoad(char **row)
/* Load a knownMore from row fetched with select * from knownMore
 * from database.  Dispose of this with knownMoreFree(). */
{
struct knownMore *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->transId = cloneString(row[1]);
ret->geneId = cloneString(row[2]);
ret->gbGeneName = sqlUnsigned(row[3]);
ret->gbProductName = sqlUnsigned(row[4]);
ret->gbProteinAcc = cloneString(row[5]);
ret->gbNgi = cloneString(row[6]);
ret->gbPgi = cloneString(row[7]);
ret->omimId = sqlUnsigned(row[8]);
ret->omimName = cloneString(row[9]);
ret->hugoId = sqlUnsigned(row[10]);
ret->hugoSymbol = cloneString(row[11]);
ret->hugoName = cloneString(row[12]);
ret->hugoMap = cloneString(row[13]);
ret->pmId1 = sqlUnsigned(row[14]);
ret->pmId2 = sqlUnsigned(row[15]);
ret->refSeqAcc = cloneString(row[16]);
ret->aliases = cloneString(row[17]);
ret->locusLinkId = sqlUnsigned(row[18]);
ret->gdbId = cloneString(row[19]);
return ret;
}

struct knownMore *knownMoreCommaIn(char **pS, struct knownMore *ret)
/* Create a knownMore out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new knownMore */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->transId = sqlStringComma(&s);
ret->geneId = sqlStringComma(&s);
ret->gbGeneName = sqlUnsignedComma(&s);
ret->gbProductName = sqlUnsignedComma(&s);
ret->gbProteinAcc = sqlStringComma(&s);
ret->gbNgi = sqlStringComma(&s);
ret->gbPgi = sqlStringComma(&s);
ret->omimId = sqlUnsignedComma(&s);
ret->omimName = sqlStringComma(&s);
ret->hugoId = sqlUnsignedComma(&s);
ret->hugoSymbol = sqlStringComma(&s);
ret->hugoName = sqlStringComma(&s);
ret->hugoMap = sqlStringComma(&s);
ret->pmId1 = sqlUnsignedComma(&s);
ret->pmId2 = sqlUnsignedComma(&s);
ret->refSeqAcc = sqlStringComma(&s);
ret->aliases = sqlStringComma(&s);
ret->locusLinkId = sqlUnsignedComma(&s);
ret->gdbId = sqlStringComma(&s);
*pS = s;
return ret;
}

void knownMoreFree(struct knownMore **pEl)
/* Free a single dynamically allocated knownMore such as created
 * with knownMoreLoad(). */
{
struct knownMore *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->transId);
freeMem(el->geneId);
freeMem(el->gbProteinAcc);
freeMem(el->gbNgi);
freeMem(el->gbPgi);
freeMem(el->omimName);
freeMem(el->hugoSymbol);
freeMem(el->hugoName);
freeMem(el->hugoMap);
freeMem(el->refSeqAcc);
freeMem(el->aliases);
freeMem(el->gdbId);
freez(pEl);
}

void knownMoreFreeList(struct knownMore **pList)
/* Free a list of dynamically allocated knownMore's */
{
struct knownMore *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    knownMoreFree(&el);
    }
*pList = NULL;
}

void knownMoreOutput(struct knownMore *el, FILE *f, char sep, char lastSep) 
/* Print out knownMore.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->transId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->geneId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->gbGeneName);
fputc(sep,f);
fprintf(f, "%u", el->gbProductName);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gbProteinAcc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gbNgi);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gbPgi);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->omimId);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->omimName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->hugoId);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hugoSymbol);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hugoName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hugoMap);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->pmId1);
fputc(sep,f);
fprintf(f, "%u", el->pmId2);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->refSeqAcc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->aliases);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->locusLinkId);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gdbId);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

