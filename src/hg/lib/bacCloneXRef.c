/* bacCloneXRef.c was originally generated by the autoSql program, which also 
 * generated bacCloneXRef.h and bacCloneXRef.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "bacCloneXRef.h"


void bacCloneXRefStaticLoad(char **row, struct bacCloneXRef *ret)
/* Load a row from bacCloneXRef table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->intName = row[1];
ret->chroms = row[2];
ret->genbank = row[3];
ret->sangerName = row[4];
ret->relationship = sqlUnsigned(row[5]);
ret->uniStsId = row[6];
ret->leftPrimer = row[7];
ret->rightPrimer = row[8];
}

struct bacCloneXRef *bacCloneXRefLoad(char **row)
/* Load a bacCloneXRef from row fetched with select * from bacCloneXRef
 * from database.  Dispose of this with bacCloneXRefFree(). */
{
struct bacCloneXRef *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->intName = cloneString(row[1]);
ret->chroms = cloneString(row[2]);
ret->genbank = cloneString(row[3]);
ret->sangerName = cloneString(row[4]);
ret->relationship = sqlUnsigned(row[5]);
ret->uniStsId = cloneString(row[6]);
ret->leftPrimer = cloneString(row[7]);
ret->rightPrimer = cloneString(row[8]);
return ret;
}

struct bacCloneXRef *bacCloneXRefLoadAll(char *fileName) 
/* Load all bacCloneXRef from a whitespace-separated file.
 * Dispose of this with bacCloneXRefFreeList(). */
{
struct bacCloneXRef *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileRow(lf, row))
    {
    el = bacCloneXRefLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bacCloneXRef *bacCloneXRefLoadAllByChar(char *fileName, char chopper) 
/* Load all bacCloneXRef from a chopper separated file.
 * Dispose of this with bacCloneXRefFreeList(). */
{
struct bacCloneXRef *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = bacCloneXRefLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bacCloneXRef *bacCloneXRefCommaIn(char **pS, struct bacCloneXRef *ret)
/* Create a bacCloneXRef out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bacCloneXRef */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->intName = sqlStringComma(&s);
ret->chroms = sqlStringComma(&s);
ret->genbank = sqlStringComma(&s);
ret->sangerName = sqlStringComma(&s);
ret->relationship = sqlUnsignedComma(&s);
ret->uniStsId = sqlStringComma(&s);
ret->leftPrimer = sqlStringComma(&s);
ret->rightPrimer = sqlStringComma(&s);
*pS = s;
return ret;
}

void bacCloneXRefFree(struct bacCloneXRef **pEl)
/* Free a single dynamically allocated bacCloneXRef such as created
 * with bacCloneXRefLoad(). */
{
struct bacCloneXRef *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->intName);
freeMem(el->chroms);
freeMem(el->genbank);
freeMem(el->sangerName);
freeMem(el->uniStsId);
freeMem(el->leftPrimer);
freeMem(el->rightPrimer);
freez(pEl);
}

void bacCloneXRefFreeList(struct bacCloneXRef **pList)
/* Free a list of dynamically allocated bacCloneXRef's */
{
struct bacCloneXRef *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bacCloneXRefFree(&el);
    }
*pList = NULL;
}

void bacCloneXRefOutput(struct bacCloneXRef *el, FILE *f, char sep, char lastSep) 
/* Print out bacCloneXRef.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->intName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chroms);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genbank);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->sangerName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->relationship);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->uniStsId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->leftPrimer);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rightPrimer);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

