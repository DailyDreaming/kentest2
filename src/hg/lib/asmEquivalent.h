/* asmEquivalent.h was originally generated by the autoSql program, which also 
 * generated asmEquivalent.c and asmEquivalent.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef ASMEQUIVALENT_H
#define ASMEQUIVALENT_H

#include "jksql.h"
#define ASMEQUIVALENT_NUM_COLS 7

extern char *asmEquivalentCommaSepFieldNames;

enum asmEquivalentSourceAuthority
    {
    asmEquivalentEnsembl = 0,
    asmEquivalentUcsc = 1,
    asmEquivalentGenbank = 2,
    asmEquivalentRefseq = 3,
    };
enum asmEquivalentDestinationAuthority
    {
    asmEquivalentEnsembl = 0,
    asmEquivalentUcsc = 1,
    asmEquivalentGenbank = 2,
    asmEquivalentRefseq = 3,
    };
struct asmEquivalent
/* Equivalence relationship of assembly versions, Ensembl: UCSC, NCBI genbank/refseq */
    {
    struct asmEquivalent *next;  /* Next in singly linked list. */
    char *source;	/* assembly name */
    char *destination;	/* equivalent assembly name */
    enum asmEquivalentSourceAuthority sourceAuthority;	/* origin of source assembly */
    enum asmEquivalentDestinationAuthority destinationAuthority;	/* origin of equivalent assembly */
    long long matchCount;	/* number of exactly matching sequences */
    long long sourceCount;	/* number of sequences in source assembly */
    long long destinationCount;	/* number of sequences in equivalent assembly */
    };

void asmEquivalentStaticLoad(char **row, struct asmEquivalent *ret);
/* Load a row from asmEquivalent table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct asmEquivalent *asmEquivalentLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all asmEquivalent from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with asmEquivalentFreeList(). */

void asmEquivalentSaveToDb(struct sqlConnection *conn, struct asmEquivalent *el, char *tableName, int updateSize);
/* Save asmEquivalent as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. This function automatically escapes quoted strings for mysql. */

struct asmEquivalent *asmEquivalentLoad(char **row);
/* Load a asmEquivalent from row fetched with select * from asmEquivalent
 * from database.  Dispose of this with asmEquivalentFree(). */

struct asmEquivalent *asmEquivalentLoadAll(char *fileName);
/* Load all asmEquivalent from whitespace-separated file.
 * Dispose of this with asmEquivalentFreeList(). */

struct asmEquivalent *asmEquivalentLoadAllByChar(char *fileName, char chopper);
/* Load all asmEquivalent from chopper separated file.
 * Dispose of this with asmEquivalentFreeList(). */

#define asmEquivalentLoadAllByTab(a) asmEquivalentLoadAllByChar(a, '\t');
/* Load all asmEquivalent from tab separated file.
 * Dispose of this with asmEquivalentFreeList(). */

struct asmEquivalent *asmEquivalentCommaIn(char **pS, struct asmEquivalent *ret);
/* Create a asmEquivalent out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new asmEquivalent */

void asmEquivalentFree(struct asmEquivalent **pEl);
/* Free a single dynamically allocated asmEquivalent such as created
 * with asmEquivalentLoad(). */

void asmEquivalentFreeList(struct asmEquivalent **pList);
/* Free a list of dynamically allocated asmEquivalent's */

void asmEquivalentOutput(struct asmEquivalent *el, FILE *f, char sep, char lastSep);
/* Print out asmEquivalent.  Separate fields with sep. Follow last field with lastSep. */

#define asmEquivalentTabOut(el,f) asmEquivalentOutput(el,f,'\t','\n');
/* Print out asmEquivalent as a line in a tab-separated file. */

#define asmEquivalentCommaOut(el,f) asmEquivalentOutput(el,f,',',',');
/* Print out asmEquivalent as a comma separated list including final comma. */

void asmEquivalentJsonOutput(struct asmEquivalent *el, FILE *f);
/* Print out asmEquivalent in JSON format. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* ASMEQUIVALENT_H */

