/* hubSearchText.h was originally generated by the autoSql program, which also 
 * generated hubSearchText.c and hubSearchText.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef HUBSEARCHTEXT_H
#define HUBSEARCHTEXT_H

#define HUBSEARCHTEXT_NUM_COLS 7

extern char *hubSearchTextCommaSepFieldNames;

enum hubSearchTextTextLength
    {
    hubSearchTextShort = 0,
    hubSearchTextLong = 1,
    hubSearchTextMeta = 2,
    };
struct hubSearchText
/* Track hub descriptions */
    {
    struct hubSearchText *next;  /* Next in singly linked list. */
    char *hubUrl;	/* Hub URL */
    char *db;	/* Assembly name (UCSC format) for Assembly and Track descriptions, NULL for hub descriptions */
    char *track;	/* Track name for track descriptions, NULL for others */
    char *label;	/* Name to display in search results */
    enum hubSearchTextTextLength textLength;	/* Length of text (short for labels, long for description pages, meta for metadata) */
    char *text;	/* Description text */
    char *parents;	/* Comma separated list of parent track of this track, NULL for others */
    char *parentTypes; /* Comma separated list of parent track types */
    };

void hubSearchTextStaticLoadWithNull(char **row, struct hubSearchText *ret);
/* Load a row from hubSearchText table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hubSearchText *hubSearchTextLoadWithNull(char **row);
/* Load a hubSearchText from row fetched with select * from hubSearchText
 * from database.  Dispose of this with hubSearchTextFree(). */

struct hubSearchText *hubSearchTextLoadAll(char *fileName);
/* Load all hubSearchText from whitespace-separated file.
 * Dispose of this with hubSearchTextFreeList(). */

struct hubSearchText *hubSearchTextLoadAllByChar(char *fileName, char chopper);
/* Load all hubSearchText from chopper separated file.
 * Dispose of this with hubSearchTextFreeList(). */

#define hubSearchTextLoadAllByTab(a) hubSearchTextLoadAllByChar(a, '\t');
/* Load all hubSearchText from tab separated file.
 * Dispose of this with hubSearchTextFreeList(). */

struct hubSearchText *hubSearchTextCommaIn(char **pS, struct hubSearchText *ret);
/* Create a hubSearchText out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hubSearchText */

void hubSearchTextFree(struct hubSearchText **pEl);
/* Free a single dynamically allocated hubSearchText such as created
 * with hubSearchTextLoad(). */

void hubSearchTextFreeList(struct hubSearchText **pList);
/* Free a list of dynamically allocated hubSearchText's */

void hubSearchTextOutput(struct hubSearchText *el, FILE *f, char sep, char lastSep);
/* Print out hubSearchText.  Separate fields with sep. Follow last field with lastSep. */

#define hubSearchTextTabOut(el,f) hubSearchTextOutput(el,f,'\t','\n');
/* Print out hubSearchText as a line in a tab-separated file. */

#define hubSearchTextCommaOut(el,f) hubSearchTextOutput(el,f,',',',');
/* Print out hubSearchText as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

struct hubSearchText *hubSearchTextLoadWithNullGiveContext(char **row, char *searchTerms);
/* Load a hubSearchText from row fetched with select * from hubSearchText
 * from database, but instead of loading the entire text field for long text results,
 * only load the pieces that provide context for the supplied searchTerms.
 * Dispose of this with hubSearchTextFree(). */

char *modifyTermsForHubSearch(char *hubSearchTerms, bool isStrictSearch);
/* This won't exactly be pretty.  MySQL treats any sequence of alphanumerics and underscores
 * as a word, and single apostrophes are allowed as long as they don't come back-to-back.
 * Cut down to those characters, then add initial + (for requiring word) and * (for word
 * expansion) as appropriate. */

void getHubSearchResults(struct sqlConnection *conn, char *hubSearchTableName,
        char *hubSearchTerms, bool checkLongText, char *dbFilter, struct hash *hubLookup,
        struct hash **searchResultHashRet, struct slName **hubsToPrintRet, char *extra);
/* Find hubs, genomes, and tracks that match the provided search terms.
 * Return all hits that satisfy the (optional) supplied assembly filter.
 * if checkLongText is FALSE, skip searching within the long description text entries
 * extra contains optional caller specific mysql filtering*/

char *hubSearchTextTableName();

#endif /* HUBSEARCHTEXT_H */

