/* knownInfo.c was originally generated by the autoSql program, which also 
 * generated knownInfo.h and knownInfo.sql.  This module links the database and the RAM 
 * representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "jksql.h"
#include "knownInfo.h"


void knownInfoStaticLoad(char **row, struct knownInfo *ret)
/* Load a row from knownInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->transId = row[1];
ret->geneId = row[2];
ret->geneName = sqlUnsigned(row[3]);
ret->productName = sqlUnsigned(row[4]);
ret->proteinId = row[5];
ret->ngi = row[6];
ret->pgi = row[7];
}

struct knownInfo *knownInfoLoad(char **row)
/* Load a knownInfo from row fetched with select * from knownInfo
 * from database.  Dispose of this with knownInfoFree(). */
{
struct knownInfo *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->transId = cloneString(row[1]);
ret->geneId = cloneString(row[2]);
ret->geneName = sqlUnsigned(row[3]);
ret->productName = sqlUnsigned(row[4]);
ret->proteinId = cloneString(row[5]);
ret->ngi = cloneString(row[6]);
ret->pgi = cloneString(row[7]);
return ret;
}

struct knownInfo *knownInfoCommaIn(char **pS, struct knownInfo *ret)
/* Create a knownInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new knownInfo */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->transId = sqlStringComma(&s);
ret->geneId = sqlStringComma(&s);
ret->geneName = sqlUnsignedComma(&s);
ret->productName = sqlUnsignedComma(&s);
ret->proteinId = sqlStringComma(&s);
ret->ngi = sqlStringComma(&s);
ret->pgi = sqlStringComma(&s);
*pS = s;
return ret;
}

void knownInfoFree(struct knownInfo **pEl)
/* Free a single dynamically allocated knownInfo such as created
 * with knownInfoLoad(). */
{
struct knownInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->transId);
freeMem(el->geneId);
freeMem(el->proteinId);
freeMem(el->ngi);
freeMem(el->pgi);
freez(pEl);
}

void knownInfoFreeList(struct knownInfo **pList)
/* Free a list of dynamically allocated knownInfo's */
{
struct knownInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    knownInfoFree(&el);
    }
*pList = NULL;
}

void knownInfoOutput(struct knownInfo *el, FILE *f, char sep, char lastSep) 
/* Print out knownInfo.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->geneName);
fputc(sep,f);
fprintf(f, "%u", el->productName);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->proteinId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->ngi);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->pgi);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}
