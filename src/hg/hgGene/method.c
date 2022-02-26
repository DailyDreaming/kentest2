/* method - UCSC Known Genes Method. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "dystring.h"
#include "spDb.h"
#include "web.h"
#include "hgGene.h"



static void methodPrint(struct section *section, 
	struct sqlConnection *conn, char *geneId)
/* Print out link to UCSC KG method and credits details. */
{

hPrintf("Click ");
hPrintf("<A HREF=\"../cgi-bin/hgGene?%s=1&%s\">", hggDoKgMethod, cartSidUrlString(cart));
hPrintf("here</A>\n");
hPrintf(" for details on how this gene model was made and data restrictions if any.");
}

struct section *methodSection(struct sqlConnection *conn,
	struct hash *sectionRa)
/* Create UCSC KG Method section. */
{
struct section *section = sectionNew(sectionRa, "method");
section->print = methodPrint;
return section;
}

