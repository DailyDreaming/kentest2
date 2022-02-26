/* regMotif.c - stuff for displaying details of regulatory
 * motifs and the like. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "dnaseq.h"
#include "jksql.h"
#include "hCommon.h"
#include "hdb.h"
#include "web.h"
#include "portable.h"
#include "cart.h"
#include "trackDb.h"
#include "hgc.h"
#include "genePred.h"
#include "dnaMotif.h"
#include "dnaMotifSql.h"
#include "growthCondition.h"
#include "transRegCode.h"
#include "transRegCodeProbe.h"
#include "flyreg.h"
#include "flyreg2.h"

static void printDnaCells(char *dna, int size)
/* Print string with each letter in a separate cell. */
{
int i;
for (i=0; i<size; ++i)
    printf("<td align='center'>%c</td>", dna[i]);
}

static void printConsensus(struct dnaMotif *motif)
/* Print motif - use bold caps for strong letters, then
 * caps, then small letters, then .'s */
{
int i, size = motif->columnCount;
char c;
float best;
for (i=0; i<size; ++i)
    {
    c = 'a';
    best = motif->aProb[i];
    if (motif->cProb[i] > best)
	{
	best = motif->cProb[i];
	c = 'c';
	}
    if (motif->gProb[i] > best)
	{
	best = motif->gProb[i];
	c = 'g';
	}
    if (motif->tProb[i] > best)
	{
	best = motif->tProb[i];
	c = 't';
	}
    printf("<td align='center'>");
    if (best >= 0.90)
	printf("<B>%c</B>", toupper(c));
    else if (best >= 0.75)
        printf("%c", toupper(c));
    else if (best >= 0.50)
        printf("%c", tolower(c));
    else if (best >= 0.40)
        printf("<span style='color:#A0A0A0;'>%c</span>", tolower(c));
    else
        printf("<span style='color:#A0A0A0;'>.</span>");
    printf("</td>");
    }
}

static void printProbRow(FILE *f, char *label, float *p, int pCount)
/* Print one row of a probability profile. */
{
int i;
fprintf(f, "<tr><td width='20' align='center'>%s</td>", label);
for (i=0; i < pCount; ++i)
    fprintf(f, "<td align='center'>%5.2f</td>", p[i]);
printf("<td></td></tr>\n");
}

static void dnaMotifPrintProbTable(struct dnaMotif *motif, FILE *f)
/* Print DNA motif probabilities. */
{
printProbRow(f, "A", motif->aProb, motif->columnCount);
printProbRow(f, "C", motif->cProb, motif->columnCount);
printProbRow(f, "G", motif->gProb, motif->columnCount);
printProbRow(f, "T", motif->tProb, motif->columnCount);
}

struct dnaMotif *loadDnaMotif(char *motifName, char *motifTable)
/* Load dnaMotif from table. */
{
struct sqlConnection *conn = hAllocConn(database);
char query[256];
struct dnaMotif *motif;
sqlSafefFrag(query, sizeof query, "name = '%s'", motifName);
motif = dnaMotifLoadWhere(conn, motifTable, query);
hFreeConn(&conn);
return motif;
}

void motifLogoAndMatrix(struct dnaSeq **seqs, int count, struct dnaMotif *motif)
/* Print out motif sequence logo and text (possibly with multiple occurences) */
{
// Detect inconsistent motif/pwm tables and suppress confusing display
if (motif != NULL)
    {
    if (seqs != NULL && motif->columnCount != seqs[0]->size)
        {
        warn("Motif seq length doesn't match PWM\n");
        return;
        }
    }
#define MOTIF_HELP_PAGE "../goldenPath/help/hgRegMotifHelp.html"
printf("<PRE>\n");
printf("<table>\n");
if (motif != NULL)
    {
    struct tempName pngTn;
    dnaMotifMakeProbabalistic(motif);
    makeTempName(&pngTn, "logo", ".png");
    dnaMotifToLogoPng(motif, 47, 140, NULL, "../trash", pngTn.forCgi);
    printf("<tr><td></td><td colspan='%d'align=right><a href=\"%s\" target=_blank>Motif display help</a></td></tr>", 
        motif->columnCount, MOTIF_HELP_PAGE);
    printf("<tr><td></td><td colspan='%d'>", motif->columnCount);
    printf("<IMG SRC=\"%s\" BORDER=1>", pngTn.forHtml);
    printf("</td><td></td></tr>\n");
    }
if (count > 0)
    {
    int i;
    for (i = 0; i < count; i++)
        {
        struct dnaSeq *seq = seqs[i];
        printf("<tr><td></td>");
        touppers(seq->dna);
        printDnaCells(seq->dna, seq->size);
        if (count == 1)
            printf("<td>this occurrence</td></tr>\n");
        else
            // is there a library routine to get 1st, 2nd ...?
            printf("<td>occurrence #%d</td></tr>\n", i + 1);
        }
    }
if (motif != NULL)
    {
    printf("<tr><td></td>");
    printConsensus(motif);
    printf("<td>motif consensus</td></tr>\n");
    dnaMotifPrintProbTable(motif, stdout);
    }
printf("</table>\n");
printf("</PRE>");
}

void motifMultipleHitsSection(struct dnaSeq **seqs, int count, struct dnaMotif *motif, char *title)
/* Print out section about motif, possibly with mutliple occurrences. */
{
// Detect inconsistent motif/pwm tables and suppress confusing display
if (motif != NULL)
    {
    if (seqs != NULL && motif->columnCount != seqs[0]->size)
        {
        warn("Motif seq length doesn't match PWM\n");
        return;
        }
    }
webNewSection("%s",title);
motifLogoAndMatrix(seqs, count, motif);
}

void motifHitSection(struct dnaSeq *seq, struct dnaMotif *motif)
/* Print out section about motif. */
{
static char *title = "Motif Sequence";
if(seq == NULL)
    motifMultipleHitsSection(NULL, 0, motif, title);
else
    motifMultipleHitsSection(&seq, 1, motif, title);
}

void doTriangle(struct trackDb *tdb, char *item, char *motifTable)
/* Display detailed info on a regulatory triangle item. */
{
int start = cartInt(cart, "o");
struct dnaSeq *seq = NULL;
struct dnaMotif *motif = loadDnaMotif(item, motifTable);
char *table = tdb->table;
int rowOffset = hOffsetPastBin(database, seqName, table);
char query[256];
struct sqlResult *sr;
char **row;
struct bed *hit = NULL;
struct sqlConnection *conn = hAllocConn(database);

cartWebStart(cart, database, "Regulatory Motif Info");
genericBedClick(conn, tdb, item, start, 6);

sqlSafef(query, sizeof query,
	"select * from %s where  name = '%s' and chrom = '%s' and chromStart = %d",
	table, item, seqName, start);
sr = sqlGetResult(conn, query);
row = sqlNextRow(sr);
if (row != NULL)
    hit = bedLoadN(row + rowOffset, 6);
sqlFreeResult(&sr);

if (hit != NULL)
    {
    seq = hDnaFromSeq(database, hit->chrom, hit->chromStart, hit->chromEnd, dnaLower);
    if (hit->strand[0] == '-')
	reverseComplement(seq->dna, seq->size);
    }
motifHitSection(seq, motif);
printTrackHtml(tdb);
}

void doFlyreg(struct trackDb *tdb, char *item)
/* flyreg.org: Drosophila DNase I Footprint db. */
{
struct dyString *query = newDyString(256);
struct sqlConnection *conn = hAllocConn(database);
struct sqlResult *sr = NULL;
char **row;
int start = cartInt(cart, "o");
int end   = cartInt(cart, "t");
char fullTable[HDB_MAX_TABLE_STRING];
boolean hasBin = FALSE;
char *motifTable = "flyregMotif";
struct dnaMotif *motif = NULL;
boolean isVersion2 = sameString(tdb->table, "flyreg2");

genericHeader(tdb, item);
if (!hFindSplitTable(database, seqName, tdb->table, fullTable, sizeof fullTable, &hasBin))
    errAbort("track %s not found", tdb->table);
sqlDyStringPrintf(query, "select * from %s where chrom = '%s' and ",
	       fullTable, seqName);
hAddBinToQuery(start, end, query);
sqlDyStringPrintf(query, "chromStart = %d and name = '%s'", start, item);
sr = sqlGetResult(conn, query->string);
if ((row = sqlNextRow(sr)) != NULL)
    {
    struct flyreg2 fr;
    if (isVersion2)
	flyreg2StaticLoad(row+hasBin, &fr);
    else
	flyregStaticLoad(row+hasBin, (struct flyreg *)(&fr));
    printf("<B>Factor:</B> %s<BR>\n", fr.name);
    printf("<B>Target:</B> %s<BR>\n", fr.target);
    if (isVersion2)
	printf("<B>Footprint ID:</B> %06d<BR>\n", fr.fpid);
    printf("<B>PubMed ID:</B> <A HREF=\"");
    printEntrezPubMedUidUrl(stdout, fr.pmid);
    printf("\" TARGET=_BLANK>%d</A><BR>\n", fr.pmid);
    bedPrintPos((struct bed *)(&fr), 3, tdb);
    if (hTableExists(database, motifTable))
	{
	motif = loadDnaMotif(item, motifTable);
	if (motif != NULL)
	    motifHitSection(NULL, motif);
	}
    }
else
    errAbort("query returned no results: \"%s\"", query->string);
dyStringFree(&query);
sqlFreeResult(&sr);
hFreeConn(&conn);
if (motif != NULL)
    webNewSection("%s",tdb->longLabel);
printTrackHtml(tdb);
}


static void wrapHgGeneLink(struct sqlConnection *conn, char *name,
	char *label, char *geneTable)
/* Wrap label with link to hgGene if possible. */
{
char query[256];
struct sqlResult *sr;
char **row;
int rowOffset = hOffsetPastBin(database, seqName, "sgdGene");
sqlSafef(query, sizeof(query),
    "select * from %s where name = '%s'", geneTable, name);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) != NULL)
    {
    struct genePred *gp = genePredLoad(row+rowOffset);
    printf("<A HREF=\"../cgi-bin/hgGene?db=%s", database);
    printf("&hgg_gene=%s", gp->name);
    printf("&hgg_chrom=%s", gp->chrom);
    printf("&hgg_start=%d", gp->txStart);
    printf("&hgg_end=%d", gp->txEnd);
    printf("\">");
    printf("%s", label);
    printf("</A>");
    }
else
    printf("%s", label);
sqlFreeResult(&sr);
}

static void transRegCodeAnchor(struct transRegCode *trc)
/* Print anchor to transRegCode details page. */
{
printf("<A HREF=\"../cgi-bin/hgc?%s", cartSidUrlString(cart));
printf("&g=transRegCode");
printf("&i=%s", trc->name);
printf("&o=%d", trc->chromStart);
printf("&c=%s", trc->chrom);
printf("\">");
}

static void sacCerHgGeneLinkName(struct sqlConnection *conn, char *name)
/* Wrap link to hgGene if possible around yeast gene name. */
{
char query[256];
char *orf;
sqlSafef(query, sizeof(query),
	"select name from sgdToName where value = '%s'", name);
orf = sqlQuickString(conn, query);
if (orf != NULL)
    wrapHgGeneLink(conn, orf, name, "sgdGene");
else
    printf("%s", name);
freez(&orf);
}

void doTransRegCode(struct trackDb *tdb, char *item, char *motifTable)
/* Display detailed info on a transcriptional regulatory code item. */
{
struct dnaMotif *motif = loadDnaMotif(item, motifTable);
int start = cartInt(cart, "o");
struct dnaSeq *seq = NULL;
char *table = tdb->table;
int rowOffset = hOffsetPastBin(database, seqName, table);
char query[256];
struct sqlResult *sr;
char **row;
struct sqlConnection *conn = hAllocConn(database);
struct transRegCode *trc = NULL;

cartWebStart(cart, database, "Regulatory Code Info");
sqlSafef(query, sizeof query,
	"select * from %s where  name = '%s' and chrom = '%s' and chromStart = %d",
	table, item, seqName, start);
sr = sqlGetResult(conn, query);
row = sqlNextRow(sr);
if (row != NULL)
    trc = transRegCodeLoad(row+rowOffset);
sqlFreeResult(&sr);

if (trc != NULL)
    {
    char strand[2];
    seq = hDnaFromSeq(database, trc->chrom, trc->chromStart, trc->chromEnd, dnaLower);
    if (seq->size != motif->columnCount)
	{
        printf("WARNING: seq->size = %d, motif->colCount=%d<BR>\n",
		seq->size, motif->columnCount);
	strand[0] = '?';
	seq = NULL;
	}
    else
	{
	strand[0] = dnaMotifBestStrand(motif, seq->dna);
	if (strand[0] == '-')
	    reverseComplement(seq->dna, seq->size);
	}
    strand[1] = 0;
    printf("<B>Name:</B> ");
    sacCerHgGeneLinkName(conn, trc->name);
    printf("<BR>\n");
    printf("<B>ChIP-chip Evidence:</B> %s<BR>\n", trc->chipEvidence);
    printf("<B>Species conserved in:</B> %d of 2<BR>\n", trc->consSpecies);
    if (seq != NULL)
	printf("<B>Bit Score of Motif Hit:</B> %4.2f<BR>\n",
	    dnaMotifBitScore(motif, seq->dna));
    printf("<B>Item score:</B> %d<BR>\n", trc->score);
    printPosOnChrom(trc->chrom, trc->chromStart, trc->chromEnd, strand, TRUE, trc->name);
    }
motifHitSection(seq, motif);
printTrackHtml(tdb);
}

static double motifScoreHere(char *chrom, int start, int end,
	char *motifName, char *motifTable)
/* Return score of motif at given position. */
{
double score;
struct dnaSeq *seq = hDnaFromSeq(database, chrom, start, end, dnaLower);
struct dnaMotif *motif = loadDnaMotif(motifName, motifTable);
char strand = dnaMotifBestStrand(motif, seq->dna);
if (strand == '-')
    reverseComplement(seq->dna, seq->size);
score = dnaMotifBitScore(motif, seq->dna);
dnaMotifFree(&motif);
dnaSeqFree(&seq);
return score;
}


static void colLabel(char *label, int columns)
/* Print out label of given width. */
{
printf("<TH BGCOLOR=#%s", HG_COL_TABLE);
if (columns > 1)
    printf(" COLSPAN=%d", columns);
printf(">%s</TH>", label);
}

struct tfCond
/* Condition tested under. */
   {
   struct tfCond *next;
   char *name;	/* Condition name. */
   double binding;	/* Binding e-val. */
   };

static int tfCondCmpName(const void *va, const void *vb)
/* Compare two tfData names. */
{
const struct tfCond *a = *((struct tfCond **)va);
const struct tfCond *b = *((struct tfCond **)vb);
return strcmp(a->name, b->name);
}


struct tfData
/* Data associated with one transcription factor. */
   {
   struct tfData *next;
   char *name;	/* Transcription factor name. */
   struct tfCond *conditionList;	/* List of growth conditions. */
   struct transRegCode *trcList;	/* List of binding sites. */
   };

static int tfDataCmpName(const void *va, const void *vb)
/* Compare two tfData names. */
{
const struct tfData *a = *((struct tfData **)va);
const struct tfData *b = *((struct tfData **)vb);
return strcmp(a->name, b->name);
}

static void ipPrintInRange(struct tfCond *condList,
	double minVal, double maxVal, struct hash *boundHash)
/* Print growth conditions that bind within range of E values.
 * Add these to boundHash. */
{
struct tfCond *cond;
boolean isFirst = TRUE;
boolean gotAny = FALSE;

printf("<TD>");
for (cond = condList; cond != NULL; cond = cond->next)
    {
    if (minVal <= cond->binding && cond->binding < maxVal)
	{
	if (isFirst)
	    isFirst = FALSE;
	else
	    printf(", ");
	printf("%s", cond->name);
	hashAdd(boundHash, cond->name, NULL);
	gotAny = TRUE;
	}
    }
if (!gotAny)
    printf("&nbsp;");
printf("</TD>");
}

static void tfBindLevelSection(struct tfData *tfList, struct sqlConnection *conn,
	char *motifTable, char *tfToConditionTable)
/* Print info on individual transcription factors that bind
 * with e-val between minVal and maxVal. */
{
struct tfData  *tf;
struct transRegCode *trc;

webNewSection("Transcription Factors Showing IP Over this Probe ");
hTableStart();
printf("<TR>");
colLabel("Transcription", 1);
colLabel("Growth Condition", 3);
colLabel("Motif Information", 3);
printf("</TR>\n");
printf("<TR>");
colLabel("Factor", 1);
colLabel("Good IP (P<0.001)", 1);
colLabel("Weak IP (P<0.005)", 1);
colLabel("No IP (P>0.005)", 1);
colLabel("Hits", 1);
colLabel("Scores", 1);
colLabel("Conservation (2 max)", 1);
printf("</TR>\n");

for (tf = tfList; tf != NULL; tf = tf->next)
    {
    struct hash *boundHash = newHash(8);
    slSort(&tf->conditionList, tfCondCmpName);
    printf("<TR>");

    /* Print transcription name. */
    printf("<TD>");
    sacCerHgGeneLinkName(conn, tf->name);
    printf("</TD>");

    /* Print stong and weak growth conditions. */
    ipPrintInRange(tf->conditionList, 0.0, 0.002, boundHash);
    ipPrintInRange(tf->conditionList, 0.002, 0.006, boundHash);

    /* Grab list of all conditions tested from database and
     * print out ones not in strong or weak as none. */
         {
	 char query[256], **row;
	 struct sqlResult *sr;
	 boolean isFirst = TRUE;
	 boolean gotAny = FALSE;
	 sqlSafef(query, sizeof(query),
	 	"select growthCondition from %s where name='%s'",
		tfToConditionTable, tf->name);
	 sr = sqlGetResult(conn, query);
	 printf("<TD>");
	 while ((row = sqlNextRow(sr)) != NULL)
	     {
	     if (!hashLookup(boundHash, row[0]))
	         {
		 if (isFirst)
		     isFirst = FALSE;
		 else
		     printf(", ");
		 printf("%s", row[0]);
		 gotAny = TRUE;
		 }
	     }
	 sqlFreeResult(&sr);
	if (!gotAny)
	    printf("&nbsp;");
	 printf("</TD>");
	 }


    /* Print motif info. */
    if (tf->trcList == NULL)
	printf("<TD>0</TD><TD>n/a</TD><TD>n/a</TD>\n");
    else
	{
	printf("<TD>%d</TD>", slCount(tf->trcList));
	/* Print scores. */
	printf("<TD>");
	for (trc = tf->trcList; trc != NULL; trc = trc->next)
	    {
	    double score;
	    if (trc != tf->trcList)
		printf(", ");
	    score = motifScoreHere(
		trc->chrom, trc->chromStart, trc->chromEnd,
		trc->name, motifTable);
	    transRegCodeAnchor(trc);
	    printf("%3.1f</A>", score);
	    }
	printf("</TD><TD>");
	for (trc = tf->trcList; trc != NULL; trc = trc->next)
	    {
	    if (trc != tf->trcList)
		printf(", ");
	    printf("%d", trc->consSpecies);
	    }
	printf("</TD>");
	}
    printf("</TR>\n");
    hashFree(&boundHash);
    }
hTableEnd();
}

void growthConditionSection(struct sqlConnection *conn, char *conditionTable)
/* Print out growth condition information. */
{
struct sqlResult *sr;
char query[256], **row;
webNewSection("Description of Growth Conditions");
sqlSafef(query, sizeof(query), "select * from %s order by name", conditionTable);
sr = sqlGetResult(conn, query);
printf("<UL>");
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct growthCondition gc;
    growthConditionStaticLoad(row, &gc);
    printf("<LI>");
    printf("<A NAME=\"GC_%s\"></A>", gc.name);
    printf("%s - <I>%s</I> %s\n",  gc.name, gc.shortLabel, gc.longLabel);
    }
printf("</UL>");
sqlFreeResult(&sr);
}

void doTransRegCodeProbe(struct trackDb *tdb, char *item,
	char *codeTable, char *motifTable,
	char *tfToConditionTable, char *conditionTable)
/* Display detailed info on a ChIP-chip probe from transRegCode experiments. */
{
char query[256];
struct sqlResult *sr;
char **row;
int rowOffset = hOffsetPastBin(database, seqName, tdb->table);
struct sqlConnection *conn = hAllocConn(database);
struct transRegCodeProbe *probe = NULL;

cartWebStart(cart, database, "ChIP-chip Probe Info");
sqlSafef(query, sizeof(query), "select * from %s where name = '%s'",
	tdb->table, item);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) != NULL)
    probe = transRegCodeProbeLoad(row+rowOffset);
sqlFreeResult(&sr);
if (probe != NULL)
    {
    struct tfData *tfList = NULL, *tf;
    struct hash *tfHash = newHash(0);
    struct transRegCode *trc;
    int i;

    /* Print basic info. */
    printf("<B>Name:</B> %s<BR>\n", probe->name);
    printPosOnChrom(probe->chrom, probe->chromStart, probe->chromEnd,
    	NULL, TRUE, probe->name);

    /* Make up list of all transcriptionFactors. */
    for (i=0; i<probe->tfCount; ++i)
        {
	/* Parse out factor and condition. */
	char *tfName = probe->tfList[i];
	char *condition = strchr(tfName, '_');
	struct tfCond *cond;
	if (condition != NULL)
	    *condition++ = 0;
	else
	    condition = "n/a";
	tf = hashFindVal(tfHash, tfName);
	if (tf == NULL)
	    {
	    AllocVar(tf);
	    hashAddSaveName(tfHash, tfName, tf, &tf->name);
	    slAddHead(&tfList, tf);
	    }
	AllocVar(cond);
	cond->name = cloneString(condition);
	cond->binding = probe->bindVals[i];
	slAddHead(&tf->conditionList, cond);
	}
    slSort(&tfList, tfDataCmpName);

    /* Fold in motif hits in region. */
    if (sqlTableExists(conn, codeTable))
        {
	sr = hRangeQuery(conn, codeTable,
		probe->chrom, probe->chromStart, probe->chromEnd,
		"chipEvidence != 'none'", &rowOffset);
	while ((row = sqlNextRow(sr)) != NULL)
	    {
	    trc = transRegCodeLoad(row+rowOffset);
	    tf = hashFindVal(tfHash, trc->name);
	    if (tf != NULL)
		slAddTail(&tf->trcList, trc);
	    }
	sqlFreeResult(&sr);
	}
    if (tfList == NULL)
	printf("No significant immunoprecipitation.");
    else
	{
	tfBindLevelSection(tfList, conn, motifTable, tfToConditionTable);
	}
    transRegCodeProbeFree(&probe);
    growthConditionSection(conn, conditionTable);
    }
printf("\n<HR>\n");
printTrackHtml(tdb);
hFreeConn(&conn);
}
