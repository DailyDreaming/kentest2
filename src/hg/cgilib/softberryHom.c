/* softberryHom.c was originally generated by the autoSql program, which also 
 * generated softberryHom.h and softberryHom.sql.  This module links the database and the RAM 
 * representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "jksql.h"
#include "softberryHom.h"


void softberryHomStaticLoad(char **row, struct softberryHom *ret)
/* Load a row from softberryHom table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->giString = row[1];
ret->description = row[2];
}

struct softberryHom *softberryHomLoad(char **row)
/* Load a softberryHom from row fetched with select * from softberryHom
 * from database.  Dispose of this with softberryHomFree(). */
{
struct softberryHom *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->giString = cloneString(row[1]);
ret->description = cloneString(row[2]);
return ret;
}

struct softberryHom *softberryHomCommaIn(char **pS, struct softberryHom *ret)
/* Create a softberryHom out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new softberryHom */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->giString = sqlStringComma(&s);
ret->description = sqlStringComma(&s);
*pS = s;
return ret;
}

void softberryHomFree(struct softberryHom **pEl)
/* Free a single dynamically allocated softberryHom such as created
 * with softberryHomLoad(). */
{
struct softberryHom *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->giString);
freeMem(el->description);
freez(pEl);
}

void softberryHomFreeList(struct softberryHom **pList)
/* Free a list of dynamically allocated softberryHom's */
{
struct softberryHom *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    softberryHomFree(&el);
    }
*pList = NULL;
}

void softberryHomOutput(struct softberryHom *el, FILE *f, char sep, char lastSep) 
/* Print out softberryHom.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->giString);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->description);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

