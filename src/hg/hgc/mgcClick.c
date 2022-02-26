/* mgcClick - click handling for MGC and ORFEome related tracks */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "hgc.h"
#include "mgcClick.h"
#include "ccdsClick.h"
#include "ccdsGeneMap.h"
#include "gbMiscDiff.h"
#include "web.h"
#include "genbank.h"
#include "htmshell.h"
#include "genePred.h"
#include "geneSimilarities.h"
#include "genbank.h"


static char *findRefSeqSummary(struct sqlConnection *conn,
                               struct geneSimilarities *refSeqs,
                               char **sumAccv)
/* Given similar refseq genes, find the first one with a RefSeq
 * summary and return that summary, or NULL if not found.  Also returns
 * accv of matched */
{
char buf[GENBANK_ACC_BUFSZ];
struct geneSim *rs;
for (rs = refSeqs->genes; rs != NULL; rs = rs->next)
    {
    char *sum = getRefSeqSummary(conn, genbankDropVer(buf, rs->gene->name));
    if (sum != NULL)
        {
        *sumAccv = cloneString(rs->gene->name);
        return sum;
        }
    }
*sumAccv = NULL;
return NULL;
}

static char *getAccVersion(struct sqlConnection *conn, char *acc)
/* given a accession, get acc.ver */
{
char query[256], accver[64];
sqlSafef(query, sizeof(query), "SELECT version FROM %s WHERE acc=\"%s\"", gbCdnaInfoTable, acc);
safef(accver, sizeof(accver), "%s.%d", acc, sqlNeedQuickNum(conn, query));
return cloneString(accver);
}

struct mgcDb
/* information about an MGC databases */
{
    char *name;       /* collection name */
    char *title;      /* collection title */
    char *organism;   /* organism name for URL, case-sensitive */
    char *server;     /* MGC server */
};

static struct mgcDb getMgcDb()
/* get the mgc database info for the current host */
{
struct mgcDb mgcDb;
mgcDb.name = "MGC";
mgcDb.title = "Mammalian Gene Collection";
mgcDb.server = "mgc";
mgcDb.organism = NULL;
/* NOTE: mgc server likes first letter of organism capitalized */
if (startsWith("hg", database) || startsWith("braNey", database))
    mgcDb.organism = "Hs";
else if (startsWith("mm", database))
    mgcDb.organism = "Mm";
else if (startsWith("rn", database))
    mgcDb.organism = "Rn";
else if (startsWith("bosTau", database))
    mgcDb.organism = "Bt";
else if (startsWith("danRer", database))
    {
    mgcDb.name = "ZGC";
    mgcDb.title = "Zebrafish Gene Collection";
    mgcDb.organism = "Dr";
    mgcDb.server = "zgc";
    }
else if (startsWith("xenTro", database))
    {
    mgcDb.name = "XGC";
    mgcDb.title = "Xenopus Gene Collection";
    mgcDb.organism = "Str";
    mgcDb.server = "xgc";
    }
else
    errAbort("can't map database \"%s\" to an MGC organism", database);
return mgcDb;
}

char *mgcDbName()
/* get just the MGC collection name for the current ucsc database */
{
return getMgcDb().name;
}

void printMgcHomeUrl(struct mgcDb *mgcDb)
/* print out an URL to link to the MGC site */
{
printf("http://%s.nci.nih.gov/", mgcDb->server);
}

void printMgcUrl(int imageId)
/* print out an URL to link to the MGC site for a full-length MGC clone */
{
struct mgcDb mgcDb = getMgcDb();
printf("http://%s.nci.nih.gov/Reagents/CloneInfo?ORG=%s&IMAGE=%d",
       mgcDb.server, mgcDb.organism, imageId);
}

static void printOrderUrl(int gi)
/* print out an URL to link to the NCBI order CGI for a full-length MGC clone */
{
printf("https://www.ncbi.nlm.nih.gov/genome/clone/orderclone.cgi?db=nucleotide&uid=%d", gi);
}

static void printImageUrl(int imageId)
/* print out an URL to link to IMAGE database for a clone */
{
printf("http://www.imageconsortium.org/IQ/bin/singleCloneQuery?clone_id=%d", imageId);
}

void printMgcDetailsUrl(char *acc, int start)
/* print out an URL to link to MGC details pages from another details page in
 * the browser.*/
{
// pass zero coordiates for window to indicate this isn't a browser click
printf("../cgi-bin/hgc?%s&g=mgcGenes&o=%d&i=%s&l=0&r=0&db=%s",
       cartSidUrlString(cart), start, acc, database);
}

static void printMBLabValidDbUrl(char *acc)
/* print out an URL to link to Brent lab validation database */
{
printf("http://mblab.wustl.edu/cgi-bin/mgc.cgi?acc=%s&action=cloneRpt&alignment=Submit", acc);
}

struct cloneInfo
/* Information on a MGC or ORFeome clone collected from various tables */
{
    boolean isMgc;    // is this MGC or ORFeome
    char *acc;
    int start;
    char *pslTbl;     // psl-format table
    char *gpTbl;      // genePred format table
    char *desc;       // genbank info
    char *organism;
    char *tissue;
    char *library;
    char *development;
    char *geneName;
    char *productName;
    char *moddate;
    char *clone;
    char *cds;
    char *keyword;
    int version;
    int imageId;
    int mgcId;
    int gi;
    char *refSeqAccv;     // best RefSeq acc.version, or NULL
    char *refSeqSum;      // RefSeq from best matching RefSeq with summary, or NULL.
    char *refSeqSumAccv;  // accv for summary, maybe different than best match
    struct geneSimilarities *refSeqs;  // most similar RefSeqs, with name set to acc.version
};

static boolean isInMBLabValidDb(char *acc)
/* check if an accession is in the Brent lab validation database */
{
boolean inMBLabValidDb = FALSE;
struct sqlConnection *fconn = sqlMayConnect("hgFixed");
if ((fconn != NULL) && sqlTableExists(fconn, "mgcMBLabValid"))
    {
    char query[64], buf[32];
    sqlSafef(query, sizeof(query), "select acc from mgcMBLabValid where acc=\"%s\"",
          acc);
    if (sqlQuickQuery(fconn, query, buf, sizeof(buf)) != NULL)
        inMBLabValidDb = TRUE;
    sqlDisconnect(&fconn);
    }
return inMBLabValidDb;
}

static void cdnaInfoLoad(struct cloneInfo *ci, struct sqlConnection *conn)
/* Loading clone information from gbCdnaInfoTable relational tables. */
{
// data from gbCdnaInfoTable and friends
char query[1024];
sqlSafef(query, sizeof(query),
      "select "
      "des.name, o.name, t.name, l.name,"
      "dev.name, gene.name, p.name, m.name,"
      "c.name,k.name,g.moddate,g.version,"
      "g.gi"
      " from "
      "%s g,%s des,%s o,%s t,%s l,%s dev,"
      "%s gene,%s p,%s m,%s c,%s k"
      " where "
      "(acc = \"%s\") and"
      "(description = des.id) and (organism = o.id) and"
      "(tissue = t.id) and (library = l.id) and"
      "(development = dev.id) and (geneName = gene.id) and"
      "(productName = p.id) and (mrnaClone = m.id) and"
      "(cds = c.id) and (keyword = k.id)", gbCdnaInfoTable, descriptionTable, organismTable, tissueTable, libraryTable, developmentTable,geneNameTable, productNameTable, mrnaCloneTable, cdsTable, keywordTable, ci->acc);
struct sqlResult *sr = sqlGetResult(conn, query);
char **row = sqlNextRow(sr);
if (row == NULL)
    errAbort("can't find %s in %s", ci->acc, gbCdnaInfoTable);
int i = 0;
ci->desc = cloneString(row[i++]);
ci->organism = cloneString(row[i++]);
ci->tissue = cloneString(row[i++]);
ci->library = cloneString(row[i++]);
ci->development = cloneString(row[i++]);
ci->geneName = cloneString(row[i++]);
ci->productName = cloneString(row[i++]);
ci->clone = cloneString(row[i++]);
ci->cds = cloneString(row[i++]);
ci->keyword = cloneString(row[i++]);
ci->moddate = cloneString(row[i++]);
ci->version = sqlUnsigned(row[i++]);
ci->gi = sqlUnsigned(row[i++]);
sqlFreeResult(&sr);
}

static char *cdnaInfoDbName(struct cloneInfo *ci)
/* get the name to use in describing this gene collection */
{
return (ci->isMgc ? mgcDbName() : "ORFeome");
}

static void getRefSeqInfo(struct sqlConnection *conn, struct cloneInfo *ci)
/* fill in refSeq info */
{
ci->refSeqs = geneSimilaritiesBuildAt(conn, TRUE, ci->acc, seqName, ci->start,
                                      ci->gpTbl, "refGene");
// replace accession in gene names with accession.version
struct geneSim *gs;
for (gs = ci->refSeqs->genes; gs != NULL; gs = gs->next)
    {
    char *accv ;

    // add version number if it's not already there
    if (strchr(gs->gene->name, '.'))
	accv = cloneString(gs->gene->name);
    else
	accv = getAccVersion(conn, gs->gene->name);
    freeMem(gs->gene->name);
    gs->gene->name = accv;
    }

if ((ci->refSeqs != NULL) && (ci->refSeqs->genes != NULL))
    {
    // use first one (highest similarity)
    ci->refSeqAccv = cloneString(ci->refSeqs->genes->gene->name);
    // get summary for first one with summary
    ci->refSeqSum = findRefSeqSummary(conn, ci->refSeqs, &ci->refSeqSumAccv);
    }
}

static void parseCloneField(struct cloneInfo *ci)
/* parse the mrnaClone field to get IMAGE and MGC fields, if available */
{
/* MGC:135061 IMAGE:40080529
 * or
 * IMAGE:100005038; FLH186078.01X; RZPDo839A0971D
 */
char buf[1024], *words[64];
safecpy(buf, sizeof(buf), ci->clone);
int nwords = chopByWhite(buf, words, ArraySize(words));
if (nwords == ArraySize(words))
    errAbort("more words in mrnaClone file than can be parsed, most likely data corruption: %s",
             ci->clone);
int i;
for (i = 0; i < nwords; i++)
    {
    char *word = words[i];
    int l = strlen(word);
    if ((l > 0) && (word[l-1] == ';'))
        word[l-1] = '\0';  // wack trailing `;'
    if (startsWith("MGC:", word))
        ci->mgcId = sqlUnsigned(word+4);
    else if (startsWith("IMAGE:", word))
        ci->imageId = sqlUnsigned(word+6);
    }
}

static struct cloneInfo *cloneInfoLoad(struct sqlConnection *conn, char *acc,
                                       int start, char *pslTbl, char *gpTbl)
/* Load clone information tables. */
{
struct cloneInfo *ci;
AllocVar(ci);
ci->acc = cloneString(acc);
ci->start = start;
ci->pslTbl = cloneString(pslTbl);
ci->gpTbl = cloneString(gpTbl);
cdnaInfoLoad(ci, conn);
parseCloneField(ci);
if (sqlTableExists(conn, "refGene"))
    getRefSeqInfo(conn, ci);
return ci;
}

static void cloneInfoFree(struct cloneInfo **ciPtr)
/* free a cloneInfo object */
{
struct cloneInfo *ci = *ciPtr;
if (ci != NULL)
    {
    freeMem(ci->acc);
    freeMem(ci->pslTbl);
    freeMem(ci->gpTbl);
    freeMem(ci->desc);
    freeMem(ci->organism);
    freeMem(ci->tissue);
    freeMem(ci->library);
    freeMem(ci->development);
    freeMem(ci->geneName);
    freeMem(ci->productName);
    freeMem(ci->moddate);
    freeMem(ci->clone);
    freeMem(ci->cds);
    freeMem(ci->keyword);
    freeMem(ci->refSeqAccv);
    freeMem(ci->refSeqSum);
    geneSimilaritiesFree(&ci->refSeqs);
    freeMem(ci);
    *ciPtr = NULL;
    }
}

static struct cloneInfo *mgcCloneInfoLoad(struct sqlConnection *conn, char *acc,
                                          int start)
/* Load MGC clone information */
{
struct cloneInfo *ci = cloneInfoLoad(conn, acc, start, "mgcFullMrna", "mgcGenes");
ci->isMgc = TRUE;
if (ci->mgcId == 0)
    errAbort("no MGC:nnnn entry in mrnaClone table for MGC clone %s", acc);
if (ci->imageId == 0)
    errAbort("no IMAGE:nnnn entry in mrnaClone table for MGC clone %s", acc);
return ci;
}

static struct cloneInfo *orfeomeCloneInfoLoad(struct sqlConnection *conn, char *acc,
                                              int start)
/* Load ORFeome clone information */
{
struct cloneInfo *ci = cloneInfoLoad(conn, acc, start, "orfeomeMrna", "orfeomeGenes");
ci->isMgc = FALSE;
return ci;
}


static void prCellLabelVal(char *label, char *val)
/* print label and value as adjacent cells  */
{
webPrintLabelCell(label);
webPrintLinkCell(val);
}

static void prInitialSection(struct cloneInfo *ci, char *collection)
/* start page and print initial section */
{
cartWebStart(cart, database, "%s Clone %s.%d", collection, ci->acc, ci->version);
printf("<B>%s</B>\n", ci->geneName);
printf("<BR>%s\n", ci->desc);
if (ci->refSeqAccv != NULL)
    printf("<BR><B>RefSeq</B>: %s\n", ci->refSeqAccv);
if (ci->refSeqSum != NULL)
    {
    printf("<BR><B>RefSeq Summary</B>:");
    if (!sameString(ci->refSeqSumAccv, ci->refSeqAccv))
        printf(" <EM>(summary from %s)</EM>:", ci->refSeqAccv); // no summary for best match
    printf(" %s\n", ci->refSeqSum);
    }
}

static void prCloneInfo(struct cloneInfo *ci)
/* print table of clone information */
{
webPrintLinkTableStart();
prCellLabelVal("Gene", ci->geneName);
webPrintLinkTableNewRow();
prCellLabelVal("Product", ci->productName);
webPrintLinkTableNewRow();
prCellLabelVal("Tissue", ci->tissue);
webPrintLinkTableNewRow();
prCellLabelVal("Library", ci->library);
webPrintLinkTableNewRow();
prCellLabelVal("Development", ci->development);
webPrintLinkTableNewRow();
prCellLabelVal("CDS", ci->cds);
webPrintLinkTableNewRow();
prCellLabelVal("Modification date", ci->moddate);
webPrintLinkTableEnd();
}

static void prSeqLinks(struct sqlConnection *conn, struct cloneInfo *ci)
/* print table of sequence links */
{
webNewSection("Sequences");
webPrintLinkTableStart();

webPrintLinkCellStart();
hgcAnchorSomewhere("htcDisplayMrna", ci->acc, ci->pslTbl, seqName);
printf("mRNA</a>");
webPrintLinkCellEnd();

webPrintLinkCellStart();
hgcAnchorSomewhere("htcTranslatedMRna", ci->acc, ci->pslTbl, seqName);
printf("Protein</A><br>");
webPrintLinkCellEnd();

webPrintLinkCellStart();
hgcAnchorSomewhere("htcGeneInGenome", ci->acc, ci->gpTbl, seqName);
printf("Genomic</A>");
webPrintLinkCellEnd();

webPrintLinkTableNewRow();

webPrintLinkCellStart();
hgcAnchorSomewhere("htcDisplayMrna", ci->acc, ci->gpTbl, seqName);
printf("Reference genome mRNA</A>");
webPrintLinkCellEnd();

#if BROKEN
// FIXME: doesn't work when genePred table is not the track; not that important
webPrintLinkCellStart();
hgcAnchorSomewhereTbl("htcTranslatedPredMRna", ci->acc, ci->pslTbl, seqName, ci->gpTbl);
printf("Reference genome protein</A>");
webPrintLinkCellEnd();
webFinishPartialLinkTable(1, 2, 3);
#else
webFinishPartialLinkTable(1, 1, 3);
#endif

webPrintLinkTableEnd();
}

static void prAlign(struct sqlConnection *conn, char *pslTbl, struct psl *psl)
/* print an alignment */
{
// genomic location
webPrintLinkCellStart();
printf("<A HREF=\"%s&db=%s&position=%s%%3A%d-%d\">%s:%d-%d</A>",
       hgTracksPathAndSettings(), database, psl->tName, psl->tStart+1, psl->tEnd,
       psl->tName, psl->tStart+1, psl->tEnd);
webPrintLinkCellEnd();

// genomic span
webPrintLinkCellRightStart();
printf("%d", psl->tEnd-psl->tStart);
webPrintLinkCellEnd();

// strand
webPrintLinkCell(psl->strand);

// mRNA location, linked to aligment viewer
webPrintLinkCellStart();
char other[128];
safef(other, sizeof(other), "%d&aliTable=%s", psl->tStart, pslTbl);
hgcAnchorSomewhere("htcCdnaAli", psl->qName, other, psl->tName);
printf("%s:%d-%d</A>", psl->qName, psl->qStart+1, psl->qEnd);
webPrintLinkCellEnd();

// identity
webPrintLinkCellRightStart();
printf("%.2f%%", 100.0 * pslIdent(psl));
webPrintLinkCellEnd();

// fraction aligned
webPrintLinkCellRightStart();
int aligned = psl->match + psl->misMatch + psl->repMatch;
printf("%.2f%%", 100.0*aligned/((float)psl->qSize));
webPrintLinkCellEnd();
}

static void prAligns(struct sqlConnection *conn, struct cloneInfo *ci)
/* print table of alignments */
{
struct psl* pslList = getAlignments(conn, ci->pslTbl, ci->acc);
assert(pslList != NULL);
slSort(&pslList, pslCmpMatch);

// header, print note about order only if we have multiple alignments and didn't
// come from another details page
webNewSection("Alignments");
if ((pslList->next != NULL) && (winStart < winEnd))
    printf("<span style='font-size:smaller;'><em>The alignment you clicked on is shown first.</em></span>\n");

webPrintLinkTableStart();
webPrintLabelCell("genomic (browser)");
webPrintLabelCell("span");
webPrintLabelCell("&nbsp;");
webPrintLabelCell("mRNA (alignment details)");
webPrintLabelCell("identity");
webPrintLabelCell("aligned");

// print with clicked alignment first
struct psl* psl;
int pass;
for (pass = 1; pass <= 2; pass++)
    {
    for (psl = pslList; psl != NULL; psl = psl->next)
        if ((pass == 1) == (psl->tStart == ci->start))
            {
            webPrintLinkTableNewRow();
            prAlign(conn, ci->pslTbl, psl);
            }
    }
webPrintLinkTableEnd();
}

enum gbMiscDiffFields
/* optional fields in gbMiscDiff table */
{
    gbMiscDiffNotes = 0x01,
    gbMiscDiffGene = 0x02,
    gbMiscDiffReplace = 0x04
};

static unsigned getMiscDiffFields(struct gbMiscDiff *gmds)
/* find which fields are present */
{
struct gbMiscDiff *gmd;
unsigned flds = 0;
for (gmd = gmds; gmd != NULL; gmd = gmd->next)
    {
    if (gmd->notes != NULL)
        flds |= gbMiscDiffNotes;
    if (gmd->gene != NULL)
        flds |=gbMiscDiffGene;
    if (gmd->replacement != NULL)
        flds |= gbMiscDiffReplace;
    }
return flds;
}

static void prMiscDiffHdr(unsigned miscDiffFlds)
/* print a header row for miscDiffs table */
{
webPrintLabelCell("mRNA start");
webPrintLabelCell("mRNA end");
if (miscDiffFlds & gbMiscDiffGene)
    webPrintLabelCell("Gene");
if (miscDiffFlds & gbMiscDiffReplace)
    webPrintLabelCell("Replace");
if (miscDiffFlds & gbMiscDiffNotes)
    webPrintLabelCell("Notes");
}

static void prMiscDiff(struct gbMiscDiff *gmd, unsigned miscDiffFlds)
/* print any gbMiscDiff row */
{
webPrintLinkTableNewRow();
webPrintIntCell(gmd->mrnaStart);
webPrintIntCell(gmd->mrnaEnd);
if (miscDiffFlds & gbMiscDiffGene)
    webPrintLinkCell(gmd->gene);
if (miscDiffFlds & gbMiscDiffReplace)
    webPrintLinkCell(gmd->replacement);
if (miscDiffFlds & gbMiscDiffNotes)
    webPrintLinkCell(gmd->notes);
}

static void prMiscDiffs(struct sqlConnection *conn, char *acc)
/* print any gbMiscDiff rows for the accession */
{
struct gbMiscDiff *gmds = NULL, *gmd;
if (sqlTableExists(conn, gbMiscDiffTable))
    gmds = sqlQueryObjs(conn, (sqlLoadFunc)gbMiscDiffLoad, sqlQueryMulti,
                        "select * from %s where acc=\"%s\"", gbMiscDiffTable, acc);
webNewSection("NCBI Clone Validation");
if (gmds != NULL)
    {
    unsigned miscDiffFlds = getMiscDiffFields(gmds);
    webPrintLinkTableStart();
    prMiscDiffHdr(miscDiffFlds);
    for (gmd = gmds; gmd != NULL; gmd = gmd->next)
        prMiscDiff(gmd, miscDiffFlds);
    webPrintLinkTableEnd();
    }
else
    printf("<EM>No clone discrepancies annotated</EM><BR><BR>\n");
}

static void prMethodsLink(struct sqlConnection *conn, char *track)
/* generate link to methods page */
{
webNewSection("Description and Methods");
printf("Click <A HREF=\"%s&g=htcTrackHtml&table=%s&c=%s&l=%d&r=%d\">here</A> for details",
       hgcPathAndSettings(), track, seqName, winStart, winEnd);
}

static void prOrderLink(char *name, struct cloneInfo *ci)
/* create link to NCBI clone order CGI */
{
webPrintLinkTableNewRow();
webPrintLinkCellStart();
printf("<a href=\"");
printOrderUrl(ci->gi);
printf("\" TARGET=_blank>Order %s clone</a>", name);
webPrintLinkCellEnd();
}

static void prImageLink(struct cloneInfo *ci)
/* create link to IMAGE database */
{
webPrintLinkTableNewRow();
webPrintLinkCellStart();
printf("<a href=\"");
printImageUrl(ci->imageId);
printf("\" TARGET=_blank>IMAGE clone %d</a>", ci->imageId);
webPrintLinkCellEnd();
}

static void prGenbankLink(struct cloneInfo *ci)
/* create link to Genbank database */
{
webPrintLinkTableNewRow();
webPrintLinkCellStart();
printf("<a href=\"");
printEntrezNucleotideUrl(stdout, ci->acc);
printf("\" TARGET=_blank>Genbank %s</a>", ci->acc);
webPrintLinkCellEnd();
}

static void prRefSeqLinks(struct cloneInfo *ci)
/* print link to RefSeq */
{
webPrintLinkTableNewRow();
webPrintLinkCellStart();
printf("<a href=\"");
printEntrezNucleotideUrl(stdout, ci->refSeqAccv);
printf("\" TARGET=_blank>RefSeq %s</a>", ci->refSeqAccv);
webPrintLinkCellEnd();
}

static void prCcdsLinks(struct sqlConnection *conn, struct cloneInfo *ci)
/* generate links to CCDS gene */
{
struct geneSimilarities *ccdsGenes
    = geneSimilaritiesBuildAt(conn, TRUE, ci->acc, seqName, ci->start,
                              ci->gpTbl, "ccdsGene");
if (ccdsGenes->genes != NULL)
    {
    /* just use cloest one */
    char *ccdsId = ccdsGenes->genes->gene->name;
    webPrintLinkTableNewRow();
    webPrintLinkCellStart();
    printf("<A href=\"");
    printCcdsUrl(conn, ccdsId);
    printf("\">%s</A>", ccdsId);
    webPrintLinkCellEnd();
    }
geneSimilaritiesFree(&ccdsGenes);
}

static void prUcscGenesLinks(struct sqlConnection *conn, struct cloneInfo *ci)
/* generate links to UCSC or known genes */
{
struct geneSimilarities *ucscGenes
    = geneSimilaritiesBuildAt(conn, TRUE, ci->acc, seqName, ci->start,
                              ci->gpTbl, "knownGene");
if (ucscGenes->genes != NULL)
    {
    /* just use cloest one */
    struct genePred *gene = ucscGenes->genes->gene;
    webPrintLinkTableNewRow();
    webPrintLinkCellStart();
    printf("<A href=\"../cgi-bin/hgGene?%s&db=%s&hgg_gene=%s&hgg_chrom=%s&hgg_start=%d&hgg_end=%d&hgg_type=knownGene\">UCSC Gene %s</A>",
           cartSidUrlString(cart), database, gene->name, seqName, gene->txStart, gene->txEnd, gene->name);
    webPrintLinkCellEnd();
    }
geneSimilaritiesFree(&ucscGenes);
}

static void prMgcCloneLinks(struct sqlConnection *conn, struct mgcDb *mgcDb, struct cloneInfo *ci)
/* print table of clone links */
{
webPrintLinkTableStart();
webPrintLabelCell("Links");
if (ci->gi > 0)
    prOrderLink(mgcDb->name, ci);

// link to MGC database
webPrintLinkTableNewRow();
webPrintLinkCellStart();
printf("<a href=\"");
printMgcUrl(ci->imageId);
printf("\" TARGET=_blank>%s clone database</a>", mgcDb->name);
webPrintLinkCellEnd();

prImageLink(ci);
prGenbankLink(ci);
if (ci->refSeqAccv != NULL)
    prRefSeqLinks(ci);
if (sqlTableExists(conn, "ccdsGene"))
    prCcdsLinks(conn, ci);
if (sqlTableExists(conn, "knownGene"))
    prUcscGenesLinks(conn, ci);

// Brent lab validation database
if (isInMBLabValidDb(ci->acc))
    {
    webPrintLinkTableNewRow();
    webPrintLinkCellStart();
    printf("<a href=\"");
    printMBLabValidDbUrl(ci->acc);
    printf("\" TARGET=_blank>Brent Lab Clone Validation</a>");
    webPrintLinkCellEnd();
    }
webPrintLinkTableEnd();
}

static void prMgcInfoLinks(struct sqlConnection *conn, char *acc, struct mgcDb *mgcDb,
                           struct cloneInfo *ci)
/* print clone info and links */
{
webNewSection("%s Clone Information and Links", mgcDb->name);
printf("<table border=0><tr valign=top><td>\n");
prCloneInfo(ci);
printf("<td>\n");
prMgcCloneLinks(conn, mgcDb, ci);
printf("</tr></table>\n");
}

static void prRefSeqSim(struct cloneInfo *ci, struct geneSim *gs)
/* print similarity information for a given RefSeq */
{
webPrintLinkTableNewRow();
// RefSeq acc and link
webPrintLinkCellStart();
printf("<a href=\"");
printEntrezNucleotideUrl(stdout, gs->gene->name);
printf("\" TARGET=_blank>%s</a>", gs->gene->name);
webPrintLinkCellEnd();

// link to browser
webPrintLinkCellStart();
printf("<A HREF=\"%s&db=%s&position=%s%%3A%d-%d\" target=_blank>%s:%d-%d</A>",
       hgTracksPathAndSettings(), database,
       gs->gene->chrom, gs->gene->txStart+1, gs->gene->txEnd,
       gs->gene->chrom, gs->gene->txStart+1, gs->gene->txEnd);
webPrintLinkCellEnd();

// similarity
webPrintLinkCellRightStart();
printf("%0.2f%%", 100.0*gs->sim);
webPrintLinkCellEnd();
}

static void prRefSeqSims(struct cloneInfo *ci)
/* print similarity information for RefSeqs */
{
webNewSection("RefSeq CDS isoform similarity of %s clone %s",
              cdnaInfoDbName(ci), ci->acc);
webPrintLinkTableStart();
webPrintLabelCell("RefSeq");
webPrintLabelCell("Position");
webPrintLabelCell("Similarity");
struct geneSim *gs;
for (gs = ci->refSeqs->genes; gs != NULL; gs = gs->next)
    prRefSeqSim(ci, gs);
webPrintLinkTableEnd();
printf("This table compares the similarity of the BLAT genomic alignments of "
       "the CDS of this %s clone with alignment of RefSeq mRNA CDSs.  This is a metric "
       "of the similarity of the exon structure of the mRNAs, rather than a measure of their "
       "nucleotide sequence similarity.", cdnaInfoDbName(ci));
}

void doMgcGenes(struct trackDb *tdb, char *acc)
/* Process click on a mgcGenes track. */
{
struct sqlConnection *conn = hAllocConn(database);
int start = cartInt(cart, "o");
struct mgcDb mgcDb = getMgcDb();
struct cloneInfo *ci = mgcCloneInfoLoad(conn, acc, start);

prInitialSection(ci, mgcDb.name);
printf("<BR><B>Clone Source</B>: <A href=\"");
printMgcHomeUrl(&mgcDb);
printf("\" TARGET=_blank>%s</A>\n", mgcDb.title);

prMgcInfoLinks(conn, acc, &mgcDb, ci);
if ((ci->refSeqs != NULL) && (ci->refSeqs->genes != NULL))
    prRefSeqSims(ci);
prSeqLinks(conn, ci);
prAligns(conn, ci);
prMiscDiffs(conn, acc);
prMethodsLink(conn, tdb->track);

cloneInfoFree(&ci);
hFreeConn(&conn);
}

static void prOrfeomeCloneLinks(struct sqlConnection *conn, char *acc, struct cloneInfo *ci)
/* print table of clone links */
{
webPrintLinkTableStart();
webPrintLabelCell("Links");
webPrintLinkTableNewRow();
if (ci->gi > 0)
    prOrderLink("ORFeome", ci);

#if 0
// link to ORFeome database
// FIXME: this doesn't appear to work, need to ask Christa
// http://www.orfeomecollaboration.org/bin/cloneStatus.pl
#endif

if (ci->imageId > 0)
    prImageLink(ci);
prGenbankLink(ci);
if (ci->refSeqAccv != NULL)
    prRefSeqLinks(ci);
if (sqlTableExists(conn, "ccdsGene"))
    prCcdsLinks(conn, ci);
if (sqlTableExists(conn, "knownGene"))
    prUcscGenesLinks(conn, ci);

webPrintLinkTableEnd();
}

static void prOrfeomeInfoLinks(struct sqlConnection *conn, char *acc, struct cloneInfo *ci)
/* print clone info and links */
{
webNewSection("ORFeome Clone Information and Links");
printf("<table border=0><tr valign=top><td>\n");
prCloneInfo(ci);
printf("<td>\n");
prOrfeomeCloneLinks(conn, acc, ci);
printf("</tr></table>\n");
}

void doOrfeomeGenes(struct trackDb *tdb, char *acc)
/* Process click on a orfeomeGenes track. */
{
struct sqlConnection *conn = hAllocConn(database);
int start = cartInt(cart, "o");
struct cloneInfo *ci = orfeomeCloneInfoLoad(conn, acc, start);

// initial section
prInitialSection(ci, "ORFeome");
printf("<BR><B>Clone Source</B>: <A href=\"http://www.orfeomecollaboration.org/\""
       " TARGET=_blank>ORFeome collaboration</A>\n");

prOrfeomeInfoLinks(conn, acc, ci);
if ((ci->refSeqs != NULL) && (ci->refSeqs->genes != NULL))
    prRefSeqSims(ci);
prSeqLinks(conn, ci);
prAligns(conn, ci);
prMiscDiffs(conn, acc);
prMethodsLink(conn, tdb->track);
cloneInfoFree(&ci);
hFreeConn(&conn);
}
