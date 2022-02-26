/* ccdsInfo.c was originally generated by the autoSql program, which also 
 * generated ccdsInfo.h and ccdsInfo.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "ccdsInfo.h"
#include "genbank.h"

/* FIXME: database should really have an enum column */

static enum ccdsInfoSrcDb parseSrcDb(char *srcDbStr, char *mrnaAcc)
/* parse the srcDb string to an enum */
{
if (sameString(srcDbStr, "N"))
    return ccdsInfoNcbi;
else if (sameString(srcDbStr, "H"))
    {
    if (startsWith("OTT", mrnaAcc))
        return ccdsInfoVega;
    else
        return ccdsInfoEnsembl;
    }
errAbort("invalid ccdsInfoSrcDb column \"%s\"", srcDbStr);
return ccdsInfoNull;
}

static char *formatSrcDb(enum ccdsInfoSrcDb srcDb)
/* format the srcDb string to an enum */
{
switch (srcDb)
    {
    case ccdsInfoNcbi:
        return "N";
    case ccdsInfoVega:
    case ccdsInfoEnsembl:
        return "H";
    case ccdsInfoNull:
        assert(FALSE);
    }
 return "?";
}


void ccdsInfoStaticLoad(char **row, struct ccdsInfo *ret)
/* Load a row from ccdsInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

strcpy(ret->ccds, row[0]);
ret->srcDb = parseSrcDb(row[1], row[2]);
strcpy(ret->mrnaAcc, row[2]);
strcpy(ret->protAcc, row[3]);
}

struct ccdsInfo *ccdsInfoLoad(char **row)
/* Load a ccdsInfo from row fetched with select * from ccdsInfo
 * from database.  Dispose of this with ccdsInfoFree(). */
{
struct ccdsInfo *ret;

AllocVar(ret);
strcpy(ret->ccds, row[0]);
ret->srcDb = parseSrcDb(row[1], row[2]);
strcpy(ret->mrnaAcc, row[2]);
strcpy(ret->protAcc, row[3]);
return ret;
}

struct ccdsInfo *ccdsInfoLoadAll(char *fileName) 
/* Load all ccdsInfo from a whitespace-separated file.
 * Dispose of this with ccdsInfoFreeList(). */
{
struct ccdsInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = ccdsInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct ccdsInfo *ccdsInfoLoadAllByChar(char *fileName, char chopper) 
/* Load all ccdsInfo from a chopper separated file.
 * Dispose of this with ccdsInfoFreeList(). */
{
struct ccdsInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = ccdsInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct ccdsInfo *ccdsInfoCommaIn(char **pS, struct ccdsInfo *ret)
/* Create a ccdsInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ccdsInfo */
{
char *s = *pS;
char srcDbBuf[2];

if (ret == NULL)
    AllocVar(ret);
sqlFixedStringComma(&s, ret->ccds, sizeof(ret->ccds));
sqlFixedStringComma(&s, srcDbBuf, sizeof(srcDbBuf));
sqlFixedStringComma(&s, ret->mrnaAcc, sizeof(ret->mrnaAcc));
ret->srcDb = parseSrcDb(srcDbBuf, ret->mrnaAcc);
sqlFixedStringComma(&s, ret->protAcc, sizeof(ret->protAcc));
*pS = s;
return ret;
}

void ccdsInfoFree(struct ccdsInfo **pEl)
/* Free a single dynamically allocated ccdsInfo such as created
 * with ccdsInfoLoad(). */
{
struct ccdsInfo *el;

if ((el = *pEl) == NULL) return;
freez(pEl);
}

void ccdsInfoFreeList(struct ccdsInfo **pList)
/* Free a list of dynamically allocated ccdsInfo's */
{
struct ccdsInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    ccdsInfoFree(&el);
    }
*pList = NULL;
}

void ccdsInfoOutput(struct ccdsInfo *el, FILE *f, char sep, char lastSep) 
/* Print out ccdsInfo.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->ccds);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", formatSrcDb(el->srcDb));
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mrnaAcc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->protAcc);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

static char *createSql =
    /* SQL to create ccdsInfo format table */
    "CREATE TABLE %s (\n"
    "    ccds char(12) not null,     # CCDS id\n"
    "    srcDb char(1) not null,     # source database: N=NCBI, H=Hinxton\n"
    "    mrnaAcc char(18) not null,  # mRNA accession (NCBI or Hinxton)\n"
    "    protAcc char(18) not null,  # protein accession (NCBI or Hinxton)\n"
    "              #Indices\n"
    "    INDEX(ccds),\n"
    "    INDEX(mrnaAcc)\n"
    ");\n";

char *ccdsInfoGetCreateSql(char *table)
/* Get sql command to create ccdsInfo table. Result should be freed. */
{
char sql[1024];
sqlSafef(sql, sizeof(sql), createSql, table);
return cloneString(sql);
}

static int cmpMRna(const void *va, const void *vb)
/* Compare to sort based on mrnaAcc. */
{
const struct ccdsInfo *a = *((struct ccdsInfo **)va);
const struct ccdsInfo *b = *((struct ccdsInfo **)vb);
return strcmp(a->mrnaAcc, b->mrnaAcc);
}

void ccdsInfoMRnaSort(struct ccdsInfo **ccdsInfos)
/* Sort list by mrnaAcc */
{
slSort(ccdsInfos, cmpMRna);
}

static int cmpCcdsMRna(const void *va, const void *vb)
/* Compare to sort based on CCDS id and mrnaAcc. */
{
const struct ccdsInfo *a = *((struct ccdsInfo **)va);
const struct ccdsInfo *b = *((struct ccdsInfo **)vb);


int dif = strcmp(a->ccds, b->ccds);
if (dif == 0)
    dif = strcmp(a->mrnaAcc, b->mrnaAcc);
return dif;
}

void ccdsInfoCcdsMRnaSort(struct ccdsInfo **ccdsInfos)
/* Sort list by CCDS id and mrnaAcc */
{
slSort(ccdsInfos, cmpCcdsMRna);
}

static char *getSrcDbWhere(enum ccdsInfoSrcDb srcDb)
/* generate where expression for CCDS srcDb.  Note: static return!! */
{
static char extraWhere[64];
switch (srcDb) {
case ccdsInfoNull:
    extraWhere[0] = '\0';
    break;
case ccdsInfoNcbi:
    safef(extraWhere, sizeof(extraWhere), " and srcDb = 'N'");
    break;
case ccdsInfoVega:
    safef(extraWhere, sizeof(extraWhere), " and srcDb = 'H' and mrnaAcc like 'OTT%%'");
    break;
case ccdsInfoEnsembl:
    safef(extraWhere, sizeof(extraWhere), " and srcDb = 'H' and mrnaAcc not like 'OTT%%'");
    break;
}
return extraWhere;
}

struct ccdsInfo *ccdsInfoSelectByCcds(struct sqlConnection *conn, char *ccdsId,
                                      enum ccdsInfoSrcDb srcDb)
/* Obtain list of ccdsInfo object for the specified id and srcDb.  If srcDb is
 * ccdsInfoNull, return all srcDbs.  Return NULL if ccdsId it's not valid */
{
char query[256];
struct sqlResult *sr;
char **row;
struct ccdsInfo *ccdsInfos = NULL;

sqlSafef(query, sizeof(query), "select * from ccdsInfo where ccds = '%s'%-s",
      ccdsId, getSrcDbWhere(srcDb));
sr = sqlGetResult(conn, query);

while ((row = sqlNextRow(sr)) != NULL)
    slSafeAddHead(&ccdsInfos, ccdsInfoLoad(row));
sqlFreeResult(&sr);

return ccdsInfos;
}

struct ccdsInfo *ccdsInfoSelectByMrna(struct sqlConnection *conn, char *mrnaAcc)
/* Obtain of ccdsInfo object for the specified mRNA or NULL if mrna is not
 * associated with a CCDS.  Version number is optional for RefSeq mrnaAcc */
{
char query[256];
struct sqlResult *sr;
char **row;
struct ccdsInfo *ccdsInfo = NULL;

if (genbankIsRefSeqAcc(mrnaAcc) && (strchr(mrnaAcc, '.') == NULL))
    sqlSafef(query, sizeof(query), "select * from ccdsInfo where mrnaAcc like '%s.%%'",
          mrnaAcc);
else
    sqlSafef(query, sizeof(query), "select * from ccdsInfo where mrnaAcc = '%s'",
          mrnaAcc);
sr = sqlGetResult(conn, query);

/* should only get one, but this is easier to code */
while ((row = sqlNextRow(sr)) != NULL)
    slSafeAddHead(&ccdsInfo, ccdsInfoLoad(row));
sqlFreeResult(&sr);

if ((ccdsInfo != NULL) && (ccdsInfo->next != NULL))
    errAbort("obtained multiple CCDSs for mRNA %s", mrnaAcc);

return ccdsInfo;
}
