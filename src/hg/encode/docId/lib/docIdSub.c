/* docIdSub.c was originally generated by the autoSql program, which also 
 * generated docIdSub.h and docIdSub.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "docId.h"
#include "portable.h"

//char *docIdTable = "docIdSub";

void docIdSubStaticLoad(char **row, struct docIdSub *ret)
/* Load a row from docIdSub table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->ix = sqlSigned(row[0]);
ret->status = sqlSigned(row[1]);
ret->assembly = row[2];
ret->submitDate = row[3];
ret->md5sum = row[4];
ret->valReport = row[5];
ret->valVersion = row[6];
ret->metaData = row[7];
ret->submitPath = row[8];
ret->submitter = row[9];
}

struct docIdSub *docIdSubLoad(char **row)
/* Load a docIdSub from row fetched with select * from docIdSub
 * from database.  Dispose of this with docIdSubFree(). */
{
struct docIdSub *ret;

AllocVar(ret);
ret->ix = sqlSigned(row[0]);
ret->status = sqlSigned(row[1]);
ret->assembly = cloneString(row[2]);
ret->submitDate = cloneString(row[3]);
ret->md5sum = cloneString(row[4]);
ret->valReport = cloneString(row[5]);
ret->valVersion = cloneString(row[6]);
ret->metaData = cloneString(row[7]);
ret->submitPath = cloneString(row[8]);
ret->submitter = cloneString(row[9]);
return ret;
}

struct docIdSub *docIdSubLoadAll(char *fileName) 
/* Load all docIdSub from a whitespace-separated file.
 * Dispose of this with docIdSubFreeList(). */
{
struct docIdSub *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[10];

while (lineFileRow(lf, row))
    {
    el = docIdSubLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct docIdSub *docIdSubLoadAllByChar(char *fileName, char chopper) 
/* Load all docIdSub from a chopper separated file.
 * Dispose of this with docIdSubFreeList(). */
{
struct docIdSub *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[10];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = docIdSubLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct docIdSub *docIdSubCommaIn(char **pS, struct docIdSub *ret)
/* Create a docIdSub out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new docIdSub */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->ix = sqlSignedComma(&s);
ret->status = sqlSignedComma(&s);
ret->assembly = sqlStringComma(&s);
ret->submitDate = sqlStringComma(&s);
ret->md5sum = sqlStringComma(&s);
ret->valReport = sqlStringComma(&s);
ret->valVersion = sqlStringComma(&s);
ret->metaData = sqlStringComma(&s);
ret->submitPath = sqlStringComma(&s);
ret->submitter = sqlStringComma(&s);
*pS = s;
return ret;
}

void docIdSubFree(struct docIdSub **pEl)
/* Free a single dynamically allocated docIdSub such as created
 * with docIdSubLoad(). */
{
struct docIdSub *el;

if ((el = *pEl) == NULL) return;
freeMem(el->assembly);
freeMem(el->submitDate);
freeMem(el->md5sum);
freeMem(el->valReport);
freeMem(el->valVersion);
freeMem(el->metaData);
freeMem(el->submitPath);
freeMem(el->submitter);
freez(pEl);
}

void docIdSubFreeList(struct docIdSub **pList)
/* Free a list of dynamically allocated docIdSub's */
{
struct docIdSub *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    docIdSubFree(&el);
    }
*pList = NULL;
}

void docIdSubOutput(struct docIdSub *el, FILE *f, char sep, char lastSep) 
/* Print out docIdSub.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->ix);
fputc(sep,f);
fprintf(f, "%d", el->status);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->assembly);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->submitDate);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->md5sum);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->valReport);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->valVersion);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->metaData);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->submitPath);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->submitter);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

char *docIdDecorate(char *composite, char *cellType, int num)
{
char buffer[10 * 1024];

safef(buffer, sizeof buffer, "%s%sD%09d",composite, cellType, num);
return cloneString(buffer);
}


boolean fileIsCompressed(char *fileName)
{
char *dot = strrchr(fileName, '.');

if (dot == NULL)
    return FALSE;

dot++;

if (sameString(dot, "bam") ||
    sameString(dot, "bigWig") ||
    sameString(dot, "gz"))
    return TRUE;

return FALSE;
}

char *docDecorateType(char *type)
// add .gz for types that should be compressed
{
if (sameString(type, "bam") ||
    sameString(type, "bigWig"))
        return type;

char *ptr;

if ((ptr = strchr(type, ' ')) != NULL)
    *ptr = 0;

char buffer[10 * 1024];

safef(buffer, sizeof buffer, "%s.gz", type);

if (ptr)
    *ptr = ' ';

return cloneString(buffer);
}

char *docIdGetPath(char *docId, char *docIdDir, char *type, char *submitPath)
{
char *ptr = docId + strlen(docId) - 1;
struct dyString *dy = newDyString(20);
char *suffix = docDecorateType(type);

dyStringPrintf(dy, "%s/", docIdDir);
for (; ptr != docId; ptr--)
    {
    dyStringPrintf(dy, "%c/", *ptr);   
    }

dyStringPrintf(dy, "%s.%s", docId, suffix);

return dyStringCannibalize(&dy);
}

char *nullString = "";

void fillNull(char **val)
{
if (*val == NULL)
    *val = nullString;
}

char *docIdSubmit(struct sqlConnection *conn, char *docIdTable, struct docIdSub *docIdSub, 
    char *docIdDir, char *type)
{

verbose(2, "Submitting------\n");
verbose(2, "status %d\n", docIdSub->status);
verbose(2, "assembly %s\n", docIdSub->assembly);
verbose(2, "submitDate %s\n", docIdSub->submitDate);
verbose(2, "md5sum %s\n", docIdSub->md5sum);
verbose(2, "valReport %s\n", docIdSub->valReport);
verbose(2, "valVersion %s\n", docIdSub->valVersion);
verbose(2, "metaData %s\n", docIdSub->metaData);
verbose(2, "submitPath %s\n", docIdSub->submitPath);
verbose(2, "submitter %s\n", docIdSub->submitter);
verbose(2, "type %s\n", type);

char query[1024 * 1024];

fillNull(&docIdSub->valReport);
fillNull(&docIdSub->md5sum);

sqlSafef(query, sizeof query, "insert into %s (status, assembly, submitDate, md5sum, valReport, valVersion, metaData, submitPath, submitter) values (\"%d\",\"%s\",\"%s\",\"%s\", \"%s\", \"%s\", \"%s\",\"%s\",\"%s\")\n", docIdTable,
    docIdSub->status, docIdSub->assembly, docIdSub->submitDate, docIdSub->md5sum, docIdSub->valReport, docIdSub->valVersion, docIdSub->metaData, docIdSub->submitPath, docIdSub->submitter);
    //docIdSub->submitDate, docIdSub->md5sum, docIdSub->valReport, "null", docIdSub->submitPath, docIdSub->submitter);
//printf("query is %s\n", query);
char *response = sqlQuickString(conn, query);

printf("submitted got response %s\n", response);

sqlSafef(query, sizeof query, "select last_insert_id()");
char *docId = cloneString(sqlQuickString(conn, query));

printf("submitted got docId %s\n", docId);


if (!fileExists(docIdSub->submitPath))
    errAbort("cannot open %s\n", docIdSub->submitPath);
char *linkToFile = docIdGetPath(docId, docIdDir, type, docIdSub->submitPath);

printf("linking %s to file %s\n", docIdSub->submitPath, linkToFile);
char *slash = strrchr(linkToFile, '/');
if (slash == NULL)
    errAbort("can't find slash in path %s\n", linkToFile);

*slash = 0;
makeDirsOnPath(linkToFile);
*slash = '/';
if (link(docIdSub->submitPath, linkToFile) < 0)
    errnoAbort("can't link %s to file %s\n", docIdSub->submitPath, linkToFile);

return docId;
}