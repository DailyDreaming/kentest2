/* identifiers - handle identifier lists: uploading, pasting,
 * and restricting to just things on the list. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "cheapcgi.h"
#include "cart.h"
#include "jksql.h"
#include "trackDb.h"
#include "portable.h"
#include "hgTables.h"
#include "trashDir.h"
#include "web.h"
#include "wikiTrack.h"
#include "htmshell.h"



static boolean forCurTable()
/* Return TRUE if cart Identifier stuff is for curTable. */
{
char *identifierDb = cartOptionalString(cart, hgtaIdentifierDb);
char *identifierTable = cartOptionalString(cart, hgtaIdentifierTable);

return (identifierDb && identifierTable &&
	sameString(identifierDb, database) &&
	(sameString(identifierTable, curTable) ||
	 sameString(connectingTableForTrack(identifierTable), curTable)));
}

static void getXrefInfo(struct sqlConnection *conn,
			char **retXrefTable, char **retIdField,
			char **retAliasField)
/* See if curTrack specifies an xref/alias table for lookup of IDs. */
{
struct trackDb *tdb  = hTrackDbForTrack(database, curTable);

if (tdb == NULL)
    tdb = curTrack;

char *xrefSpec = tdb ? trackDbSettingClosestToHomeOrDefault(tdb, "idXref",NULL) : NULL;
char *xrefTable = NULL, *idField = NULL, *aliasField = NULL;
if (xrefSpec != NULL)
    {
    char *words[3];
    chopLine(cloneString(xrefSpec), words);
    if (isEmpty(words[2]))
	errAbort("trackDb error: track %s, setting idXref must be followed "
		 "by three words (xrefTable, idField, aliasField).",
		 curTrack->track);
    xrefTable = words[0];
    idField = words[1];
    aliasField = words[2];
    if (!sqlTableExists(conn, xrefTable) ||
	sqlFieldIndex(conn, xrefTable, idField) < 0 ||
	sqlFieldIndex(conn, xrefTable, aliasField) < 0)
	xrefTable = idField = aliasField = NULL;
    }
if (retXrefTable != NULL)
    *retXrefTable = xrefTable;
if (retIdField != NULL)
    *retIdField = idField;
if (retAliasField != NULL)
    *retAliasField = aliasField;
}

static struct slName *getExamples(char *db, struct sqlConnection *conn,
				  char *table, char *field, int count)
/* Return a list of several example values of table.field. */
{
boolean isTabix = FALSE;
if (isBamTable(table))
    {
    assert(sameString(field, "qName"));
    return randomBamIds(table, conn, count);
    }
else if (isBigBed(db, table, curTrack, ctLookupName))
    {
    assert(sameString(field, "name"));
    return randomBigBedIds(table, conn, count);
    }
else if (isVcfTable(table, &isTabix))
    {
    assert(sameString(field, "id"));
    return randomVcfIds(table, conn, count, isTabix);
    }
else
    {
    char fullTable[HDB_MAX_TABLE_STRING];
    char *c = strchr(table, '.');
    if (c || ! hFindSplitTable(database, NULL, table, fullTable, sizeof fullTable, NULL))
	safecpy(fullTable, sizeof(fullTable), table);
    return sqlRandomSampleConn(conn, fullTable, field, count);
    }
}

static void explainIdentifiers(char *db, struct sqlConnection *conn, char *idField)
/* Tell the user what field(s) they may paste/upload values for, and give
 * some examples. */
{
char *xrefTable = NULL, *xrefIdField = NULL, *aliasField = NULL;
getXrefInfo(conn, &xrefTable, &xrefIdField, &aliasField);
hPrintf("The items must be values of the <B>%s</B> field of the currently "
	"selected table, <B>%s</B>",
	idField, curTable);
if (aliasField != NULL)
    {
    if (sameString(curTable, xrefTable))
	{
	if (!sameString(idField, aliasField))
	    hPrintf(", or the <B>%s</B> field.\n", aliasField);
	else
	    hPrintf(".\n");
	}
    else
	hPrintf(", or the <B>%s</B> field of the alias table <B>%s</B>.\n",
		aliasField, xrefTable);
    }
else
    hPrintf(".\n");
hPrintf("(The \"describe table schema\" button shows more information about "
	"the table fields.)\n");

// on a browserbox, db is on the UCSC server, so cannot select into db, even if temporary
if (!isCustomTrack(curTable) && !hIsBrowserbox() && (conn == NULL || sqlCanCreateTemp(conn)))
    {
    struct slName *exampleList = NULL, *ex;
    hPrintf("Some example values:<BR>\n");
    exampleList = getExamples(db, conn, curTable, idField,
			      aliasField != NULL ? 3 : 5);
    for (ex = exampleList;  ex != NULL;  ex = ex->next)
	{
	char *tmp = htmlEncode(ex->name);
	hPrintf("<TT>%s</TT><BR>\n", tmp);
	freeMem(tmp);
	}

    if (aliasField != NULL)
	{
	char tmpTable[512];
	char query[2048];
	// do not use any db. prefix on curTable for name
	char *plainXrefTable = strrchr(xrefTable, '.');  
	if (plainXrefTable)
	    plainXrefTable++;
	else
	    plainXrefTable = xrefTable;
	char *plainCurTable = strrchr(curTable, '.');  
	if (plainCurTable)
	    plainCurTable++;
	else
	    plainCurTable = curTable;
	safef(tmpTable, sizeof(tmpTable), "hgTemp.tmp%s%s", plainCurTable, plainXrefTable);
	if (differentString(xrefTable, curTable))
	    sqlSafef(query, sizeof(query),
		  "create temporary table %s select %s.%s as %s from %s,%s "
		  "where %s.%s = %s.%s and %s.%s != %s.%s limit 100000",
		  tmpTable, xrefTable, aliasField, aliasField, xrefTable, curTable,
		  xrefTable, xrefIdField, curTable, idField,
		  xrefTable, xrefIdField, xrefTable, aliasField);
	else
	    sqlSafef(query, sizeof(query),
		  "create temporary table %s select %s from %s "
		  "where %s != %s limit 100000",
		  tmpTable, aliasField, xrefTable, aliasField, xrefIdField);
	sqlUpdate(conn, query);

	exampleList = getExamples(db, conn, tmpTable, aliasField, 3);
	for (ex = exampleList;  ex != NULL;  ex = ex->next)
	    hPrintf("<TT>%s</TT><BR>\n", ex->name);
	}
    hPrintf("\n");
    }
}

void doPasteIdentifiers(struct sqlConnection *conn)
/* Respond to paste identifiers button. */
{
struct sqlConnection *alternateConn = conn;
char *actualDb = database;
if (sameWord(curTable, WIKI_TRACK_TABLE))
    {
    alternateConn = wikiConnect();
    actualDb = wikiDbName();
    }

char *oldPasted = forCurTable() ?
    cartUsualString(cart, hgtaPastedIdentifiers, "") : "";
struct hTableInfo *hti = maybeGetHti(actualDb, curTable, conn);
char *idField = getIdField(actualDb, curTrack, curTable, hti);
htmlOpen("Paste In Identifiers for %s", curTableLabel());
if (idField == NULL)
    errAbort("Sorry, I can't tell which field of table %s to treat as the "
	     "identifier field.", curTable);
hPrintf("<FORM ACTION=\"%s\" METHOD=%s>\n", getScriptName(),
	cartUsualString(cart, "formMethod", "POST"));
cartSaveSession(cart);
hPrintf("Please paste in the identifiers you want to include.\n");
if (sqlDatabaseExists("hgTemp"))
    explainIdentifiers(actualDb, alternateConn, idField);
else
    warn("No hgTemp database found for temporary tables.<br>Please see https://genome.ucsc.edu/goldenPath/help/mirrorManual.html#mysql-setup for more information.");
hPrintf("<BR>\n");
cgiMakeTextArea(hgtaPastedIdentifiers, oldPasted, 10, 70);
hPrintf("<BR>\n");
cgiMakeButton(hgtaDoPastedIdentifiers, "submit");
hPrintf(" ");
cgiMakeButton(hgtaDoClearPasteIdentifierText, "clear");
hPrintf(" ");
cgiMakeButton(hgtaDoMainPage, "cancel");
hPrintf("</FORM>");
cgiDown(0.9);
htmlClose();
if (sameWord(curTable, WIKI_TRACK_TABLE))
    wikiDisconnect(&alternateConn);
}

void doUploadIdentifiers(struct sqlConnection *conn)
/* Respond to upload identifiers button. */
{
struct hTableInfo *hti = maybeGetHti(database, curTable, conn);
char *idField = getIdField(database, curTrack, curTable, hti);
htmlOpen("Upload Identifiers for %s", curTableLabel());
if (idField == NULL)
    errAbort("Sorry, I can't tell which field of table %s to treat as the "
	     "identifier field.", curTable);
hPrintf("<FORM ACTION=\"%s\" METHOD=POST ENCTYPE=\"multipart/form-data\">\n",
	getScriptName());
cartSaveSession(cart);
hPrintf("Please enter the name of a file from your computer that contains a ");
hPrintf("space, tab, or ");
hPrintf("line separated list of the items you want to include.\n");
explainIdentifiers(database, conn, idField);
hPrintf("<BR>\n");
hPrintf("<INPUT TYPE=FILE NAME=\"%s\"> ", hgtaPastedIdentifiers);
hPrintf("<BR>\n");
cgiMakeButton(hgtaDoPastedIdentifiers, "submit");
hPrintf(" ");
cgiMakeButton(hgtaDoMainPage, "cancel");
hPrintf("</FORM>");
cgiDown(0.9);
htmlClose();
}

static void addPrimaryIdsToHash(struct sqlConnection *conn, struct hash *hash,
				char *idField, struct slName *tableList,
				struct lm *lm, char *extraWhere)
/* For each table in tableList, query all idField values and add to hash,
 * id -> uppercased id for case-insensitive matching. */
{
struct slName *table;
struct sqlResult *sr;
char **row;
struct dyString *query = dyStringNew(0);
for (table = tableList;  table != NULL;  table = table->next)
    {
    dyStringClear(query);
    sqlDyStringPrintf(query, "select %s from %s", idField, table->name);
    if (extraWhere != NULL)
	dyStringPrintf(query, " where %s", extraWhere);
    sr = sqlGetResult(conn, query->string);
    while ((row = sqlNextRow(sr)) != NULL)
	{
	if (isNotEmpty(row[0]))
	    {
	    char *origCase = lmCloneString(lm, row[0]);
	    touppers(row[0]);
	    hashAdd(hash, row[0], origCase);
	    }
	}
    sqlFreeResult(&sr);
    }
}

static void addXrefIdsToHash(struct sqlConnection *conn, struct hash *hash,
			     char *idField, char *xrefTable, char *xrefIdField,
			     char *aliasField, struct lm *lm, char *extraWhere)
/* Query all id-alias pairs from xrefTable (where id actually appears
 * in curTable) and hash alias -> id.  Convert alias to upper case for
 * case-insensitive matching.
 * Ignore self (alias = id) mappings -- we already got those above. */
{
struct sqlResult *sr;
char **row;
struct dyString *query = dyStringNew(0);
if (sameString(xrefTable, curTable))
    sqlDyStringPrintf(query, "select %s,%s from %s", aliasField, xrefIdField, xrefTable);
else
    /* Get only the aliases for items actually in curTable.idField: */
    sqlDyStringPrintf(query,
	  "select %s.%s,%s.%s from %s,%s where %s.%s = %s.%s",
	  xrefTable, aliasField, xrefTable, xrefIdField,
	  xrefTable, curTable,
	  xrefTable, xrefIdField, curTable, idField);
if (extraWhere != NULL)
    // extraWhere begins w/ID field of curTable=xrefTable.  Skip that field name and
    // use "xrefTable.aliasField" with the IN (...) condition that follows:
    sqlDyStringPrintf(query, " %s %s.%s %-s",
		   (sameString(xrefTable, curTable) ? "where" : "and"),
		   xrefTable, aliasField, skipToSpaces(extraWhere));
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    if (sameString(row[0], row[1]))
	continue;
    touppers(row[0]);
    hashAdd(hash, row[0], lmCloneString(lm, row[1]));
    }
sqlFreeResult(&sr);
}

static struct hash *getAllPossibleIds(struct sqlConnection *conn,
				      struct lm *lm, char *idField, char *extraWhere)
/* If curTable is a custom track or bigFile, return NULL.  Otherwise,
 * make a hash of all identifiers in curTable (and alias tables if specified)
 * so that we can check the validity of pasted/uploaded identifiers. */
{
// this assumes that there is a knownGene table even if the track is a bigGenePred
if (isCustomTrack(curTable) || isLongTabixTable(curTable) || isBamTable(curTable) || isVcfTable(curTable, NULL) ||
    (differentString("knownGene", curTable) && isBigBed(database, curTable, curTrack, ctLookupName)))
    return NULL;

struct hash *matchHash = hashNew(20);
struct slName *tableList;
char *xrefTable = NULL, *xrefIdField = NULL, *aliasField = NULL;
struct sqlConnection *alternateConn = conn;

if (sameWord(curTable, WIKI_TRACK_TABLE))
    alternateConn = wikiConnect();

if (sameWord(curTable, WIKI_TRACK_TABLE))
    tableList = slNameNew(WIKI_TRACK_TABLE);
else if (strchr(curTable, '.'))
    tableList = slNameNew(curTable);
else
    tableList = hSplitTableNames(database, curTable);
if (idField != NULL)
    addPrimaryIdsToHash(alternateConn, matchHash, idField, tableList, lm, extraWhere);
getXrefInfo(alternateConn, &xrefTable, &xrefIdField, &aliasField);
if (xrefTable != NULL)
    {
    addXrefIdsToHash(alternateConn, matchHash, idField,
		     xrefTable, xrefIdField, aliasField, lm, extraWhere);
    }
if (sameWord(curTable, WIKI_TRACK_TABLE))
    wikiDisconnect(&alternateConn);
return matchHash;
}

static char *slNameToInExpression(char *field, struct slName *allTerms)
/* Given an slName list, return a SQL "field IN ('term1', 'term2', ...)" expression
 * to be used in a WHERE clause. */
{
struct dyString *dy = dyStringNew(0);
sqlDyStringPrintfFrag(dy, "%s in (", field);
boolean first = TRUE;
struct slName *term;
for (term = allTerms;  term != NULL;  term = term->next)
    {
    if (first)
	first = FALSE;
    else
	dyStringAppend(dy, ", ");
    sqlDyStringPrintf(dy, "'%s'", term->name);
    }
dyStringAppend(dy, ")");
return dyStringCannibalize(&dy);
}

#define MAX_IDTEXT (64 * 1024)
#define DEFAULT_MAX_IDS_IN_WHERE 10000


void doPastedIdentifiers(struct sqlConnection *conn)
/* Process submit in paste identifiers page. */
{
char *idText = trimSpaces(cartString(cart, hgtaPastedIdentifiers));
htmlOpen("Table Browser (Input Identifiers)");
if (isNotEmpty(idText))
    {
    /* Write terms to temp file, checking whether they have matches, and
     * save temp file name. */
    boolean saveIdText = (strlen(idText) < MAX_IDTEXT);
    char *idTextForLf = saveIdText ? cloneString(idText) : idText;
    struct lineFile *lf = lineFileOnString("idText", TRUE, idTextForLf);
    char *line, *word;
    struct tempName tn;
    FILE *f;
    int totalTerms = 0, foundTerms = 0;
    struct slName* missingTerms = NULL;
    struct dyString *exampleMissingIds = dyStringNew(256);
    char *actualDb = database;
    if (sameWord(curTable, WIKI_TRACK_TABLE))
	actualDb = wikiDbName();
    struct hTableInfo *hti = maybeGetHti(actualDb, curTable, conn);
    char *idField = getIdField(actualDb, curTrack, curTable, hti);
    if (idField == NULL)
	{
	warn("Sorry, I can't tell which field of table %s to treat as the "
	     "identifier field.", curTable);
	webNewSection("Table Browser");
	cartRemove(cart, hgtaIdentifierDb);
	cartRemove(cart, hgtaIdentifierTable);
	cartRemove(cart, hgtaIdentifierFile);
	mainPageAfterOpen(conn);
	htmlClose();
	return;
	}
    struct slName *allTerms = NULL, *term;
    while (lineFileNext(lf, &line, NULL))
	{
	while ((word = nextWord(&line)) != NULL)
	    {
	    term = slNameNew(word);
	    slAddHead(&allTerms, term);
	    totalTerms++;
	    }
	}
    slReverse(&allTerms);
    lineFileClose(&lf);
    char *extraWhere = NULL;
    int maxIdsInWhere = cartUsualInt(cart, "hgt_maxIdsInWhere", DEFAULT_MAX_IDS_IN_WHERE);
    if (totalTerms > 0 && totalTerms <= maxIdsInWhere)
	extraWhere = slNameToInExpression(idField, allTerms);

    struct lm *lm = lmInit(0);
    struct hash *matchHash = getAllPossibleIds(conn, lm, idField, extraWhere);
    trashDirFile(&tn, "hgtData", "identifiers", ".key");
    f = mustOpen(tn.forCgi, "w");
    for (term = allTerms;  term != NULL;  term = term->next)
	{
	struct slName *matchList = NULL, *match;
	if (matchHash == NULL)
	    {
	    matchList = slNameNew(term->name);
	    }
	else
	    {
	    /* Support multiple alias->id mappings: */
	    char upcased[1024];
	    safecpy(upcased, sizeof(upcased), term->name);
	    touppers(upcased);
	    struct hashEl *hel = hashLookup(matchHash, upcased);
	    if (hel != NULL)
		{
		matchList = slNameNew((char *)hel->val);
		while ((hel = hashLookupNext(hel)) != NULL)
		    {
		    match = slNameNew((char *)hel->val);
		    slAddHead(&matchList, match);
		    }
		}
	    }
	if (matchList != NULL)
	    {
	    foundTerms++;
	    for (match = matchList;  match != NULL;  match = match->next)
		{
		mustWrite(f, match->name, strlen(match->name));
		mustWrite(f, "\n", 1);
		}
	    }
	else 
	    {
	    slAddHead(&missingTerms, slNameNew(term->name));
	    }
	}
    slReverse(&missingTerms);
    carefulClose(&f);
    cartSetString(cart, hgtaIdentifierDb, database);
    cartSetString(cart, hgtaIdentifierTable, curTable);
    cartSetString(cart, hgtaIdentifierFile, tn.forCgi);
    if (saveIdText)
	freez(&idTextForLf);
    else
	cartRemove(cart, hgtaPastedIdentifiers);
    int missingCount = totalTerms - foundTerms;
    if (missingCount > 0)
	{
	char *xrefTable, *aliasField;
	getXrefInfo(conn, &xrefTable, NULL, &aliasField);
	boolean xrefIsSame = xrefTable && sameString(curTable, xrefTable);
	struct tempName tn;
	trashDirFile(&tn, "hgt/missingIds", cartSessionId(cart), ".tmp");
	FILE *f = mustOpen(tn.forCgi, "w");
	int exampleCount = 0;
	for (term = missingTerms;  term != NULL;  term = term->next)
	    {
	    if (exampleCount < 10)
		{
		++exampleCount;
		dyStringPrintf(exampleMissingIds, "%s\n", term->name);
		}
	    fprintf(f, "%s\n", term->name);
	    }
	carefulClose(&f);

	warn("Note: %d of the %d given identifiers have no match in "
	     "table %s, field %s%s%s%s%s.  "
	     "Try the \"describe table schema\" button for more "
	     "information about the table and field.\n"
	     "%d %smissing identifier(s):\n"
	     "%s\n"
	     "<a href='%s|none|'>Complete list of missing identifiers<a>\n",
	     (totalTerms - foundTerms), totalTerms,
	     curTable, idField,
	     (xrefTable ? (xrefIsSame ? "" : " or in alias table ") : ""),
	     (xrefTable ? (xrefIsSame ? "" : xrefTable) : ""),
	     (xrefTable ? (xrefIsSame ? " or in field " : ", field ") : ""),
	     (xrefTable ? aliasField : ""),
	     exampleCount,
	     exampleCount < missingCount ? "example " : "",
	     exampleMissingIds->string,
	     tn.forHtml
	    );
	webNewSection("Table Browser");
	}
    lmCleanup(&lm);
    hashFree(&matchHash);
    }
else
    {
    cartRemove(cart, hgtaIdentifierFile);
    }
mainPageAfterOpen(conn);
htmlClose();
}

char *identifierFileName()
/* File name identifiers are in, or NULL if not for curTable or no such file. */
{
char *fileName = cartOptionalString(cart, hgtaIdentifierFile);
if (fileName == NULL)
    return NULL;
if (! forCurTable())
    return NULL;
if (fileExists(fileName))
    return fileName;
else
    {
    cartRemove(cart, hgtaIdentifierFile);
    return NULL;
    }
}

struct hash *identifierHash(char *db, char *table)
/* Return hash full of identifiers from the given table (or NULL). */
{
char dbDotTable[2048];
if (sameString(table, WIKI_TRACK_TABLE))
    safecpy(dbDotTable, sizeof(dbDotTable), table);
else if (!sameString(db, database))
    safef(dbDotTable, sizeof(dbDotTable), "%s.%s", db, table);
else
    safecpy(dbDotTable, sizeof(dbDotTable), table);
if (! (sameString(dbDotTable, curTable) ||
       sameString(connectingTableForTrack(dbDotTable), curTable)) )
    return NULL;
char *fileName = identifierFileName();
if (fileName == NULL)
    return NULL;
else
    {
    struct lineFile *lf = lineFileOpen(fileName, TRUE);
    struct hash *hash = hashNew(18);
    char *line, *word;
    while (lineFileNext(lf, &line, NULL))
        {
	while ((word = nextWord(&line)) != NULL)
	    hashAdd(hash, word, NULL);
	}
    lineFileClose(&lf);
    return hash;
    }
}

char *identifierWhereClause(char *idField, struct hash *idHash)
/* If the number of pasted IDs is reasonably low, return a where-clause component for the IDs. */
{
if (idHash == NULL || idField == NULL)
    return NULL;
int numIds = hashNumEntries(idHash);
int maxIdsInWhere = cartUsualInt(cart, "hgt_maxIdsInWhere", DEFAULT_MAX_IDS_IN_WHERE);
if (numIds > 0 && numIds <= maxIdsInWhere)
    {
    struct dyString *dy = dyStringNew(16 * numIds);
    dyStringPrintf(dy, "%s in (", idField);
    struct hashCookie hc = hashFirst(idHash);
    boolean first = TRUE;
    char *id;
    while ((id = hashNextName(&hc)) != NULL)
	{
	if (first)
	    first = FALSE;
	else
	    dyStringAppend(dy, ", ");
	dyStringPrintf(dy, "'%s'", id);
	}
    dyStringAppend(dy, ")");
    return dyStringCannibalize(&dy);
    }
return NULL;
}

void doClearPasteIdentifierText(struct sqlConnection *conn)
/* Respond to clear within paste identifier page. */
{
cartRemove(cart, hgtaPastedIdentifiers);
doPasteIdentifiers(conn);
}

void doClearIdentifiers(struct sqlConnection *conn)
/* Respond to clear identifiers button. */
{
char *fileName;

htmlOpen("Table Browser (Cleared Identifiers)");
fileName = identifierFileName();
if (fileName != NULL)
    remove(fileName);
cartRemove(cart, hgtaIdentifierFile);
cartRemove(cart, hgtaPastedIdentifiers);
mainPageAfterOpen(conn);
htmlClose();
}


