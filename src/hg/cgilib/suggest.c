/* code to support suggesting genes given a prefix typed by the user. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "suggest.h"


char *connGeneSuggestTable(struct sqlConnection *conn)
// return name of gene suggest table if this connection has tables to support gene autocompletion, NULL otherwise
{
if(sqlTableExists(conn, "knownCanonical"))
    return "knownCanonical";
else if(sqlTableExists(conn, "refGene"))
    return "refGene";
else
    return NULL;
}

boolean assemblySupportsGeneSuggest(char *database)
// return true if this assembly has tables to support gene autocompletion
{
char *knownDatabase = hdbDefaultKnownDb(database);
struct sqlConnection *conn = hAllocConn(knownDatabase);
char *table = connGeneSuggestTable(conn);
hFreeConn(&conn);
return table != NULL;
}

char *assemblyGeneSuggestTrack(char *database)
// return name of gene suggest track if this assembly has tables to support gene autocompletion, NULL otherwise
// Do NOT free returned string.
{
char *knownDatabase = hdbDefaultKnownDb(database);
struct sqlConnection *conn = hAllocConn(knownDatabase);
char *table = connGeneSuggestTable(conn);
hFreeConn(&conn);
if(table != NULL)
    {
    if(sameString(table, "knownCanonical"))
        {
        if (differentString(knownDatabase, database))
            {
            return hdbGetMasterGeneTrack(knownDatabase);
            }
        return "knownGene";
        }
    else
        return "refGene";
    }
else
    return NULL;
}
