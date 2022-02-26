/* encodeRna.c was originally generated by the autoSql program, which also 
 * generated encodeRna.h and encodeRna.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "encode/encodeRna.h"


void encodeRnaStaticLoad(char **row, struct encodeRna *ret)
/* Load a row from encodeRna table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
ret->strand = row[5];
ret->source = row[6];
ret->type = row[7];
ret->fullScore = atof(row[8]);
ret->isPsuedo = sqlUnsigned(row[9]);
ret->isRmasked = sqlUnsigned(row[10]);
ret->isTranscribed = sqlUnsigned(row[11]);
ret->isPrediction = sqlUnsigned(row[12]);
ret->transcribedIn = row[13];
}

struct encodeRna *encodeRnaLoad(char **row)
/* Load a encodeRna from row fetched with select * from encodeRna
 * from database.  Dispose of this with encodeRnaFree(). */
{
struct encodeRna *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
ret->strand = cloneString(row[5]);
ret->source = cloneString(row[6]);
ret->type = cloneString(row[7]);
ret->fullScore = atof(row[8]);
ret->isPsuedo = sqlUnsigned(row[9]);
ret->isRmasked = sqlUnsigned(row[10]);
ret->isTranscribed = sqlUnsigned(row[11]);
ret->isPrediction = sqlUnsigned(row[12]);
ret->transcribedIn = cloneString(row[13]);
return ret;
}

struct encodeRna *encodeRnaLoadAll(char *fileName) 
/* Load all encodeRna from a whitespace-separated file.
 * Dispose of this with encodeRnaFreeList(). */
{
struct encodeRna *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileRow(lf, row))
    {
    el = encodeRnaLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeRna *encodeRnaLoadAllByChar(char *fileName, char chopper) 
/* Load all encodeRna from a chopper separated file.
 * Dispose of this with encodeRnaFreeList(). */
{
struct encodeRna *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = encodeRnaLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeRna *encodeRnaLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all encodeRna from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with encodeRnaFreeList(). */
{
struct encodeRna *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = encodeRnaLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void encodeRnaSaveToDb(struct sqlConnection *conn, struct encodeRna *el, char *tableName, int updateSize)
/* Save encodeRna as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */
{
struct dyString *update = newDyString(updateSize);
sqlDyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s','%s','%s',%g,%u,%u,%u,%u,'%s')", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->name,  el->score,  el->strand,  el->source,  el->type,  el->fullScore,  el->isPsuedo,  el->isRmasked,  el->isTranscribed,  el->isPrediction,  el->transcribedIn);
sqlUpdate(conn, update->string);
freeDyString(&update);
}


struct encodeRna *encodeRnaCommaIn(char **pS, struct encodeRna *ret)
/* Create a encodeRna out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodeRna */
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
ret->source = sqlStringComma(&s);
ret->type = sqlStringComma(&s);
ret->fullScore = sqlFloatComma(&s);
ret->isPsuedo = sqlUnsignedComma(&s);
ret->isRmasked = sqlUnsignedComma(&s);
ret->isTranscribed = sqlUnsignedComma(&s);
ret->isPrediction = sqlUnsignedComma(&s);
ret->transcribedIn = sqlStringComma(&s);
*pS = s;
return ret;
}

void encodeRnaFree(struct encodeRna **pEl)
/* Free a single dynamically allocated encodeRna such as created
 * with encodeRnaLoad(). */
{
struct encodeRna *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->strand);
freeMem(el->source);
freeMem(el->type);
freeMem(el->transcribedIn);
freez(pEl);
}

void encodeRnaFreeList(struct encodeRna **pList)
/* Free a list of dynamically allocated encodeRna's */
{
struct encodeRna *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    encodeRnaFree(&el);
    }
*pList = NULL;
}

void encodeRnaOutput(struct encodeRna *el, FILE *f, char sep, char lastSep) 
/* Print out encodeRna.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->source);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%g", el->fullScore);
fputc(sep,f);
fprintf(f, "%u", el->isPsuedo);
fputc(sep,f);
fprintf(f, "%u", el->isRmasked);
fputc(sep,f);
fprintf(f, "%u", el->isTranscribed);
fputc(sep,f);
fprintf(f, "%u", el->isPrediction);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->transcribedIn);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

