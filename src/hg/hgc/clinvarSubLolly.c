#include "common.h"
#include "wiggle.h"
#include "cart.h"
#include "hgc.h"
#include "hCommon.h"
#include "hgColors.h"
#include "bigBed.h"
#include "hui.h"
#include "subText.h"   
#include "trackHub.h"   
#include "clinvarSubLolly.h"   
#include "chromAlias.h"

char *statusByScore[] =
{
"Other",
"Benign",
"Likely Benign",
"Uncertain significance",
"Likely pathogenic",
"Pathogenic",
};

void printSubmissions(struct trackDb *tdb, char *chrom, int start, int end, unsigned wantScore, int numSubs, boolean not)
/* Print out all the submissions at this position that match the 
 * desired score (ie. clinical status).
 * If "not" is true, then print out all submissions that do NOT match score.
 * We're grabbing these records out of a second bigBed that's referenced by
 * "xrefDataUrl" in the trackDb.
 */
{
char *xrefDataUrl = hReplaceGbdb(trackDbSetting(tdb, "xrefDataUrl"));
struct bbiFile *bbi =  bigBedFileOpenAlias(xrefDataUrl, chromAliasChromToAliasHash(database));
struct asObject *as = bigBedAsOrDefault(bbi);
struct lm *lm = lmInit(0);
struct bigBedInterval *bbList = bigBedIntervalQuery(bbi, chrom, start, end, 0, lm);

int count;
if (not)
    count = slCount(bbList) - numSubs;
else 
    count = numSubs;

// nothing to print
if (count == 0)
    return;

if (!not)
    printf("<BR><B>There are %d submissions at this position with clinical significance '%s'.</B><BR>\n", count, statusByScore[wantScore]);
else
    printf("<BR><B>There are %d submissions at this position with some other clinical significance.</B><BR>\n", count);

// Step through intervals at the position to find ones that match the score we're looking for
struct bigBedInterval *bb;
for (bb = bbList; bb != NULL; bb = bb->next)
    {
    char *fields[bbi->fieldCount];
    int restCount = chopTabs(cloneString(bb->rest), fields);
    int score = atoi(fields[1]);
    if (not ^ (score != wantScore))
        continue;

    int restBedFields = 6;
    char **extraFields = (fields + restBedFields);
    int extraFieldCount = restCount - restBedFields;
    extraFieldsPrintAs(tdb, NULL, extraFields, extraFieldCount, as);
    }
printf("<BR>");
}

void doClinvarSubLolly(struct trackDb *tdb, char *item)
/* Put up page for clinvarSubLolly track. */
{
genericHeader(tdb, NULL);
struct sqlConnection *conn = NULL;

if (!trackHubDatabase(database))
    conn = hAllocConnTrack(database, tdb);

int start = cartInt(cart, "o");
int end = cartInt(cart, "t");
char *chrom = cartString(cart, "c");
char *fileName = bbiNameFromSettingOrTable(tdb, conn, tdb->table);
struct bbiFile *bbi =  bigBedFileOpenAlias(hReplaceGbdb(fileName), chromAliasChromToAliasHash(database));
struct lm *lm = lmInit(0);
struct bigBedInterval *bbList = bigBedIntervalQuery(bbi, chrom, start, end, 0, lm);

struct bigBedInterval *bb;
char *fields[bbi->fieldCount];
for (bb = bbList; bb != NULL; bb = bb->next)
    {
    if (!(bb->start == start && bb->end == end))
	continue;

    // our names are unique
    char *name = cloneFirstWordByDelimiterNoSkip(bb->rest, '\t');
    boolean match = (isEmpty(name) && isEmpty(item)) || sameOk(name, item);
    if (!match)
        continue;

    char startBuf[16], endBuf[16];
    bigBedIntervalToRow(bb, chrom, startBuf, endBuf, fields, bbi->fieldCount);
    int numSubs = chopString(fields[12], ",", NULL, 0);

    printPos(chrom, bb->start, bb->end, NULL, FALSE, name);
    // print all the submissions that match the clinical significance of the
    // bead that the user clicked on.
    printSubmissions(tdb,  chrom, start, end, atoi(fields[4]), numSubs, FALSE);

    // no print the ones with a different clinical status
    printSubmissions(tdb,  chrom, start, end, atoi(fields[4]), numSubs, TRUE);

    // we found what we wanted
    break;
    }
printTrackHtml(tdb);
}
