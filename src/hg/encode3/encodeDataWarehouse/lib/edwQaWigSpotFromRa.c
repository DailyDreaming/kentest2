/* Parser to read in a edwQaWigSpot from a ra file where tags in ra file correspond to fields in a
 * struct. This program was generated by raToStructGen. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "obscure.h"
#include "sqlNum.h"
#include "sqlList.h"
#include "ra.h"
#include "raToStruct.h"
#include "encodeDataWarehouse.h"
#include "edwQaWigSpotFromRa.h"

struct raToStructReader *edwQaWigSpotRaReader()
/* Make a raToStructReader for edwQaWigSpot */
{
static char *fields[] = {
    "spotRatio",
    "enrichment",
    "basesInGenome",
    "basesInSpots",
    "sumSignal",
    "spotSumSignal",
    };
static char *requiredFields[] = {
    "spotRatio",
    "enrichment",
    "basesInGenome",
    "basesInSpots",
    "sumSignal",
    "spotSumSignal",
    };
return raToStructReaderNew("edwQaWigSpot", ArraySize(fields), fields, ArraySize(requiredFields), requiredFields);
}


struct edwQaWigSpot *edwQaWigSpotFromNextRa(struct lineFile *lf, struct raToStructReader *reader)
/* Return next stanza put into an edwQaWigSpot. */
{
enum fields
    {
    spotRatioField,
    enrichmentField,
    basesInGenomeField,
    basesInSpotsField,
    sumSignalField,
    spotSumSignalField,
    };
if (!raSkipLeadingEmptyLines(lf, NULL))
    return NULL;

struct edwQaWigSpot *el;
AllocVar(el);

bool *fieldsObserved = reader->fieldsObserved;
bzero(fieldsObserved, reader->fieldCount);

char *tag, *val;
while (raNextTagVal(lf, &tag, &val, NULL))
    {
    struct hashEl *hel = hashLookup(reader->fieldIds, tag);
    if (hel != NULL)
        {
	int id = ptToInt(hel->val);
	if (fieldsObserved[id])
	     errAbort("Duplicate tag %s line %d of %s\n", tag, lf->lineIx, lf->fileName);
	fieldsObserved[id] = TRUE;
	switch (id)
	    {
	    case spotRatioField:
	        {
	        el->spotRatio = sqlDouble(val);
		break;
	        }
	    case enrichmentField:
	        {
	        el->enrichment = sqlDouble(val);
		break;
	        }
	    case basesInGenomeField:
	        {
	        el->basesInGenome = sqlLongLong(val);
		break;
	        }
	    case basesInSpotsField:
	        {
	        el->basesInSpots = sqlLongLong(val);
		break;
	        }
	    case sumSignalField:
	        {
	        el->sumSignal = sqlDouble(val);
		break;
	        }
	    case spotSumSignalField:
	        {
	        el->spotSumSignal = sqlDouble(val);
		break;
	        }
	    default:
	        internalErr();
		break;
	    }
	}
    }

raToStructReaderCheckRequiredFields(reader, lf);
return el;
}

struct edwQaWigSpot *edwQaWigSpotLoadRa(char *fileName)
/* Return list of all edwQaWigSpot in ra file. */
{
struct raToStructReader *reader = edwQaWigSpotRaReader();
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct edwQaWigSpot *el, *list = NULL;
while ((el = edwQaWigSpotFromNextRa(lf, reader)) != NULL)
    slAddHead(&list, el);
slReverse(&list);
lineFileClose(&lf);
raToStructReaderFree(&reader);
return list;
}

struct edwQaWigSpot *edwQaWigSpotOneFromRa(char *fileName)
/* Return edwQaWigSpot in file and insist there be exactly one record. */
{
struct edwQaWigSpot *one = edwQaWigSpotLoadRa(fileName);
if (one == NULL)
    errAbort("No data in %s", fileName);
if (one->next != NULL)
    errAbort("Multiple records in %s", fileName);
return one;
}

