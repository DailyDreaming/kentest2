/* Joining.c - stuff to help create joined queries.  Most of
 * the joining is happening in C rather than SQL, mostly so
 * that we can filter on an ID hash as we go, and also deal
 * with prefixes and suffixes to the dang keys. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "localmem.h"
#include "memalloc.h"
#include "dystring.h"
#include "obscure.h"
#include "jksql.h"
#include "joiner.h"
#include "hdb.h"
#include "hgTables.h"
#include "trackHub.h"
#include "hubConnect.h"



struct joinedRow
/* A row that is joinable.  Allocated in joinableResult->lm. */
    {
    struct joinedRow *next;
    struct slName **fields;	/* Fields user has requested. */
    struct slName **keys;	/* Fields to key from. */
    bool passedFilter;		/* TRUE if row has passed filter at this stage. */
    bool hitThisTable;		/* TRUE if hit on this table. */
    };

struct joinedTables
/* Database query result table that is joinable. */
    {
    struct joinedTables *next;
    struct lm *lm;	/* Local memory structure - used for row allocations. */
    struct joinerDtf *fieldList;	/* Fields user has requested. */
    int fieldCount;	/* Number of fields - calculated at start of load. */
    struct joinerDtf *keyList;	/* List of keys. */
    int keyCount;	/* Number of keys- calculated at start of load. */
    struct joinedRow *rowList;	/* Rows - allocated in lm. */
    int rowCount;	/* Number of rows. */
    int maxRowCount;	/* Max row count allowed (0 means no limit) */
    struct dyString *filter;  /* Filter if any applied. */
    };

static void joinedTablesSepOutFile(struct joinedTables *joined, FILE *f, char outSep)
/* write outSep separated fields to file handle */
{
struct joinedRow *jr;
struct joinerDtf *field;
int outCount = 0;

if (f == NULL)
    f = stdout;

/* Print out field names. */
if (joined->filter)
    {
    fprintf(f, "#filter: %s\n", joined->filter->string);
    }
fprintf(f, "#");
for (field = joined->fieldList; field != NULL; field = field->next)
    {
    if (outSep == ',') fputc('"', f);
    fprintf(f, "%s.%s.%s", field->database, field->table, field->field);
    if (outSep == ',') fputc('"', f);
    if (field->next == NULL)
        fprintf(f, "\n");
    else
        fprintf(f, "%c", outSep);
    }
for (jr = joined->rowList; jr != NULL; jr = jr->next)
    {
    int i;
    if (jr->passedFilter)
        {
        for (i=0; i<joined->fieldCount; ++i)
            {
            struct slName *s;
            if (i != 0)
                fprintf(f, "%c", outSep);
            s = jr->fields[i];
            if (s == NULL)
                {
                if (outSep == ',') fputc('"', f);
                fprintf(f, "n/a");
                if (outSep == ',') fputc('"', f);
                }
            else if (s->next == NULL)
                {
                if (outSep == ',') fputc('"', f);
                fprintf(f, "%s", s->name);
                if (outSep == ',') fputc('"', f);
                }
            else
                {
                char *lastS = NULL;
                if (outSep == ',') fputc('"', f);
                while (s != NULL)
                    {
                    if (lastS == NULL || !sameString(lastS, s->name))
                        {
                        fprintf(f, "%s,", s->name);
                        }
                    lastS = s->name;
                    s = s->next;
                    }
                if (outSep == ',') fputc('"', f);
                }
            }
        fprintf(f, "\n");
        ++outCount;
        }
    }
}

static struct joinedTables *joinedTablesNew(int fieldCount, 
	int keyCount, int maxRowCount)
/* Make up new empty joinedTables. */
{
struct joinedTables *jt;

AllocVar(jt);
jt->lm = lmInit(64*1024);
jt->fieldCount = fieldCount;
jt->keyCount = keyCount;
jt->maxRowCount = maxRowCount;
return jt;
}

static void joinedTablesFree(struct joinedTables **pJt)
/* Free up joinable table and associated rows. */
{
struct joinedTables *jt = *pJt;
if (jt != NULL)
    {
    lmCleanup(&jt->lm);
    joinerDtfFreeList(&jt->fieldList);
    joinerDtfFreeList(&jt->keyList);
    dyStringFree(&jt->filter);
    freez(pJt);
    }
}

static struct joinedRow *jrRowAdd(struct joinedTables *joined, char **row,
	int fieldCount, int keyCount)
/* Add new row to joinable table. */
{
if (joined->maxRowCount != 0 && joined->rowCount >= joined->maxRowCount)
    {
    warn("Stopping after %d rows, try restricting region or adding filter", 
    	joined->rowCount);
    return NULL;
    }
else
    {
    struct joinedRow *jr;
    int i;
    struct lm *lm = joined->lm;
    lmAllocVar(lm, jr);
    lmAllocArray(lm, jr->fields, joined->fieldCount);
    lmAllocArray(lm, jr->keys, joined->keyCount);
    jr->passedFilter = TRUE;
    for (i=0; i<fieldCount; ++i)
	jr->fields[i] = lmSlName(lm, row[i]);
    row += fieldCount;
    for (i=0; i<keyCount; ++i)
	jr->keys[i] = lmSlName(lm, row[i]);
    slAddHead(&joined->rowList, jr);
    joined->rowCount += 1;
    return jr;
    }
}

static void jrRowExpand(struct joinedTables *joined, 
	struct joinedRow *jr, char **row,
        int fieldOffset, int fieldCount, int keyOffset, int keyCount)
/* Add some more to row. */
{
int i;
struct slName **keys = jr->keys + keyOffset;
struct slName **fields = jr->fields + fieldOffset;
struct lm *lm = joined->lm;
struct slName *name;

if (fieldCount + fieldOffset > joined->fieldCount)
    internalErr();
if (keyCount + keyOffset > joined->keyCount)
    internalErr();
for (i=0; i<fieldCount; ++i)
    {
    name = lmSlName(lm, row[i]);
    slAddTail(&fields[i], name);
    }
row += fieldCount;
for (i=0; i<keyCount; ++i)
    {
    name = lmSlName(lm, row[i]);
    slAddHead(&keys[i], name);
    }
}

static char *chopKey(struct slName *prefixList, 
	struct slName *suffixList, char *key)
/* Chop off any prefixes/suffixes from key. */
{
struct slName *n;

for (n=prefixList; n != NULL; n = n->next)
    {
    if (startsWith(n->name, key))
	{
        key += strlen(n->name);
	break;
	}
    }
for (n=suffixList; n!=NULL; n = n->next)
    {
    char *e = strstr(key, n->name);
    if (e != NULL)
        {
	*e = 0;
	break;
	}
    }
return key;
}

static struct joinerDtf *fieldsToDtfs(struct slName *fieldList)
/* Convert list from dotted triple to dtf format. */
{
struct joinerDtf *dtfList = NULL, *dtf;
struct slName *field;
for (field = fieldList; field != NULL; field = field->next)
    {
    dtf = joinerDtfFromDottedTriple(field->name);
    slAddHead(&dtfList, dtf);
    }
slReverse(&dtfList);
return dtfList;
}

static void makeOrderedCommaFieldList(struct slName *orderList, 
	struct joinerDtf *dtfList, struct dyString *dy)
/* Given list in order, and a subset of fields to use, make
 * a comma separated list of that subset in order. */
{
struct joinerDtf *dtf;
struct slName *order;
boolean first = TRUE;
struct hash *dtfHash = newHash(8);

/* Build up hash of field names. */
for (dtf = dtfList; dtf != NULL; dtf = dtf->next)
    hashAdd(dtfHash, dtf->field, NULL);
for (order = orderList; order != NULL; order = order->next)
    {
    if (hashLookup(dtfHash, order->name))
        {
	if (first)
	    first = FALSE;
	else
	    dyStringAppendC(dy, ',');
	dyStringAppend(dy, order->name);
	}
    }
hashFree(&dtfHash);
}

void makeDbOrderedCommaFieldList(struct sqlConnection *conn, 
	char *table, struct joinerDtf *dtfList, struct dyString *dy)
/* Assumes that dtfList all points to same table.  This will return 
 * a comma-separated field list in the same order as the fields are 
 * in database. */
{
char *split = chromTable(conn, table);
struct slName *fieldList = sqlListFields(conn, split);
makeOrderedCommaFieldList(fieldList, dtfList, dy);
slFreeList(&fieldList);
freez(&split);
}

static void makeCtOrderedCommaFieldList(struct joinerDtf *dtfList, 
	struct dyString *dy)
/* Make comma-separated field list in same order as fields are in
 * custom track. */
{
char *track = dtfList->table;
struct customTrack *ct = ctLookupName(track);
char *type = ct->dbTrackType;
struct slName *fieldList = NULL;
if (startsWithWord("makeItems", type) || 
        sameWord("bedDetail", type) || 
        sameWord("barChart", type) || 
        sameWord("interact", type) || 
        sameWord("pgSnp", type))
    {
    struct sqlConnection *conn = hAllocConn(CUSTOM_TRASH);
    fieldList = sqlListFields(conn, ct->dbTableName);
    hFreeConn(&conn);
    }
else
    {
    fieldList = getBedFields(15);
    }
makeOrderedCommaFieldList(fieldList, dtfList, dy);
slFreeList(&fieldList);
}

static void makeBigBedOrderedCommaFieldList(struct joinerDtf *dtfList,
	struct dyString *dy)
/* Make comma-separated field list in same order as fields are in
 * big bed. */
{
struct sqlConnection *conn = NULL;
if (!trackHubDatabase(database))
    conn = hAllocConn(dtfList->database);
struct slName *fieldList = bigBedGetFields(dtfList->table, conn);
makeOrderedCommaFieldList(fieldList, dtfList, dy);
slFreeList(&fieldList);
hFreeConn(&conn);
}

static void makeLongTabixOrderedCommaFieldList(struct joinerDtf *dtfList,
	struct dyString *dy)
/* Make comma-separated field list in same order as fields are in
 * long tabix file. */
{
struct slName *fieldList = getLongTabixFields();
makeOrderedCommaFieldList(fieldList, dtfList, dy);
slFreeList(&fieldList);
}

static void makeBamOrderedCommaFieldList(struct joinerDtf *dtfList,
	struct dyString *dy)
/* Make comma-separated field list in same order as fields are in
 * big bed. */
{
struct slName *fieldList = bamGetFields();
makeOrderedCommaFieldList(fieldList, dtfList, dy);
slFreeList(&fieldList);
}

static void makeVcfOrderedCommaFieldList(struct joinerDtf *dtfList,
	struct dyString *dy)
/* Make comma-separated field list in same order as fields are in
 * big bed. */
{
struct slName *fieldList = vcfGetFields();
makeOrderedCommaFieldList(fieldList, dtfList, dy);
slFreeList(&fieldList);
}

static void makeHicOrderedCommaFieldList(struct joinerDtf *dtfList,
	struct dyString *dy)
/* Make comma-separated field list in same order as fields are in
 * big bed. */
{
struct slName *fieldList = hicGetFields();
makeOrderedCommaFieldList(fieldList, dtfList, dy);
slFreeList(&fieldList);
}

struct tableJoiner
/* List of fields in a single table. */
    {
    struct tableJoiner *next;	/* Next in list. */
    char *database;		/* Database we're in.  Not alloced here. */
    char *table;		/* Table we're in.  Not alloced here. */
    struct joinerDtf *fieldList;	/* Fields. */
    struct slRef *keysOut;	/* Keys that connect to output.
                                 * Value is joinerPair. */
    boolean loaded;	/* If true is loaded. */
    };

void tableJoinerFree(struct tableJoiner **pTf)
/* Free up memory associated with tableJoiner. */
{
struct tableJoiner *tj = *pTf;
if (tj != NULL)
    {
    joinerDtfFreeList(&tj->fieldList);
    slFreeList(&tj->keysOut);
    freez(pTf);
    }
}

void tableJoinerFreeList(struct tableJoiner **pList)
/* Free a list of dynamically allocated tableJoiner's */
{
struct tableJoiner *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    tableJoinerFree(&el);
    }
*pList = NULL;
}

static void orderFieldsInTable(struct tableJoiner *tj)
/* Rearrange order of fields in tj->fieldList so that 
 * they are the same as the order in the database. */
{
struct sqlConnection *conn = hAllocConn(tj->database);
char *splitTable = chromTable(conn, tj->table);
struct hash *fieldHash = hashNew(0);
struct slName *field, *fieldList = sqlListFields(conn, splitTable);
struct joinerDtf *dtf, *newList = NULL;

/* Build up hash of field names. */
for (dtf = tj->fieldList; dtf != NULL; dtf = dtf->next)
    hashAdd(fieldHash, dtf->field, dtf);
hFreeConn(&conn);

/* Build up new list in correct order. */
for (field = fieldList; field != NULL; field = field->next)
    {
    dtf = hashFindVal(fieldHash, field->name);
    if (dtf != NULL)
        {
	slAddHead(&newList, dtf);
	}
    }
slFreeList(&fieldList);
slReverse(&newList);
tj->fieldList = newList;
freez(&splitTable);
}

static void addDtfListToBundle(struct hash *hash, struct tableJoiner **pList,
	struct joinerDtf *dtfList, boolean addField)
/* Add table to hash/list if haven't seen it already.  Optionally add
 * field to table. */
{
char fullName[256];
struct joinerDtf *dtf, *dupe;
struct tableJoiner *tj;
for (dtf = dtfList; dtf != NULL; dtf = dtf->next)
    {
    safef(fullName, sizeof(fullName), "%s.%s", dtf->database, dtf->table);
    tj = hashFindVal(hash, fullName);
    if (tj == NULL)
        {
	AllocVar(tj);
	hashAdd(hash, fullName, tj);
	tj->database = dtf->database;
	tj->table = dtf->table;
	slAddHead(pList, tj);
	}
    if (addField)
	{
	dupe = joinerDtfClone(dtf);
	slAddTail(&tj->fieldList, dupe);
	}
    }
}

static struct tableJoiner *bundleFieldsIntoTables(struct joinerDtf *resultFields,
	struct joinerDtf *filterTables)
/* Convert list of fields to list of tables.  */
{
struct hash *hash = newHash(0);
struct tableJoiner *tjList = NULL, *tj;

addDtfListToBundle(hash, &tjList, resultFields, TRUE);
addDtfListToBundle(hash, &tjList, filterTables, FALSE);
slReverse(&tjList);
for (tj = tjList; tj != NULL; tj = tj->next)
    orderFieldsInTable(tj);
return tjList;
}

static struct joinerDtf *tableToDtfs(struct tableJoiner *tjList)
/* Make dtfList from tjList, with empty field fields. */
{
struct joinerDtf *list = NULL, *dtf;
struct tableJoiner *tj;

for (tj = tjList; tj != NULL; tj = tj->next)
    {
    dtf = joinerDtfNew(tj->database, tj->table, NULL);
    slAddHead(&list, dtf);
    }
slReverse(&list);
return list;
}

static struct tableJoiner *findTable(struct tableJoiner *tjList, char *db, char *table)
/* Find table that matches. */
{
struct tableJoiner *tj;
char dbTableBuf[256];
dbOverrideFromTable(dbTableBuf, &db, &table);
for (tj = tjList; tj != NULL; tj = tj->next)
    {
    if (sameString(tj->database,db) && sameString(tj->table, table))
        return tj;
    }
return NULL;
}

int tableFieldsCmp(const void *va, const void *vb)
/* Compare two tableJoiner. */
{
const struct tableJoiner *a = *((struct tableJoiner **)va);
const struct tableJoiner *b = *((struct tableJoiner **)vb);
int diff;
diff = strcmp(a->database, b->database);
if (diff == 0)
    diff = strcmp(a->table, b->table);
return diff;
}

void orderTables(struct tableJoiner **pTfList, char *primaryDb, char *primaryTable)
/* Order table list so that primary table is first and the rest are
 * alphabetical.  This is the order they are listed in the UI.
 * (However we get them scrambled since the CGI variables are not
 * ordered.)*/
{
struct tableJoiner *tjList = *pTfList;
struct tableJoiner *primaryTf = findTable(tjList, primaryDb, primaryTable);
if (primaryTf != NULL)
    slRemoveEl(&tjList, primaryTf);
slSort(&tjList, tableFieldsCmp);
if (primaryTf != NULL)
    {
    slAddHead(&tjList, primaryTf);
    }
*pTfList = tjList;
}

boolean inKeysOut(struct tableJoiner *tj, struct joinerPair *route)
/* See if key implied by route is already in tableJoiner. */
{
struct slRef *ref;
for (ref = tj->keysOut; ref != NULL; ref = ref->next)
    {
    struct joinerPair *jp = ref->val;
    struct joinerDtf *a = jp->a;
    if (sameString(a->database, route->a->database)
     && sameString(a->table, route->a->table)
     && sameString(a->field, route->a->field))
         return TRUE;
    }
return FALSE;
}

void addOutKeys(struct hash *tableHash, struct joinerPair *routeList, 
	struct tableJoiner **pTfList)
/* Add output keys to tableJoiners.  These are in table hash.
 * We may also have to add some fieldLess tables to the mix,
 * which will get appended to *pTfList, and added to the hash
 * if need be. */
{
struct joinerPair *route;
struct tableJoiner *tj;
char fullName[256];

for (route = routeList; route != NULL; route = route->next)
    {
    struct joinerDtf *a = route->a;
    safef(fullName, sizeof(fullName), "%s.%s", a->database, a->table);
    tj = hashFindVal(tableHash, fullName);
    if (tj == NULL)
        {
	AllocVar(tj);
	tj->database = a->database;
	tj->table = a->table;
	slAddTail(pTfList, tj);
	hashAdd(tableHash, fullName, tj);
	}
    if (!inKeysOut(tj, route))
	refAdd(&tj->keysOut, route);
    }
}

void dyStringAddList(struct dyString *dy, char *s)
/* Add s to end of string.  If string is non-empty
 * first add comma.  This make it relatively easy to
 * build up comma-separates lists. */
{
if (dy->stringSize > 0)
    dyStringAppendC(dy, ',');
dyStringAppend(dy, s);
}

void tjLoadSome(struct region *regionList,
    struct joinedTables *joined, int fieldOffset, int keyOffset,
    char *idField, struct hash *idHash, 
    struct joinerField *jf,
    struct tableJoiner *tj, boolean isPositional, boolean isFirst)
/* Load up rows. */
{
struct region *region;
struct dyString *sqlFields = dyStringNew(0);
struct joinerDtf *dtf;
struct slRef *ref;
struct joinerPair *jp;
int fieldCount = 0, keyCount = 0;
int idFieldIx = -1;
struct sqlConnection *conn = hAllocConn(tj->database);
char *identifierFilter = NULL;
boolean needUpdateFilter = FALSE;
struct joinedRow *jr;

if (isFirst)
    identifierFilter = identifierWhereClause(idField, idHash);

// ignore returned filter from this call to filterClause:
filterClause(tj->database, tj->table, regionList->chrom, identifierFilter);

/* Record combined filter. */
// Show only the SQL filter built from filter page options, not identifierFilter,
// because identifierFilter can get enormous (like 126kB for 12,500 rsIDs).
char *filterNoIds = filterClause(tj->database, tj->table, regionList->chrom, NULL);
if (filterNoIds != NULL)
    {
    if (joined->filter == NULL)
	joined->filter = dyStringNew(0);
    else
	dyStringAppend(joined->filter, " AND ");
    dyStringAppend(joined->filter, filterNoIds);
    if (!isFirst)
	{
        needUpdateFilter = TRUE;
	for (jr = joined->rowList; jr != NULL; jr = jr->next)
	    jr->hitThisTable = FALSE;
	}
    }

/* Create field spec for sql - first fields user will see, and
 * second keys if any. */
for (dtf = tj->fieldList; dtf != NULL; dtf = dtf->next)
    {
    struct joinerDtf *dupe = joinerDtfClone(dtf);
    slAddTail(&joined->fieldList, dupe);
    dyStringAddList(sqlFields, dtf->field);
    ++fieldCount;
    }
for (ref = tj->keysOut; ref != NULL; ref = ref->next)
    {
    struct joinerDtf *dupe;
    jp = ref->val;
    dupe = joinerDtfClone(jp->a);
    slAddTail(&joined->keyList, dupe);
    dyStringAddList(sqlFields, jp->a->field);
    ++keyCount;
    }
if (idHash != NULL)
    {
    if (idField == NULL)
        internalErr();
    idFieldIx = fieldCount + keyCount;
    dyStringAddList(sqlFields, idField);
    }

for (region = regionList; region != NULL; region = region->next)
    {
    char **row;
    /* We free at end of loop so we get new one for each chromosome. */
    char *filter = filterClause(tj->database, tj->table, region->chrom, identifierFilter);
    struct sqlResult *sr = regionQuery(conn, tj->table, 
    	sqlFields->string, region, isPositional, filter);
    while (sr != NULL && (row = sqlNextRow(sr)) != NULL)
        {
	if (idFieldIx < 0)
	    {
	    if (jrRowAdd(joined, row, fieldCount, keyCount) == NULL)
	        break;
	    }
	else
	    {
	    char *id = row[idFieldIx];
	    if (isFirst)
		{
		if (hashLookup(idHash, id))
		    {
		    if (jrRowAdd(joined, row, fieldCount, keyCount) == NULL)
		        break;
		    }
		}
	    else
		{
                // chop id column by jf->separator if necessary
                int sepCount = 0;
                if (jf && isNotEmpty(jf->separator))
                    sepCount = countChars(id, jf->separator[0]);
                int wordCount = sepCount + 1;
                char *idWords[wordCount];
                if (wordCount > 1)
                    // There may or may not be a word past the final separator,
                    // So get the real wordCount here:
                    wordCount = chopString(id, jf->separator, idWords, ArraySize(idWords));
                else
                    idWords[0] = id;
                int i;
                for (i = 0;  i < wordCount;  i++)
                    {
                    struct slName *chopBefore = jf ? jf->chopBefore : NULL;
                    struct slName *chopAfter = jf ? jf->chopAfter : NULL;
                    idWords[i] = chopKey(chopBefore, chopAfter, trimSpaces(idWords[i]));
                    struct hashEl *bucket;
                    for (bucket = hashLookup(idHash, idWords[i]); bucket != NULL;
                         bucket = hashLookupNext(bucket))
                        {
                        jr = bucket->val;
                        jr->hitThisTable = TRUE;
                        jrRowExpand(joined, jr, row, 
                                    fieldOffset, fieldCount, keyOffset, keyCount);
                        }
                    }
		}
	    }
	}
    sqlFreeResult(&sr);
    freez(&filter);
    if (!isPositional)
        break;
    }
if (isFirst)
    slReverse(&joined->rowList);
if (needUpdateFilter)
    {
    for (jr = joined->rowList; jr != NULL; jr = jr->next)
	{
	jr->passedFilter &= jr->hitThisTable;
	}
    }
tj->loaded = TRUE;
hFreeConn(&conn);
}
	
struct joinedTables *tjLoadFirst(struct region *regionList,
	struct tableJoiner *tj, int totalFieldCount,
	int totalKeyCount, int maxRowCount)
/* Load up first table in series of joined tables.  This allocates
 * field and key arrays big enough for all. */
{
struct joinedTables *joined = joinedTablesNew(totalFieldCount, 
	totalKeyCount, maxRowCount);
struct hash *idHash = NULL;
struct hTableInfo *hti = getHtiOnDb(tj->database, tj->table);
char *idField = getIdField(tj->database, curTrack, tj->table, hti);
if (idField != NULL)
    idHash = identifierHash(tj->database, tj->table);
tjLoadSome(regionList, joined, 0, 0, 
	idField, idHash, NULL, tj,
	isPositional(tj->database, tj->table), TRUE);
hashFree(&idHash);
return joined;
}

int findDtfIndex(struct joinerDtf *list, struct joinerDtf *dtf)
/* Find dtf on list. */
{
struct joinerDtf *el;
int ix;
for (el = list, ix=0; el != NULL; el = el->next, ++ix)
    {
    if (sameString(el->database, dtf->database)
     && sameString(el->table, dtf->table)
     && sameString(el->field, dtf->field))
        return ix;
    }
return -1;
}

struct hash *hashKeyField(struct joinedTables *joined, int keyIx,
	struct joinerField *jf)
/* Make a hash based on key field. */
{
int hashSize = digitsBaseTwo(joined->rowCount);
struct hash *hash = NULL;
struct joinedRow *jr;

if (hashSize > 20)
    hashSize = 20;
hash = newHash(hashSize);
for (jr = joined->rowList; jr != NULL; jr = jr->next)
    {
    struct slName *key;
    for (key = jr->keys[keyIx]; key != NULL; key = key->next)
	{
	if (jf->separator == NULL)
	    {
	    char *s = chopKey(jf->chopBefore, jf->chopAfter, trimSpaces(key->name));
	    if (s[0] != 0)
		hashAdd(hash, s, jr);
	    }
	else
	    {
	    char *s = key->name, *e;
	    char sep = jf->separator[0];
	    while (s != NULL && s[0] != 0)
	        {
		e = strchr(s, sep);
		if (e != NULL)
		    *e++ = 0;
		s = chopKey(jf->chopBefore, jf->chopAfter, s);
		if (s[0] != 0)
		    hashAdd(hash, s, jr);
		s = e;
		}
	    }
	}
    }
return hash;
}

struct tableJoiner *findTableJoiner(struct tableJoiner *list, 
    char *database, char *table)
/* Find tableJoiner for given database/table. */
{
struct tableJoiner *el;
for (el = list; el != NULL; el = el->next)
    {
    if (sameString(el->database, database) && sameString(el->table, table))
        return el;
    }
return NULL;
}

static struct joinerField *findJoinerField(struct joinerSet *js, 
	struct joinerDtf *dtf)
/* Find field in set if any that matches dtf */
{
struct slRef *chain = joinerSetInheritanceChain(js), *link;
struct joinerField *jf, *ret = NULL;
for (link = chain; link != NULL; link = link->next)
    {
    js = link->val;
    for (jf = js->fieldList; jf != NULL; jf = jf->next)
	 {
	 if (sameString(dtf->table, jf->table) && sameString(dtf->field, jf->field))
	     {
	     if (slNameInList(jf->dbList, dtf->database))
		 {
		 ret = jf;
		 break;
		 }
	     }
	 }
    if (ret != NULL)
        break;
    }
slFreeList(&chain);
return ret;
}

static struct joinedTables *joinedTablesCreate( struct joiner *joiner, 
	char *primaryDb, char *primaryTable,
	struct joinerDtf *fieldList, struct joinerDtf *filterTables,
	int maxRowCount, struct region *regionList)
/* Create joinedTables structure from fields. */
{
struct tableJoiner *tj, *tjList = bundleFieldsIntoTables(fieldList, filterTables);
struct joinerPair *routeList = NULL, *route;
struct joinedTables *joined = NULL;
struct hash *tableHash = newHash(8);
int totalKeyCount = 0, totalFieldCount = 0;
int curKeyCount = 0, curFieldCount = 0;
struct joinerDtf *tableDtfs;

for (tj = tjList; tj != NULL; tj = tj->next)
    {
    char buf[256];
    safef(buf, sizeof(buf), "%s.%s", tj->database, tj->table);
    hashAdd(tableHash, buf, tj);
    }
orderTables(&tjList, primaryDb, primaryTable);
tableDtfs = tableToDtfs(tjList);
routeList = joinerFindRouteThroughAll(joiner, tableDtfs);
if (routeList == NULL)
    errAbort("Can't find route from %s to %s via all.joiner", primaryTable, tjList->next->table);
addOutKeys(tableHash, routeList, &tjList);

/* If first table is non-positional then it will lead to a lot
 * of n/a's in later fields unless we treat the genome-wide. */
if (!isPositional(tjList->database, tjList->table))
    regionList = getRegionsFullGenome();
/* Count up total fields and keys. */
for (tj = tjList; tj != NULL; tj = tj->next)
    {
    totalKeyCount += slCount(tj->keysOut);
    totalFieldCount += slCount(tj->fieldList);
    }

/* Do first table.  This one uses identifier hash if any. */
    {
    joined = tjLoadFirst(regionList,
	    tjList, totalFieldCount, totalKeyCount, maxRowCount);
    curKeyCount = slCount(tjList->keysOut);
    curFieldCount = slCount(tjList->fieldList);
    }

/* Follow routing list for rest. */
if (!sameString(tjList->database, routeList->a->database))
    internalErr();
if (!sameString(tjList->table, routeList->a->table))
    internalErr();
for (route = routeList; route != NULL; route = route->next)
    {
    struct tableJoiner *tj = findTableJoiner(tjList, 
	    route->b->database, route->b->table);
    struct joinerField *jfA = NULL, *jfB = NULL;
    if (tj == NULL)
	internalErr();
    jfA = findJoinerField(route->identifier, route->a);
    if (jfA == NULL)
	{
	internalErr();
	}
    jfB = findJoinerField(route->identifier, route->b);
    if (jfB == NULL)
	internalErr();
    if (!tj->loaded)
	{
	int keyIx;
	struct hash *keyHash = NULL;
	keyIx = findDtfIndex(joined->keyList, route->a);
	if (keyIx < 0)
	    internalErr();
	keyHash = hashKeyField(joined, keyIx, jfA);
	tjLoadSome(regionList, joined, curFieldCount, curKeyCount,
	    route->b->field, keyHash, 
	    jfB,
	    tj, isPositional(tj->database, tj->table),  FALSE);
	curKeyCount += slCount(tj->keysOut);
	curFieldCount += slCount(tj->fieldList);
	hashFree(&keyHash);
	}
    }
joinerDtfFreeList(&tableDtfs);
hashFree(&tableHash);
tableJoinerFreeList(&tjList);
return joined;
}

static boolean allSameTable(struct joinerDtf *fieldList, struct joinerDtf *filterTables)
/* Return TRUE if all db/tables in fieldList and filterTables are same. */
{
struct joinerDtf *first = fieldList, *dtf;

if (first == NULL) internalErr();
for (dtf = first->next; dtf != NULL; dtf = dtf->next)
    if (!joinerDtfSameTable(first, dtf))
        return FALSE;
for (dtf = filterTables; dtf != NULL; dtf = dtf->next)
    if (!joinerDtfSameTable(first, dtf))
        return FALSE;
return TRUE;
}

static boolean isPrimary(char *db, char *table)
/* Return TRUE if db.table is the primary table in this query. */
{
if (sameString(db, database) && sameString(table, curTable))
    return TRUE;
else
    {
    char dbTable[256];
    safef(dbTable, sizeof(dbTable), "%s.%s", db, table);
    if (sameString(dbTable, curTable))
	return TRUE;
    }
return FALSE;
}

boolean joinRequired(char *db, char *table,
		     struct slName *fieldList,
		     struct joinerDtf **pDtfList,
		     struct joinerDtf **pFilterTables)
/* Given a list of db.table.field values, determine whether a joining query 
 * will be required.  Consider not only the tables to be queried but also 
 * the tables to be filtered (which can be a different set).  If db.table 
 * is not the primary db.table in the search, no tables are to be filtered.
 * If pDtfList is not null, make it point to the list of joinerDtfs 
 * derived from fieldList. 
 * If pFilterTables is not null, make it point to the list of joinerDtfs 
 * containing the dbs and tables (but not fields) of tables to be filtered. */
{
struct joinerDtf *dtfList = fieldsToDtfs(fieldList);
struct joinerDtf *filterTables = filteringTables();
boolean ret = FALSE;

/* Ignore filterTables() if this query is not on the primary table. */
if (! isPrimary(db, table))
    joinerDtfFreeList(&filterTables);

ret = (! allSameTable(dtfList, filterTables));

if (pDtfList != NULL)
    *pDtfList = dtfList;
else
    joinerDtfFreeList(&dtfList);
if (pFilterTables != NULL)
    *pFilterTables = filterTables;
else
    joinerDtfFreeList(&filterTables);
return ret;
}


void sepOutSelectedFields(
	char *primaryDb,		/* The primary database. */
	char *primaryTable, 		/* The primary table. */
	FILE *f,			/* file for output, null for stdout */
	struct slName *fieldList,	/* List of db.table.field */
    char outSep)               /* The separator for the output */
/* Do tab-separated output on selected fields, which may
 * or may not include multiple tables. */
{
struct joinerDtf *dtfList = NULL;
struct joinerDtf *filterTables = NULL;
boolean doJoin = joinRequired(primaryDb, primaryTable,
			      fieldList, &dtfList, &filterTables);

boolean hasIdentifiers = (identifierFileName() != NULL);

char *regionType = cartUsualString(cart, hgtaRegionType, "genome");
boolean hasRegions = sameString(regionType, hgtaRegionTypeRange)
		 ||  sameString(regionType, hgtaRegionTypeEncode)
		 || (sameString(regionType, hgtaRegionTypeUserRegions) && (userRegionsFileName() != NULL));

if (hasIdentifiers || hasRegions)
    {
    boolean hasTable = FALSE;
    char *dtfDb, *dtfTable;
    char split[1024];
    /* Choosing the All Tables group and then choosing a db like hgFixed or go 
     * causes it to inserts a $db. in front of the table name while leaving the primaryDb as the assembly. 
     * In effect, the table field is sometimes overloaded to carry this extra database for all tables support. */
    char *sep = strchr(primaryTable, '.');
    if (!isHubTrack(primaryTable) && sep)
	{
	safecpy(split, sizeof split, primaryTable);
	sep = strchr(split, '.');
	*sep++ = 0;
	dtfDb = split;
	dtfTable = sep;
	}
    else
	{
	dtfDb = primaryDb;
	dtfTable = primaryTable;
	}
    /* see if we already have the primary table in output fields or filter */
    struct joinerDtf *temp;
    for (temp = dtfList; temp; temp = temp->next)
	if (sameString(temp->database, dtfDb) && sameString(temp->table, dtfTable))
	    hasTable = TRUE;
    for (temp = filterTables; temp; temp = temp->next)
	if (sameString(temp->database, dtfDb) && sameString(temp->table, dtfTable))
	    hasTable = TRUE;
    /* if primary table is not in output or filter, 
     *  add it to the filterTables list to trigger joining and identifier and/or region filtering */
    if (!hasTable)
	{	    
	struct joinerDtf *dtf;
	AllocVar(dtf);
	dtf->database = cloneString(dtfDb);
	dtf->table = cloneString(dtfTable);
	slAddTail(&filterTables, dtf);
	doJoin = TRUE;
	}
    }


if (! doJoin)
    {
    struct sqlConnection *conn = NULL;

    if (!trackHubDatabase(database))
	conn = hAllocConn(dtfList->database);
    struct dyString *dy = dyStringNew(0);
    
    if (isBigBed(database, dtfList->table, NULL, ctLookupName))
	makeBigBedOrderedCommaFieldList(dtfList, dy);
    else if (isLongTabixTable(dtfList->table))
        makeLongTabixOrderedCommaFieldList(dtfList, dy);
    else if (isBamTable(dtfList->table))
        makeBamOrderedCommaFieldList(dtfList, dy);
    else if (isVcfTable(dtfList->table, NULL))
        makeVcfOrderedCommaFieldList(dtfList, dy);
    else if (isHicTable(dtfList->table))
        makeHicOrderedCommaFieldList(dtfList, dy);
    else if (isCustomTrack(dtfList->table))
        makeCtOrderedCommaFieldList(dtfList, dy);
    else
	makeDbOrderedCommaFieldList(conn, dtfList->table, dtfList, dy);
    doTabOutTable(dtfList->database, dtfList->table, f, conn, dy->string, outSep);
    hFreeConn(&conn);
    }
else
    {
    struct joiner *joiner = allJoiner;
    struct joinedTables *joined = joinedTablesCreate(joiner, 
    	primaryDb, primaryTable, dtfList, filterTables, 1000000, getRegions());
    joinedTablesSepOutFile(joined, f, outSep);
    joinedTablesFree(&joined);
    }
joinerDtfFreeList(&dtfList);
joinerDtfFreeList(&filterTables);
}


static struct slName *getBedFieldSlNameList(struct hTableInfo *hti,
					    char *db, char *table)
/* Return the bed-compat field list for the given table, as 
 * slName list of "$db.$table.$field". */
{
struct slName *snList = NULL, *sn = NULL;
int fieldCount = 0;
char *fields = NULL;
char *words[16];
char dtf[256];
int i;
if (hti == NULL)
    errAbort("Can't find table info for table %s.%s", db, table);
bedSqlFieldsExceptForChrom(hti, &fieldCount, &fields);
/* Update our notion of fieldCount -- the chrom field is omitted, and 
 * (if applicable) the reserved field is omitted too: */
fieldCount = chopCommas(fields, words);
for (i=fieldCount-1;  i >= 0;  i--)
    {
    // Skip placeholder field names:
    if (sameString(words[i], "0") || sameString(words[i], "'.'"))
	continue;
    safef(dtf, sizeof(dtf), "%s.%s.%s", db, table, words[i]);
    sn = slNameNew(dtf);
    slAddHead(&snList, sn);
    }
safef(dtf, sizeof(dtf), "%s.%s.%s", db, table, hti->chromField);
sn = slNameNew(dtf);
slAddHead(&snList, sn);
freez(&fields);
return snList;
}


static int *getBedFieldIndices(struct joinedTables *joined,
			       struct hTableInfo *hti)
/* Go through the list of joined fields and determine how they map to bed 
 * fields. */
{
int *indices = needMem(sizeof(int) * 12);
struct joinerDtf *field = NULL;
int i;
for (i=0;  i < 12;  i++)
    {
    indices[i] = -1;
    }
i = 0;
for (field = joined->fieldList;  field != NULL;  field = field->next)
    {
    if (sameString(field->field, hti->chromField))
	indices[0] = i;
    else if (sameString(field->field, hti->startField))
	indices[1] = i;
    else if (sameString(field->field, hti->endField))
	indices[2] = i;
    else if (sameString(field->field, hti->nameField))
	indices[3] = i;
    else if (sameString(field->field, hti->scoreField))
	indices[4] = i;
    else if (sameString(field->field, hti->strandField))
	indices[5] = i;
    else if (sameString(field->field, hti->cdsStartField))
	indices[6] = i;
    else if (sameString(field->field, hti->cdsEndField))
	indices[7] = i;
    /* Reserved is skipped!  So indices after this point are 1 smaller. */
    else if (sameString(field->field, hti->countField))
	indices[8] = i;
    else if (sameString(field->field, hti->endsSizesField))
	indices[9] = i;
    else if (sameString(field->field, hti->startsField))
	indices[10] = i;
    i++;
    }
return indices;
}

static struct bed *joinedTablesToBed(struct joinedTables *joined,
				     struct hTableInfo *hti, int bedFieldCount,
				     struct lm *lm)
/* Make a bedList from the joining query results. */
{
struct joinedRow *jr;
struct bed *bedList = NULL;
boolean isPsl = sameString("tStarts", hti->startsField);
boolean isGenePred = sameString("exonEnds", hti->endsSizesField);
boolean isBedWithBlocks = ((sameString("chromStarts", hti->startsField) ||
			    sameString("blockStarts", hti->startsField))
			   && sameString("blockSizes", hti->endsSizesField));
boolean pslKnowIfProtein, pslIsProtein;
int *bedFieldIndices = getBedFieldIndices(joined, hti);
/* The reserved field is skipped, so adjust count for row creation: */
int adjBedFieldCount = (bedFieldCount > 8) ? bedFieldCount-1 : bedFieldCount;
char *chrom = NULL;

for (jr = joined->rowList; jr != NULL; jr = jr->next)
    {
    if (jr->passedFilter)
	{
	int i;
	struct bed *bed = NULL;
	char *row[16];
	for (i=0;  i < adjBedFieldCount;  i++)
	    {
	    if (bedFieldIndices[i] < 0)
		row[i] = "0";
	    else
		row[i] = jr->fields[bedFieldIndices[i]]->name;
	    }
	if ((chrom == NULL) || (! sameString(chrom, row[0])))
	    chrom = lmCloneString(lm, row[0]);
	bed = bedFromRow(chrom, row+1, bedFieldCount,
			 isPsl, isGenePred, isBedWithBlocks,
			 &pslKnowIfProtein, &pslIsProtein, lm);
	slAddHead(&bedList, bed);
	}
    }
freeMem(bedFieldIndices);
slReverse(&bedList);
return bedList;
}

struct bed *dbGetFilteredBedsOnRegions(struct sqlConnection *conn, 
	char *db, char *dbVarName, char *table, char *tableVarName,
	struct region *regionList, struct lm *lm, 
	int *retFieldCount)
/* Get list of beds from database, in all regions, that pass filtering. */
{
/* A joining query may be required if the filter incorporates linked tables. */
struct hTableInfo *hti = getHti(db, table, conn);
struct slName *fieldList = getBedFieldSlNameList(hti, db, table);
struct joinerDtf *dtfList = NULL;
struct joinerDtf *filterTables = NULL;
boolean doJoin = joinRequired(db, table,
			      fieldList, &dtfList, &filterTables);
struct region *region;
struct bed *bedList = NULL;
char *idField = getIdField(db, curTrack, table, hti);
struct hash *idHash = identifierHash(db, table);

if (! doJoin)
    {
    for (region = regionList; region != NULL; region = region->next)
	{
	char *identifierFilter = identifierWhereClause(idField, idHash);
	char *filter = filterClause(dbVarName, tableVarName, region->chrom, identifierFilter);
	struct bed *bedListRegion = getRegionAsMergedBed(dbVarName, tableVarName,
				    region, filter, idHash, lm, retFieldCount);
	struct bed *bed, *nextBed;
	for (bed = bedListRegion; bed != NULL; bed = nextBed)
	    {
	    nextBed = bed->next;
	    slAddHead(&bedList, bed);
	    }
	freez(&filter);
	}
    slReverse(&bedList);
    }
else
    {
    struct joiner *joiner = allJoiner;
    struct joinedTables *joined = joinedTablesCreate(joiner, 
    	db, table, dtfList, filterTables, 1000000, regionList);
    int bedFieldCount = hTableInfoBedFieldCount(hti);
    if (retFieldCount != NULL)
	*retFieldCount = bedFieldCount;
    bedList = joinedTablesToBed(joined, hti, bedFieldCount, lm);
    joinedTablesFree(&joined);
    }
joinerDtfFreeList(&dtfList);
joinerDtfFreeList(&filterTables);
hashFree(&idHash);
return bedList;
}

