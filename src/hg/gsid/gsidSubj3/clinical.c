/* clinical - do Clinical section. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "dystring.h"
#include "cheapcgi.h"
#include "spDb.h"
#include "gsidSubj3.h"
#include "hdb.h"
#include "net.h"


static boolean clinicalExists(struct section *section,
	struct sqlConnection *conn, char *subjId)
/* Return TRUE if clinicalAll table exists and it has an entry with the gene symbol */
{
if (sqlTableExists(conn, "gsidClinicRec") == TRUE)
    {
    return(TRUE);
    }
return(FALSE);
}

static void clinicalPrint(struct section *section,
	struct sqlConnection *conn, char *subjId)
/* Print out Clinical section. */
{
char bigQuery[2000];

struct sqlResult *sr;
char **row;
char *specimenId, *labCode, *daysCollection, *hivQuan, *cd4Count;
char *naString = strdup("N/A");
printf("<TABLE BGCOLOR='#222222' CELLSPACING=1 CELLPADDING=3><TR>\n");

printf("<TR>\n");
printf("<TD align=left BGCOLOR=\"#8686D1\"><B style='color:#FFFFFF;'>Days After Estimated<BR>Infection (DAEI)*</B></TD>\n");
printf("<TD align=center BGCOLOR=\"#8686D1\"><B style='color:#FFFFFF;'>HIV-1 RNA<BR>copies/mL</B></TD>\n");
printf("<TD align=center BGCOLOR=\"#8686D1\"><B style='color:#FFFFFF;'>CD4<BR>cells/microliter</B></TD>\n");
printf("</TR>\n");

/* complex query to ensure date is correctly sorted */
sqlSafef(bigQuery, sizeof(bigQuery),
"select specimenId, labCode, daysCollection, hivQuan, cd4Count from gsidClinicRec where subjId='%s' order by daysCollection",
subjId);

sr = sqlMustGetResult(conn, bigQuery);
row = sqlNextRow(sr);

/* special processing if this subject does not have any clinical data */
if (row == NULL)
    {
    /*specimenId     = row[0];
    labCode        = row[1];
    daysCollection = row[2];
    hivQuan        = row[3];
    cd4Count       = row[4];
    */

    daysCollection = naString;
    hivQuan = naString;
    cd4Count = naString;

    printf("<TR>");
    printf("<TD align=right BGCOLOR=\"#D9F8E4\">%s</TD>\n", daysCollection);
    printf("<TD align=right BGCOLOR=\"#D9F8E4\">%s</TD>\n", hivQuan);

    printf("<TD align=right BGCOLOR=\"#D9F8E4\">%s</TD>\n", cd4Count);
    printf("</TR>");
    }

while (row != NULL)
    {
    specimenId     = row[0];
    labCode        = row[1];
    daysCollection = row[2];
    hivQuan        = row[3];
    cd4Count       = row[4];

    if (daysCollection == NULL) daysCollection = naString;
    if (hivQuan  == NULL) hivQuan = naString;
    if (cd4Count == NULL) cd4Count = naString;

    printf("<TR>");
    printf("<TD align=right BGCOLOR=\"#D9F8E4\">%s</TD>\n", daysCollection);
    //printf("<TD align=right BGCOLOR=\"#D9F8E4\">%s</TD>\n", cd4Count);
    if (sameWord(hivQuan, "1000000"))
	{
    	printf("<TD align=right BGCOLOR=\"#D9F8E4\">&gt;&nbsp;1000000</TD>\n");
    	}
    else
        {
        if (sameWord(hivQuan, "200"))
	    {
    	    printf("<TD align=right BGCOLOR=\"#D9F8E4\">&lt;&nbsp;400</TD>\n");
    	    }
    	else
	    {
            printf("<TD align=right BGCOLOR=\"#D9F8E4\">%s</TD>\n", hivQuan);
    	    }
	}

    if (sameWord(cd4Count, "0"))
	{
    	printf("<TD align=right BGCOLOR=\"#D9F8E4\">N/A</TD>\n");
    	}
    else
	{
    	printf("<TD align=right BGCOLOR=\"#D9F8E4\">%s</TD>\n", cd4Count);
    	}

    printf("</TR>");

    row = sqlNextRow(sr);
    }
sqlFreeResult(&sr);
printf("</TR></TABLE>");
printf("<br>* Estimated Study Day of Infection (ESDI), ");
printf("click <a href=\"http://www.gsid.org/downloads/methods_and_conventions.pdf\" target=_blank> here </a>");
printf(" for further explanation.\n");
printf("<br>* Days After Estimated Infection (DAEI), ");
printf("click <a href=\"http://www.gsid.org/downloads/methods_and_conventions.pdf\" target=_blank> here </a>");
printf(" for further explanation.\n");
return;
}

struct section *clinicalSection(struct sqlConnection *conn,
	struct hash *sectionRa)
/* Create clinical section. */
{
struct section *section = sectionNew(sectionRa, "clinical");
section->exists = clinicalExists;
section->print = clinicalPrint;
return section;
}

