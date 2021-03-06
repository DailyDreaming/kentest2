/* geneScore.c was originally generated by the autoSql program, which also 
 * generated geneScore.h and geneScore.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "geneScore.h"


void geneScoreStaticLoad(char **row, struct geneScore *ret)
/* Load a row from geneScore table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->chrom = row[1];
ret->txStart = sqlSigned(row[2]);
ret->score = atof(row[3]);
}

struct geneScore *geneScoreLoad(char **row)
/* Load a geneScore from row fetched with select * from geneScore
 * from database.  Dispose of this with geneScoreFree(). */
{
struct geneScore *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->chrom = cloneString(row[1]);
ret->txStart = sqlSigned(row[2]);
ret->score = atof(row[3]);
return ret;
}

struct geneScore *geneScoreLoadAll(char *fileName) 
/* Load all geneScore from a whitespace-separated file.
 * Dispose of this with geneScoreFreeList(). */
{
struct geneScore *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = geneScoreLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct geneScore *geneScoreLoadAllByChar(char *fileName, char chopper) 
/* Load all geneScore from a chopper separated file.
 * Dispose of this with geneScoreFreeList(). */
{
struct geneScore *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = geneScoreLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct geneScore *geneScoreCommaIn(char **pS, struct geneScore *ret)
/* Create a geneScore out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new geneScore */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->chrom = sqlStringComma(&s);
ret->txStart = sqlSignedComma(&s);
ret->score = sqlFloatComma(&s);
*pS = s;
return ret;
}

void geneScoreFree(struct geneScore **pEl)
/* Free a single dynamically allocated geneScore such as created
 * with geneScoreLoad(). */
{
struct geneScore *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->chrom);
freez(pEl);
}

void geneScoreFreeList(struct geneScore **pList)
/* Free a list of dynamically allocated geneScore's */
{
struct geneScore *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    geneScoreFree(&el);
    }
*pList = NULL;
}

void geneScoreOutput(struct geneScore *el, FILE *f, char sep, char lastSep) 
/* Print out geneScore.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->txStart);
fputc(sep,f);
fprintf(f, "%g", el->score);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

