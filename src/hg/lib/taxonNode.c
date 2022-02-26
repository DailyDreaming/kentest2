/* taxonNode.c was originally generated by the autoSql program, which also 
 * generated taxonNode.h and taxonNode.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "taxonNode.h"


void taxonNodeStaticLoad(char **row, struct taxonNode *ret)
/* Load a row from taxonNode table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->taxon = sqlUnsigned(row[0]);
ret->parent = sqlUnsigned(row[1]);
ret->rank = row[2];
ret->emblcode = row[3];
ret->division = sqlUnsigned(row[4]);
ret->inheritedDiv = sqlUnsigned(row[5]);
ret->geneticCode = sqlUnsigned(row[6]);
ret->inheritedGC = sqlUnsigned(row[7]);
ret->mitoGeneticCode = sqlUnsigned(row[8]);
ret->inheritedMitoGC = sqlUnsigned(row[9]);
ret->GenBankHidden = sqlUnsigned(row[10]);
ret->notSequenced = sqlUnsigned(row[11]);
ret->comments = row[12];
}

struct taxonNode *taxonNodeLoad(char **row)
/* Load a taxonNode from row fetched with select * from taxonNode
 * from database.  Dispose of this with taxonNodeFree(). */
{
struct taxonNode *ret;

AllocVar(ret);
ret->taxon = sqlUnsigned(row[0]);
ret->parent = sqlUnsigned(row[1]);
ret->rank = cloneString(row[2]);
ret->emblcode = cloneString(row[3]);
ret->division = sqlUnsigned(row[4]);
ret->inheritedDiv = sqlUnsigned(row[5]);
ret->geneticCode = sqlUnsigned(row[6]);
ret->inheritedGC = sqlUnsigned(row[7]);
ret->mitoGeneticCode = sqlUnsigned(row[8]);
ret->inheritedMitoGC = sqlUnsigned(row[9]);
ret->GenBankHidden = sqlUnsigned(row[10]);
ret->notSequenced = sqlUnsigned(row[11]);
ret->comments = cloneString(row[12]);
return ret;
}

struct taxonNode *taxonNodeLoadAll(char *fileName) 
/* Load all taxonNode from a whitespace-separated file.
 * Dispose of this with taxonNodeFreeList(). */
{
struct taxonNode *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[13];

while (lineFileRow(lf, row))
    {
    el = taxonNodeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct taxonNode *taxonNodeLoadAllByChar(char *fileName, char chopper) 
/* Load all taxonNode from a chopper separated file.
 * Dispose of this with taxonNodeFreeList(). */
{
struct taxonNode *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[13];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = taxonNodeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct taxonNode *taxonNodeLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all taxonNode from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with taxonNodeFreeList(). */
{
struct taxonNode *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = taxonNodeLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void taxonNodeSaveToDb(struct sqlConnection *conn, struct taxonNode *el, char *tableName, int updateSize)
/* Save taxonNode as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */
{
struct dyString *update = newDyString(updateSize);
sqlDyStringPrintf(update, "insert into %s values ( %u,%u,'%s','%s',%u,%u,%u,%u,%u,%u,%u,%u,'%s')", 
	tableName,  el->taxon,  el->parent,  el->rank,  el->emblcode,  el->division,  el->inheritedDiv,  el->geneticCode,  el->inheritedGC,  el->mitoGeneticCode,  el->inheritedMitoGC,  el->GenBankHidden,  el->notSequenced,  el->comments);
sqlUpdate(conn, update->string);
freeDyString(&update);
}


struct taxonNode *taxonNodeCommaIn(char **pS, struct taxonNode *ret)
/* Create a taxonNode out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new taxonNode */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->taxon = sqlUnsignedComma(&s);
ret->parent = sqlUnsignedComma(&s);
ret->rank = sqlStringComma(&s);
ret->emblcode = sqlStringComma(&s);
ret->division = sqlUnsignedComma(&s);
ret->inheritedDiv = sqlUnsignedComma(&s);
ret->geneticCode = sqlUnsignedComma(&s);
ret->inheritedGC = sqlUnsignedComma(&s);
ret->mitoGeneticCode = sqlUnsignedComma(&s);
ret->inheritedMitoGC = sqlUnsignedComma(&s);
ret->GenBankHidden = sqlUnsignedComma(&s);
ret->notSequenced = sqlUnsignedComma(&s);
ret->comments = sqlStringComma(&s);
*pS = s;
return ret;
}

void taxonNodeFree(struct taxonNode **pEl)
/* Free a single dynamically allocated taxonNode such as created
 * with taxonNodeLoad(). */
{
struct taxonNode *el;

if ((el = *pEl) == NULL) return;
freeMem(el->rank);
freeMem(el->emblcode);
freeMem(el->comments);
freez(pEl);
}

void taxonNodeFreeList(struct taxonNode **pList)
/* Free a list of dynamically allocated taxonNode's */
{
struct taxonNode *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    taxonNodeFree(&el);
    }
*pList = NULL;
}

void taxonNodeOutput(struct taxonNode *el, FILE *f, char sep, char lastSep) 
/* Print out taxonNode.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%u", el->taxon);
fputc(sep,f);
fprintf(f, "%u", el->parent);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rank);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->emblcode);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->division);
fputc(sep,f);
fprintf(f, "%u", el->inheritedDiv);
fputc(sep,f);
fprintf(f, "%u", el->geneticCode);
fputc(sep,f);
fprintf(f, "%u", el->inheritedGC);
fputc(sep,f);
fprintf(f, "%u", el->mitoGeneticCode);
fputc(sep,f);
fprintf(f, "%u", el->inheritedMitoGC);
fputc(sep,f);
fprintf(f, "%u", el->GenBankHidden);
fputc(sep,f);
fprintf(f, "%u", el->notSequenced);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->comments);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */
