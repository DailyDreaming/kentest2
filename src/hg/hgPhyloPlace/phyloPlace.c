/* Place SARS-CoV-2 sequences in phylogenetic tree using usher program. */

/* Copyright (C) 2020 The Regents of the University of California */

#include "common.h"
#include "bigBed.h"
#include "cheapcgi.h"
#include "errCatch.h"
#include "fa.h"
#include "genePred.h"
#include "hCommon.h"
#include "hash.h"
#include "hgConfig.h"
#include "htmshell.h"
#include "hui.h"
#include "iupac.h"
#include "jsHelper.h"
#include "linefile.h"
#include "obscure.h"
#include "parsimonyProto.h"
#include "phyloPlace.h"
#include "phyloTree.h"
#include "pipeline.h"
#include "psl.h"
#include "ra.h"
#include "regexHelper.h"
#include "trashDir.h"
#include "vcf.h"

// Globals:
static boolean measureTiming = FALSE;

// wuhCor1-specific:
char *chrom = "NC_045512v2";
int chromSize = 29903;

// Parameter constants:
int maxGenotypes = 1000;        // Upper limit on number of samples user can upload at once.
boolean showParsimonyScore = FALSE;


char *phyloPlaceDbSetting(char *db, char *settingName)
/* Return a setting from hgPhyloPlaceData/<db>/config.ra or NULL if not found. */
{
static struct hash *configHash = NULL;
static char *configDb = NULL;
if (!sameOk(db, configDb))
    {
    char configFile[1024];
    safef(configFile, sizeof configFile, PHYLOPLACE_DATA_DIR "/%s/config.ra", db);
    if (fileExists(configFile))
        {
        configHash = raReadSingle(configFile);
        configDb = cloneString(db);
        }
    }
if (sameOk(db, configDb))
    return cloneString(hashFindVal(configHash, settingName));
return NULL;
}

char *phyloPlaceDbSettingPath(char *db, char *settingName)
/* Return path to a file named by a setting from hgPhyloPlaceData/<db>/config.ra,
 * or NULL if not found.  (Append hgPhyloPlaceData/<db>/ to the beginning of relative path) */
{
char *fileName = phyloPlaceDbSetting(db, settingName);
if (isNotEmpty(fileName) && fileName[0] != '/' && !fileExists(fileName))
    {
    struct dyString *dy = dyStringCreate(PHYLOPLACE_DATA_DIR "/%s/%s", db, fileName);
    if (fileExists(dy->string))
        return dyStringCannibalize(&dy);
    else
        return NULL;
    }
return fileName;
}

char *getUsherPath(boolean abortIfNotFound)
/* Return hgPhyloPlaceData/usher if it exists, else NULL.  Do not free the returned value. */
{
char *usherPath = PHYLOPLACE_DATA_DIR "/usher";
if (fileExists(usherPath))
    return usherPath;
else if (abortIfNotFound)
    errAbort("Missing usher executable (expected to be at %s)", usherPath);
return NULL;
}

char *getMatUtilsPath(boolean abortIfNotFound)
/* Return hgPhyloPlaceData/matUtils if it exists, else NULL.  Do not free the returned value. */
{
char *matUtilsPath = PHYLOPLACE_DATA_DIR "/matUtils";
if (fileExists(matUtilsPath))
    return matUtilsPath;
else if (abortIfNotFound)
    errAbort("Missing matUtils executable (expected to be at %s)", matUtilsPath);
return NULL;
}

char *getUsherAssignmentsPath(char *db, boolean abortIfNotFound)
/* If <db>/config.ra specifies the file for use by usher --load-assignments and the file exists,
 * return the path, else NULL.  Do not free the returned value. */
{
char *usherAssignmentsPath = phyloPlaceDbSettingPath(db, "usherAssignmentsFile");
if (isNotEmpty(usherAssignmentsPath) && fileExists(usherAssignmentsPath))
    return usherAssignmentsPath;
else if (abortIfNotFound)
    errAbort("Missing usher protobuf file (config setting in "
             PHYLOPLACE_DATA_DIR "/%s/config.ra = %s", db, usherAssignmentsPath);
return NULL;
}

//#*** This needs to go in a lib so CGIs know whether to include it in the menu. needs better name.
boolean hgPhyloPlaceEnabled()
/* Return TRUE if hgPhyloPlace is enabled in hg.conf and db wuhCor1 exists. */
{
char *cfgSetting = cfgOption("hgPhyloPlaceEnabled");
boolean isEnabled = (isNotEmpty(cfgSetting) &&
                     differentWord(cfgSetting, "off") && differentWord(cfgSetting, "no"));
return (isEnabled && hDbExists("wuhCor1"));
}

static void addPathIfNecessary(struct dyString *dy, char *db, char *fileName)
/* If fileName exists, copy it into dy, else try hgPhyloPlaceData/<db>/fileName */
{
dyStringClear(dy);
if (fileExists(fileName))
    dyStringAppend(dy, fileName);
else
    dyStringPrintf(dy, PHYLOPLACE_DATA_DIR "/%s/%s", db, fileName);
}

struct treeChoices *loadTreeChoices(char *db)
/* If <db>/config.ra specifies a treeChoices file, load it up, else return NULL. */
{
struct treeChoices *treeChoices = NULL;
char *filename = phyloPlaceDbSettingPath(db, "treeChoices");
if (isNotEmpty(filename) && fileExists(filename))
    {
    AllocVar(treeChoices);
    int maxChoices = 128;
    AllocArray(treeChoices->protobufFiles, maxChoices);
    AllocArray(treeChoices->metadataFiles, maxChoices);
    AllocArray(treeChoices->sources, maxChoices);
    AllocArray(treeChoices->descriptions, maxChoices);
    AllocArray(treeChoices->aliasFiles, maxChoices);
    struct lineFile *lf = lineFileOpen(filename, TRUE);
    char *line;
    while (lineFileNextReal(lf, &line))
        {
        char *words[6];
        int wordCount = chopTabs(line, words);
        lineFileExpectAtLeast(lf, 4, wordCount);
        if (treeChoices->count >= maxChoices)
            {
            warn("File %s has too many lines, only showing first %d phylogenetic tree choices",
                 filename, maxChoices);
            break;
            }
        struct dyString *dy = dyStringNew(0);
        addPathIfNecessary(dy, db, words[0]);
        treeChoices->protobufFiles[treeChoices->count] = cloneString(dy->string);
        addPathIfNecessary(dy, db, words[1]);
        treeChoices->metadataFiles[treeChoices->count] = cloneString(dy->string);
        treeChoices->sources[treeChoices->count] = cloneString(words[2]);
        // Description can be either a file or just some text.
        addPathIfNecessary(dy, db, words[3]);
        if (fileExists(dy->string))
            {
            char *desc = NULL;
            readInGulp(dy->string, &desc, NULL);
            treeChoices->descriptions[treeChoices->count] = desc;
            }
        else
            treeChoices->descriptions[treeChoices->count] = cloneString(words[3]);
        if (wordCount > 4)
            {
            addPathIfNecessary(dy, db, words[4]);
            treeChoices->aliasFiles[treeChoices->count] = cloneString(dy->string);
            }
        treeChoices->count++;
        dyStringFree(&dy);
        }
    lineFileClose(&lf);
    }
return treeChoices;
}

static char *urlFromTn(struct tempName *tn)
/* Make a full URL to a trash file that our net.c code will be able to follow, for when we can't
 * just leave it up to the user's web browser to do the right thing with "../". */
{
struct dyString *dy = dyStringCreate("%s%s", hLocalHostCgiBinUrl(), tn->forHtml);
return dyStringCannibalize(&dy);
}

void reportTiming(int *pStartTime, char *message)
/* Print out a report to stderr of how much time something took. */
{
if (measureTiming)
    {
    int now = clock1000();
    fprintf(stderr, "%dms to %s\n", now - *pStartTime, message);
    *pStartTime = now;
    }
}

static boolean lfLooksLikeFasta(struct lineFile *lf)
/* Peek at file to see if it looks like FASTA, i.e. begins with a >header. */
{
boolean hasFastaHeader = FALSE;
char *line;
if (lineFileNext(lf, &line, NULL))
    {
    if (line[0] == '>')
        hasFastaHeader = TRUE;
    lineFileReuse(lf);
    }
return hasFastaHeader;
}

static void rInformativeBasesFromTree(struct phyloTree *node, boolean *informativeBases)
/* For each variant associated with a non-leaf node, set informativeBases[chromStart]. */
{
if (node->numEdges > 0)
    {
    if (node->priv)
        {
        struct singleNucChange *snc, *sncs = node->priv;
        for (snc = sncs;  snc != NULL;  snc = snc->next)
            informativeBases[snc->chromStart] = TRUE;
        }
    int i;
    for (i = 0;  i < node->numEdges;  i++)
        rInformativeBasesFromTree(node->edges[i], informativeBases);
    }
}

static boolean *informativeBasesFromTree(struct phyloTree *bigTree, struct slName **maskSites)
/* Return an array indexed by reference position with TRUE at positions that have a non-leaf
 * variant in bigTree.  If a position is in maskSites but is informative in bigTree then remove
 * it from maskSites because it was not masked (yet) back when the tree was built. */
{
boolean *informativeBases;
AllocArray(informativeBases, chromSize);
if (bigTree)
    {
    rInformativeBasesFromTree(bigTree, informativeBases);
    int i;
    for (i = 0;  i < chromSize;  i++)
        {
        if (maskSites[i] && informativeBases[i])
            maskSites[i] = NULL;
        }
    }
return informativeBases;
}

static boolean lfLooksLikeVcf(struct lineFile *lf)
/* Peek at file to see if it looks like VCF, i.e. begins with a ##fileformat=VCF header. */
{
boolean hasVcfHeader = FALSE;
char *line;
if (lineFileNext(lf, &line, NULL))
    {
    if (startsWith("##fileformat=VCF", line))
        hasVcfHeader = TRUE;
    lineFileReuse(lf);
    }
return hasVcfHeader;
}

static struct tempName *checkAndSaveVcf(struct lineFile *lf, struct dnaSeq *refGenome,
                                        struct slName **maskSites, struct seqInfo **retSeqInfoList,
                                        struct slName **retSampleIds)
/* Save the contents of lf to a trash file.  If it has a reasonable number of genotype columns
 * with recognizable genotypes, and the coordinates seem to be in range, then return the path
 * to the trash file.  Otherwise complain and return NULL. */
{
struct tempName *tn;
AllocVar(tn);
trashDirFile(tn, "ct", "ct_pp", ".vcf");
FILE *f = mustOpen(tn->forCgi, "w");
struct seqInfo *seqInfoList = NULL;
struct slName *sampleIds = NULL;
struct errCatch *errCatch = errCatchNew();
if (errCatchStart(errCatch))
    {
    char *line;
    int lineSize;
    int sampleCount = 0;
    while (lineFileNext(lf, &line, &lineSize))
        {
        if (startsWith("#CHROM\t", line))
            {
            //#*** TODO: if the user uploads a sample with the same ID as one already in the
            //#*** saved assignment file, then usher will ignore it!
            //#*** Better check for that and warn the user.
            int colCount = chopTabs(line, NULL);
            if (colCount == 1)
                {
                lineFileAbort(lf, "VCF requires tab-separated columns, but no tabs found");
                }
            sampleCount = colCount - VCF_NUM_COLS_BEFORE_GENOTYPES;
            if (sampleCount < 1 || sampleCount > maxGenotypes)
                {
                if (sampleCount < 1)
                    lineFileAbort(lf, "VCF header #CHROM line has %d columns; expecting at least %d "
                                  "columns including sample IDs for genotype columns",
                                  colCount, 10);
                else
                    lineFileAbort(lf, "VCF header #CHROM line defines %d samples but only up to %d "
                                  "are supported",
                                  sampleCount, maxGenotypes);
                }
            char lineCopy[lineSize+1];
            safecpy(lineCopy, sizeof lineCopy, line);
            char *words[colCount];
            chopTabs(lineCopy, words);
            struct hash *uniqNames = hashNew(0);
            int i;
            for (i = VCF_NUM_COLS_BEFORE_GENOTYPES;  i < colCount;  i++)
                {
                if (hashLookup(uniqNames, words[i]))
                    lineFileAbort(lf, "VCF sample names in #CHROM line must be unique, but '%s' "
                                  "appears more than once", words[i]);
                hashAdd(uniqNames, words[i], NULL);
                slNameAddHead(&sampleIds, words[i]);
                struct seqInfo *si;
                AllocVar(si);
                si->seq = cloneDnaSeq(refGenome);
                si->seq->name = cloneString(words[i]);
                slAddHead(&seqInfoList, si);
                }
            slReverse(&seqInfoList);
            slReverse(&sampleIds);
            hashFree(&uniqNames);
            fputs(line, f);
            fputc('\n', f);
            }
        else if (line[0] == '#')
            {
            fputs(line, f);
            fputc('\n', f);
            }
        else
            {
            if (sampleCount < 1)
                {
                lineFileAbort(lf, "VCF header did not include #CHROM line defining sample IDs for "
                              "genotype columns");
                }
            int colCount = chopTabs(line, NULL);
            int genotypeCount = colCount - VCF_NUM_COLS_BEFORE_GENOTYPES;
            if (genotypeCount != sampleCount)
                {
                lineFileAbort(lf, "VCF header defines %d samples but there are %d genotype columns",
                              sampleCount, genotypeCount);
                }
            char *words[colCount];
            chopTabs(line, words);
            //#*** TODO: check that POS is sorted
            int pos = strtol(words[1], NULL, 10);
            if (pos > chromSize)
                {
                lineFileAbort(lf, "VCF POS value %d exceeds size of reference sequence (%d)",
                              pos, chromSize);
                }
            // make sure REF value (if given) matches reference genome
            int chromStart = pos - 1;
            struct slName *maskedReasons = maskSites[chromStart];
            char *ref = words[3];
            if (strlen(ref) != 1)
                {
                // Not an SNV -- skip it.
                //#*** should probably report or at least count these...
                continue;
                }
            char refBase = toupper(refGenome->dna[chromStart]);
            if (ref[0] == '*' || ref[0] == '.')
                ref[0] = refBase;
            else if (ref[0] != refBase)
                lineFileAbort(lf, "VCF REF value at position %d is '%s', expecting '%c' "
                              "(or '*' or '.')",
                              pos, ref, refBase);
            char altStrCopy[strlen(words[4])+1];
            safecpy(altStrCopy, sizeof altStrCopy, words[4]);
            char *alts[strlen(words[4])+1];
            chopCommas(altStrCopy, alts);
            //#*** Would be nice to trim out indels from ALT column -- but that would require
            //#*** adjusting genotype codes below.
            struct seqInfo *si = seqInfoList;
            int i;
            for (i = VCF_NUM_COLS_BEFORE_GENOTYPES;  i < colCount;  i++, si = si->next)
                {
                if (words[i][0] != '.' && !isdigit(words[i][0]))
                    {
                    lineFileAbort(lf, "VCF genotype columns must contain numeric allele codes; "
                                  "can't parse '%s'", words[i]);
                    }
                else
                    {
                    if (words[i][0] == '.')
                        {
                        si->seq->dna[chromStart] = 'n';
                        si->nCountMiddle++;
                        }
                    else
                        {
                        int alIx = atol(words[i]);
                        if (alIx > 0)
                            {
                            char *alt = alts[alIx-1];
                            if (strlen(alt) == 1)
                                {
                                si->seq->dna[chromStart] = alt[0];
                                struct singleNucChange *snc = sncNew(chromStart, ref[0], '\0',
                                                                     alt[0]);
                                if (maskedReasons)
                                    {
                                    slAddHead(&si->maskedSncList, snc);
                                    slAddHead(&si->maskedReasonsList, slRefNew(maskedReasons));
                                    }
                                else
                                    {
                                    if (isIupacAmbiguous(alt[0]))
                                        si->ambigCount++;
                                    slAddHead(&si->sncList, snc);
                                    }
                                }
                            }
                        }
                    }
                }
            if (!maskedReasons)
                {
                fputs(chrom, f);
                for (i = 1;  i < colCount;  i++)
                    {
                    fputc('\t', f);
                    fputs(words[i], f);
                    }
                fputc('\n', f);
                }
            }
        }
    }
errCatchEnd(errCatch);
carefulClose(&f);
if (errCatch->gotError)
    {
    warn("%s", errCatch->message->string);
    unlink(tn->forCgi);
    freez(&tn);
    }
errCatchFree(&errCatch); 
struct seqInfo *si;
for (si = seqInfoList;  si != NULL;  si = si->next)
    slReverse(&si->sncList);
*retSeqInfoList = seqInfoList;
*retSampleIds = sampleIds;
return tn;
}

static void addSampleMutsFromSeqInfo(struct hash *samplePlacements, struct hash *seqInfoHash)
/* We used to get placementInfo->sampleMuts from DEBUG mode usher output in runUsher.c, but
 * the DEBUG mode output went away with the change to server-mode usher.  Instead, now we fill
 * it in from seqInfo->sncList which has the same info. */
{
struct hashEl *hel;
struct hashCookie cookie = hashFirst(samplePlacements);
while ((hel = hashNext(&cookie)) != NULL)
    {
    struct placementInfo *pi = hel->val;
    struct seqInfo *si = hashFindVal(seqInfoHash, pi->sampleId);
    if (si)
        {
        struct slName *sampleMuts = NULL;
        struct singleNucChange *snc;
        for (snc = si->sncList;  snc != NULL;  snc = snc->next)
            {
            char mutStr[64];
            safef(mutStr, sizeof mutStr, "%c%d%c", snc->refBase, snc->chromStart+1, snc->newBase);
            slNameAddHead(&sampleMuts, mutStr);
            }
        slReverse(&sampleMuts);
        pi->sampleMuts = sampleMuts;
        }
    else
        errAbort("addSampleMutsFromSeqInfo: no seqInfo for placed sequence '%s'", pi->sampleId);
    }
}

static void displaySampleMuts(struct placementInfo *info)
{
printf("<p>Differences from the reference genome "
       "(<a href='https://www.ncbi.nlm.nih.gov/nuccore/NC_045512.2' target=_blank>"
       "NC_045512.2</a>): ");
if (info->sampleMuts == NULL)
    printf("(None; identical to reference)");
else
    {
    struct slName *sln;
    for (sln = info->sampleMuts;  sln != NULL;  sln = sln->next)
        {
        if (sln != info->sampleMuts)
            printf(", ");
        printf("%s", sln->name);
        }
    }
puts("</p>");
}

boolean isInternalNodeName(char *nodeName, int minNewNode)
/* Return TRUE if nodeName looks like an internal node ID from the protobuf tree, i.e. is numeric
 * or <USHER_NODE_PREFIX>_<number> and, if minNewNode > 0, number is less than minNewNode. */
{
if (startsWith(USHER_NODE_PREFIX, nodeName))
    nodeName += strlen(USHER_NODE_PREFIX);
return isAllDigits(nodeName) && (minNewNode <= 0 || (atoi(nodeName) < minNewNode));
}

static void variantPathPrint(struct variantPathNode *variantPath)
/* Print out a variantPath; print nodeName only if non-numeric
 * (i.e. a sample ID not internal node) */
{
struct variantPathNode *vpn;
for (vpn = variantPath;  vpn != NULL;  vpn = vpn->next)
    {
    if (vpn != variantPath)
        printf(" > ");
    if (!isInternalNodeName(vpn->nodeName, 0))
        printf("%s: ", vpn->nodeName);
    struct singleNucChange *snc;
    for (snc = vpn->sncList;  snc != NULL;  snc = snc->next)
        {
        if (snc != vpn->sncList)
            printf(", ");
        printf("%c%d%c", snc->parBase, snc->chromStart+1, snc->newBase);
        }
    }
}

static void displayVariantPath(struct variantPathNode *variantPath, char *sampleId)
/* Display mutations on the path to this sample. */
{
printf("<p>Mutations along the path from the root of the phylogenetic tree to %s:<br>",
       sampleId);
if (variantPath)
    {
    variantPathPrint(variantPath);
    puts("<br>");
    }
else
    puts("(None; your sample was placed at the root of the phylogenetic tree)");
puts("</p>");
}

static struct variantPathNode *findLastInternalNode(struct variantPathNode *variantPath,
                                                    int minNewNode)
/* Return the last node in variantPath with a numeric name less than minNewNode, or NULL. */
{
if (!variantPath || !isInternalNodeName(variantPath->nodeName, minNewNode))
    return NULL;
while (variantPath->next && isInternalNodeName(variantPath->next->nodeName, minNewNode))
    variantPath = variantPath->next;
if (variantPath && isInternalNodeName(variantPath->nodeName, minNewNode))
    return variantPath;
return NULL;
}

static int mutCountCmp(const void *a, const void *b)
/* Compare number of mutations of phyloTree nodes a and b. */
{
const struct phyloTree *nodeA = *(struct phyloTree * const *)a;
const struct phyloTree *nodeB = *(struct phyloTree * const *)b;
return slCount(nodeA->priv) - slCount(nodeB->priv);
}

static char *leafWithLeastMuts(struct phyloTree *node, struct mutationAnnotatedTree *bigTree)
/* If node has a leaf child with no mutations of its own, return the name of that leaf.
 * Otherwise, if node has leaf children, return the name of the leaf with the fewest mutations.
 * Otherwise return NULL. */
{
char *leafName = NULL;
int leafCount = 0;
int i;
for (i = 0;  i < node->numEdges;  i++)
    {
    struct phyloTree *kid = node->edges[i];
    if (kid->numEdges == 0)
        {
        leafCount++;
        struct singleNucChange *kidMuts = kid->priv;
        if (!kidMuts)
            {
            struct slName *nodeList = hashFindVal(bigTree->condensedNodes, kid->ident->name);
            if (nodeList)
                leafName = cloneString(nodeList->name);
            else
                leafName = cloneString(kid->ident->name);
            break;
            }
        }
    }
if (leafName == NULL && leafCount)
    {
    // Pick the leaf with the fewest mutations.
    struct phyloTree *leafKids[leafCount];
    int leafIx = 0;
    for (i = 0;  i < node->numEdges;  i++)
        {
        struct phyloTree *kid = node->edges[i];
        if (kid->numEdges == 0)
                leafKids[leafIx++] = kid;
        }
    qsort(leafKids, leafCount, sizeof(leafKids[0]), mutCountCmp);
    leafName = cloneString(leafKids[0]->ident->name);
    }
return leafName;
}

static char *findNearestNeighbor(struct mutationAnnotatedTree *bigTree, char *sampleId,
                                 struct variantPathNode *variantPath)
/* Use the sequence of mutations in variantPath to find sampleId's parent node in bigTree,
 * then look for most similar leaf sibling(s). */
{
char *nearestNeighbor = NULL;
int bigTreeINodeCount = phyloCountInternalNodes(bigTree->tree);
int minNewNode = bigTreeINodeCount + 1; // 1-based
struct variantPathNode *lastOldNode = findLastInternalNode(variantPath, minNewNode);
struct phyloTree *node = lastOldNode ? hashFindVal(bigTree->nodeHash, lastOldNode->nodeName) :
                                       bigTree->tree;
if (lastOldNode && !node)
    {
    if (startsWith(USHER_NODE_PREFIX, lastOldNode->nodeName))
        // protobuf still has number even if usher prepends USHER_NODE_PREFIX when parsing.
        node = hashFindVal(bigTree->nodeHash, lastOldNode->nodeName+strlen(USHER_NODE_PREFIX));
    if (node == NULL)
        errAbort("Can't find last internal node %s for sample %s", lastOldNode->nodeName, sampleId);
    }
// Look for a leaf kid with no mutations relative to the parent, should be closest.
if (node->numEdges == 0)
    {
    struct slName *nodeList = hashFindVal(bigTree->condensedNodes, node->ident->name);
    if (nodeList)
        nearestNeighbor = cloneString(nodeList->name);
    else
        nearestNeighbor = cloneString(node->ident->name);
    }
else
    {
    nearestNeighbor = leafWithLeastMuts(node, bigTree);
    if (nearestNeighbor == NULL && node->parent != NULL)
        nearestNeighbor = leafWithLeastMuts(node->parent, bigTree);
    }
return nearestNeighbor;
}

static void printVariantPathNoNodeNames(FILE *f, struct variantPathNode *variantPath)
/* Print out variant path with no node names (even if non-numeric) to f. */
{
struct variantPathNode *vpn;
for (vpn = variantPath;  vpn != NULL;  vpn = vpn->next)
    {
    if (vpn != variantPath)
        fprintf(f, " > ");
    struct singleNucChange *snc;
    for (snc = vpn->sncList;  snc != NULL;  snc = snc->next)
        {
        if (snc != vpn->sncList)
            fprintf(f, ", ");
        fprintf(f, "%c%d%c", snc->parBase, snc->chromStart+1, snc->newBase);
        }
    }
}

static struct hash *getSampleMetadata(char *metadataFile)
/* If config.ra defines a metadataFile, load its contents into a hash indexed by EPI ID and return;
 * otherwise return NULL. */
{
struct hash *sampleMetadata = NULL;
if (isNotEmpty(metadataFile) && fileExists(metadataFile))
    {
    sampleMetadata = hashNew(0);
    struct lineFile *lf = lineFileOpen(metadataFile, TRUE);
    int headerWordCount = 0;
    char **headerWords = NULL;
    char *line;
    // Check for header line
    if (lineFileNext(lf, &line, NULL))
        {
        if (startsWithWord("strain", line))
            {
            char *headerLine = cloneString(line);
            headerWordCount = chopString(headerLine, "\t", NULL, 0);
            AllocArray(headerWords, headerWordCount);
            chopString(headerLine, "\t", headerWords, headerWordCount);
            }
        else
            errAbort("Missing header line from metadataFile %s", metadataFile);
        }
    int strainIx = stringArrayIx("strain", headerWords, headerWordCount);
    int epiIdIx = stringArrayIx("gisaid_epi_isl", headerWords, headerWordCount);
    int genbankIx = stringArrayIx("genbank_accession", headerWords, headerWordCount);
    int dateIx = stringArrayIx("date", headerWords, headerWordCount);
    int authorIx = stringArrayIx("authors", headerWords, headerWordCount);
    int nCladeIx = stringArrayIx("Nextstrain_clade", headerWords, headerWordCount);
    int gCladeIx = stringArrayIx("GISAID_clade", headerWords, headerWordCount);
    int lineageIx = stringArrayIx("pangolin_lineage", headerWords, headerWordCount);
    if (lineageIx < 0)
        lineageIx = stringArrayIx("pango_lineage", headerWords, headerWordCount);
    int countryIx = stringArrayIx("country", headerWords, headerWordCount);
    int divisionIx = stringArrayIx("division", headerWords, headerWordCount);
    int locationIx = stringArrayIx("location", headerWords, headerWordCount);
    int countryExpIx = stringArrayIx("country_exposure", headerWords, headerWordCount);
    int divExpIx = stringArrayIx("division_exposure", headerWords, headerWordCount);
    int origLabIx = stringArrayIx("originating_lab", headerWords, headerWordCount);
    int subLabIx = stringArrayIx("submitting_lab", headerWords, headerWordCount);
    int regionIx = stringArrayIx("region", headerWords, headerWordCount);
    int nCladeUsherIx = stringArrayIx("Nextstrain_clade_usher", headerWords, headerWordCount);
    int lineageUsherIx = stringArrayIx("pango_lineage_usher", headerWords, headerWordCount);
    while (lineFileNext(lf, &line, NULL))
        {
        char *words[headerWordCount];
        int wordCount = chopTabs(line, words);
        lineFileExpectWords(lf, headerWordCount, wordCount);
        struct sampleMetadata *met;
        AllocVar(met);
        if (strainIx >= 0)
            met->strain = cloneString(words[strainIx]);
        if (epiIdIx >= 0)
            met->epiId = cloneString(words[epiIdIx]);
        if (genbankIx >= 0 && !sameString("?", words[genbankIx]))
            met->gbAcc = cloneString(words[genbankIx]);
        if (dateIx >= 0)
            met->date = cloneString(words[dateIx]);
        if (authorIx >= 0)
            met->author = cloneString(words[authorIx]);
        if (nCladeIx >= 0)
            met->nClade = cloneString(words[nCladeIx]);
        if (gCladeIx >= 0)
            met->gClade = cloneString(words[gCladeIx]);
        if (lineageIx >= 0)
            met->lineage = cloneString(words[lineageIx]);
        if (countryIx >= 0)
            met->country = cloneString(words[countryIx]);
        if (divisionIx >= 0)
            met->division = cloneString(words[divisionIx]);
        if (locationIx >= 0)
            met->location = cloneString(words[locationIx]);
        if (countryExpIx >= 0)
            met->countryExp = cloneString(words[countryExpIx]);
        if (divExpIx >= 0)
            met->divExp = cloneString(words[divExpIx]);
        if (origLabIx >= 0)
            met->origLab = cloneString(words[origLabIx]);
        if (subLabIx >= 0)
            met->subLab = cloneString(words[subLabIx]);
        if (regionIx >= 0)
            met->region = cloneString(words[regionIx]);
        if (nCladeUsherIx >= 0)
            met->nCladeUsher = cloneString(words[nCladeUsherIx]);
        if (lineageUsherIx >= 0)
            met->lineageUsher = cloneString(words[lineageUsherIx]);
        // If epiId and/or genbank ID is included, we'll probably be using that to look up items.
        if (epiIdIx >= 0 && !isEmpty(words[epiIdIx]))
            hashAdd(sampleMetadata, words[epiIdIx], met);
        if (genbankIx >= 0 && !isEmpty(words[genbankIx]) && !sameString("?", words[genbankIx]))
            {
            if (strchr(words[genbankIx], '.'))
                {
                // Index by versionless accession
                char copy[strlen(words[genbankIx])+1];
                safecpy(copy, sizeof copy, words[genbankIx]);
                char *dot = strchr(copy, '.');
                *dot = '\0';
                hashAdd(sampleMetadata, copy, met);
                }
            else
                hashAdd(sampleMetadata, words[genbankIx], met);
            }
        if (strainIx >= 0 && !isEmpty(words[strainIx]))
            hashAdd(sampleMetadata, words[strainIx], met);
        }
    lineFileClose(&lf);
    }
return sampleMetadata;
}

char *epiIdFromSampleName(char *sampleId)
/* If an EPI_ISL_# ID is present somewhere in sampleId, extract and return it, otherwise NULL. */
{
char *epiId = cloneString(strstr(sampleId, "EPI_ISL_"));
if (epiId)
    {
    char *p = epiId + strlen("EPI_ISL_");
    while (isdigit(*p))
        p++;
    *p = '\0';
    }
return epiId;
}

char *gbIdFromSampleName(char *sampleId)
/* If a GenBank accession is present somewhere in sampleId, extract and return it, otherwise NULL. */
{
char *gbId = NULL;
regmatch_t substrs[2];
if (regexMatchSubstr(sampleId, "([A-Z][A-Z][0-9]{6})", substrs, ArraySize(substrs)))
    {
    // Make sure there are word boundaries around the match
    if ((substrs[1].rm_so == 0 || !isalnum(sampleId[substrs[1].rm_so-1])) &&
        !isalnum(sampleId[substrs[1].rm_eo]))
        gbId = cloneStringZ(sampleId+substrs[1].rm_so, substrs[1].rm_eo - substrs[1].rm_so);
    }
return gbId;
}

struct sampleMetadata *metadataForSample(struct hash *sampleMetadata, char *sampleId)
/* Look up sampleId in sampleMetadata, by accession if sampleId seems to include an accession. */
{
struct sampleMetadata *met = NULL;
if (sampleMetadata == NULL)
    return NULL;
char *epiId = epiIdFromSampleName(sampleId);
if (epiId)
    met = hashFindVal(sampleMetadata, epiId);
if (!met)
    {
    char *gbId = gbIdFromSampleName(sampleId);
    if (gbId)
        met = hashFindVal(sampleMetadata, gbId);
    }
if (!met)
    met = hashFindVal(sampleMetadata, sampleId);
if (!met && strchr(sampleId, '|'))
    {
    char copy[strlen(sampleId)+1];
    safecpy(copy, sizeof copy, sampleId);
    char *words[4];
    int wordCount = chopString(copy, "|", words, ArraySize(words));
    if (isNotEmpty(words[0]))
        met = hashFindVal(sampleMetadata, words[0]);
    if (met == NULL && wordCount > 1 && isNotEmpty(words[1]))
        met = hashFindVal(sampleMetadata, words[1]);
    }
// If it's one of our collapsed node names, dig out the example name and try that.
if (!met && isdigit(sampleId[0]) && strstr(sampleId, "_from_"))
    {
    char *eg = strstr(sampleId, "_eg_");
    if (eg)
        met = hashFindVal(sampleMetadata, eg+strlen("_eg_"));
    }
return met;
}

static char *lineageForSample(struct hash *sampleMetadata, char *sampleId)
/* Look up sampleId's lineage in epiToLineage file. Return NULL if we don't find a match. */
{
char *lineage = NULL;
struct sampleMetadata *met = metadataForSample(sampleMetadata, sampleId);
if (met)
    lineage = met->lineage;
return lineage;
}

static void findNearestNeighbors(struct hash *samplePlacements, struct hash *sampleMetadata,
                                 struct mutationAnnotatedTree *bigTree)
/* For each placed sample, find the nearest neighbor in the bigTree and its assigned lineage,
 * and fill in those two fields of placementInfo. */
{
if (!bigTree)
    return;
struct hashCookie cookie = hashFirst(samplePlacements);
struct hashEl *hel;
while ((hel = hashNext(&cookie)) != NULL)
    {
    struct placementInfo *info = hel->val;
    info->nearestNeighbor = findNearestNeighbor(bigTree, info->sampleId, info->variantPath);
    if (isNotEmpty(info->nearestNeighbor))
        info->neighborLineage = lineageForSample(sampleMetadata, info->nearestNeighbor);
    }
}

static void printLineageUrl(char *lineage)
/* If lineage is not empty/NULL, print ": lineage <lineage>" and link to outbreak.info
 * (unless lineage is "None") */
{
if (isNotEmpty(lineage))
    {
    if (differentString(lineage, "None"))
        printf(": lineage <a href='"OUTBREAK_INFO_URLBASE"%s' target=_blank>%s</a>",
               lineage, lineage);
    else
        printf(": lineage %s", lineage);
    }
}

static void displayNearestNeighbors(struct placementInfo *info, char *source)
/* Use info->variantPaths to find sample's nearest neighbor(s) in tree. */
{
if (isNotEmpty(info->nearestNeighbor))
    {
    printf("<p>Nearest neighboring %s sequence already in phylogenetic tree: %s",
           source, info->nearestNeighbor);
    printLineageUrl(info->neighborLineage);
    puts("</p>");
    }
}

static int placementInfoRefCmpSampleMuts(const void *va, const void *vb)
/* Compare slRef->placementInfo->sampleMuts lists.  Shorter lists first.  Using alpha sort
 * to distinguish between different sampleMuts contents arbitrarily; the purpose is to
 * clump samples with identical lists. */
{
struct slRef * const *rra = va;
struct slRef * const *rrb = vb;
struct placementInfo *pa = (*rra)->val;
struct placementInfo *pb = (*rrb)->val;
int diff = slCount(pa->sampleMuts) - slCount(pb->sampleMuts);
if (diff == 0)
    {
    struct slName *slnA, *slnB;
    for (slnA = pa->sampleMuts, slnB = pb->sampleMuts;  slnA != NULL;
         slnA = slnA->next, slnB = slnB->next)
        {
        diff = strcmp(slnA->name, slnB->name);
        if (diff != 0)
            break;
        }
    }
return diff;
}

static struct slRef *getPlacementRefList(struct slName *sampleIds, struct hash *samplePlacements)
/* Look up sampleIds in samplePlacements and return ref list of placements. */
{
struct slRef *placementRefs = NULL;
struct slName *sample;
for (sample = sampleIds;  sample != NULL;  sample = sample->next)
    {
    struct placementInfo *info = hashFindVal(samplePlacements, sample->name);
    if (!info)
        errAbort("getPlacementRefList: can't find placement info for sample '%s'",
                 sample->name);
    slAddHead(&placementRefs, slRefNew(info));
    }
slReverse(&placementRefs);
return placementRefs;
}

static int countIdentical(struct slRef *placementRefs)
/* Return the number of placements that have identical sampleMuts lists. */
{
int clumpCount = 0;
struct slRef *ref;
for (ref = placementRefs;  ref != NULL;  ref = ref->next)
    {
    clumpCount++;
    if (ref->next == NULL || placementInfoRefCmpSampleMuts(&ref, &ref->next))
        break;
    }
return clumpCount;
}

static void asciiTree(struct phyloTree *node, char *indent, boolean isLast,
                      struct dyString *dyLine, struct slPair **pRowList)
/* Until we can make a real graphic, at least print an ascii tree build up a (reversed) list of
 * lines so that we can add some text to the right later. */
{
if (isNotEmpty(indent) || isNotEmpty(node->ident->name))
    {
    if (node->ident->name && !isInternalNodeName(node->ident->name, 0))
        {
        dyStringPrintf(dyLine, "%s %s", indent, node->ident->name);
        slPairAdd(pRowList, node->ident->name, cloneString(dyLine->string));
        dyStringClear(dyLine);
        }
    }
int indentLen = strlen(indent);
char indentForKids[indentLen+1];
safecpy(indentForKids, sizeof indentForKids, indent);
if (indentLen >= 4)
    {
    if (isLast)
        safecpy(indentForKids+indentLen - 4, 4 + 1, "    ");
    else
        safecpy(indentForKids+indentLen - 4, 4 + 1, "|   ");
    }
if (node->numEdges > 0)
    {
    char kidIndent[strlen(indent)+5];
    safef(kidIndent, sizeof kidIndent, "%s%s", indentForKids, "+---");
    int i;
    for (i = 0;  i < node->numEdges;  i++)
        asciiTree(node->edges[i], kidIndent, (i == node->numEdges - 1), dyLine, pRowList);
    }
}

static void asciiTreeWithNeighborInfo(struct phyloTree *subtree, struct hash *samplePlacements)
/* Print out an ascii tree with nearest neighbor & lineage to the right as suggested by Joe deRisi */
{
struct dyString *dy = dyStringNew(0);
struct slPair *rowList = NULL;
asciiTree(subtree, "", TRUE, dy, &rowList);
slReverse(&rowList);
int maxLen = 0;
struct slPair *row;
for (row = rowList;  row != NULL;  row = row->next)
    {
    char *asciiRow = row->val;
    int len = strlen(asciiRow);
    if (len > maxLen)
        maxLen = len;
    }
for (row = rowList;  row != NULL;  row = row->next)
    {
    char *asciiRow = row->val;
    char *neighbor = "?";
    char *lineage = "?";
    struct placementInfo *info = hashFindVal(samplePlacements, row->name);
    if (info)
        {
        if (isNotEmpty(info->nearestNeighbor))
            neighbor = info->nearestNeighbor;
        if (isNotEmpty(info->neighborLineage))
            lineage = info->neighborLineage;
        }
    printf("%-*s  %s  %s\n", maxLen, asciiRow, neighbor, lineage);
    }
slNameFreeList(&rowList);
dyStringFree(&dy);
}

static void describeSamplePlacements(struct slName *sampleIds, struct hash *samplePlacements,
                                     struct phyloTree *subtree, struct hash *sampleMetadata,
                                     char *source)
/* Report how each sample fits into the big tree. */
{
// Sort sample placements by sampleMuts so we can group identical samples.
struct slRef *placementRefs = getPlacementRefList(sampleIds, samplePlacements);
slSort(&placementRefs, placementInfoRefCmpSampleMuts);
int relatedCount = slCount(placementRefs);
int clumpSize = countIdentical(placementRefs);
if (clumpSize < relatedCount && relatedCount > 2)
    {
    // Not all of the related sequences are identical, so they will be broken down into
    // separate "clumps".  List all related samples first to avoid confusion.
    puts("<pre>");
    asciiTreeWithNeighborInfo(subtree, samplePlacements);
    puts("</pre>");
    }
struct slRef *refsToGo = placementRefs;
while ((clumpSize = countIdentical(refsToGo)) > 0)
    {
    struct slRef *ref = refsToGo;
    struct placementInfo *info = ref->val;
    if (clumpSize > 1)
        {
        // Sort identical samples alphabetically:
        struct slName *sortedSamples = NULL;
        int i;
        for (i = 0, ref = refsToGo;  ref != NULL && i < clumpSize;  ref = ref->next, i++)
            {
            info = ref->val;
            slNameAddHead(&sortedSamples, info->sampleId);
            }
        slNameSort(&sortedSamples);
        printf("<b>%d identical samples:</b>\n<ul>\n", clumpSize);
        struct slName *sln;
        for (sln = sortedSamples;  sln != NULL;  sln = sln->next)
            printf("<li><b>%s</b>\n", sln->name);
        puts("</ul>");
        }
    else
        {
        printf("<b>%s</b>\n", info->sampleId);
        ref = ref->next;
        }
    refsToGo = ref;
    displaySampleMuts(info);
    if (info->imputedBases)
        {
        puts("<p>Base values imputed by parsimony:\n<ul>");
        struct baseVal *bv;
        for (bv = info->imputedBases;  bv != NULL;  bv = bv->next)
            printf("<li>%d: %s\n", bv->chromStart+1, bv->val);
        puts("</ul>");
        puts("</p>");
        }
    displayVariantPath(info->variantPath, clumpSize == 1 ? info->sampleId : "samples");
    displayNearestNeighbors(info, source);
    if (showParsimonyScore && info->parsimonyScore > 0)
        printf("<p>Parsimony score added by your sample: %d</p>\n", info->parsimonyScore);
        //#*** TODO: explain parsimony score
    }
}

struct phyloTree *phyloPruneToIds(struct phyloTree *node, struct slName *sampleIds)
/* Prune all descendants of node that have no leaf descendants in sampleIds. */
{
if (node->numEdges)
    {
    struct phyloTree *prunedKids = NULL;
    int i;
    for (i = 0;  i < node->numEdges;  i++)
        {
        struct phyloTree *kidIntersected = phyloPruneToIds(node->edges[i], sampleIds);
        if (kidIntersected)
            slAddHead(&prunedKids, kidIntersected);
        }
    int kidCount = slCount(prunedKids);
    assert(kidCount <= node->numEdges);
    if (kidCount > 1)
        {
        slReverse(&prunedKids);
        // There is no phyloTreeFree, but if we ever add one, should use it here.
        node->numEdges = kidCount;
        struct phyloTree *kid;
        for (i = 0, kid = prunedKids;  i < kidCount;  i++, kid = kid->next)
            {
            node->edges[i] = kid;
            kid->parent = node;
            }
        }
    else
        return prunedKids;
    }
else if (! (node->ident->name && slNameInList(sampleIds, node->ident->name)))
    node = NULL;
return node;
}

static struct subtreeInfo *subtreeInfoForSample(struct subtreeInfo *subtreeInfoList, char *name,
                                                int *retIx)
/* Find the subtree that contains sample name and set *retIx to its index in the list.
 * If we can't find it, return NULL and set *retIx to -1. */
{
struct subtreeInfo *ti;
int ix;
for (ti = subtreeInfoList, ix = 0;  ti != NULL;  ti = ti->next, ix++)
    if (slNameInList(ti->subtreeUserSampleIds, name))
        break;
if (ti == NULL)
    ix = -1;
*retIx = ix;
return ti;
}

static void lookForCladesAndLineages(struct hash *samplePlacements,
                                     boolean *retGotClades, boolean *retGotLineages)
/* See if UShER has annotated any clades and/or lineages for seqs. */
{
boolean gotClades = FALSE, gotLineages = FALSE;
struct hashEl *hel;
struct hashCookie cookie = hashFirst(samplePlacements);
while ((hel = hashNext(&cookie)) != NULL)
    {
    struct placementInfo *pi = hel->val;
    if (pi)
        {
        if (isNotEmpty(pi->nextClade))
            gotClades = TRUE;
        if (isNotEmpty(pi->pangoLineage))
            gotLineages = TRUE;
        if (gotClades && gotLineages)
            break;
        }
    }
*retGotClades = gotClades;
*retGotLineages = gotLineages;
}

static char *nextstrainHost()
/* Return the nextstrain hostname from an hg.conf param, or NULL if missing. */
{
return cfgOption("nextstrainHost");
}

static char *nextstrainUrlFromTn(struct tempName *jsonTn)
/* Return a link to Nextstrain to view an annotated subtree. */
{
char *jsonUrlForNextstrain = urlFromTn(jsonTn);
char *protocol = strstr(jsonUrlForNextstrain, "://");
if (protocol)
    jsonUrlForNextstrain = protocol + strlen("://");
struct dyString *dy = dyStringCreate("%s/fetch/%s", nextstrainHost(), jsonUrlForNextstrain);
return dyStringCannibalize(&dy);
}

static void makeNextstrainButton(char *id, struct tempName *tn, char *label, char *mouseover)
/* Make a button to view an auspice JSON file in Nextstrain. */
{
char *nextstrainUrl = nextstrainUrlFromTn(tn);
struct dyString *js = dyStringCreate("window.open('%s');", nextstrainUrl);
cgiMakeOnClickButtonWithMsg(id, js->string, label, mouseover);
dyStringFree(&js);
freeMem(nextstrainUrl);
}

static void makeNextstrainButtonN(char *idBase, int ix, int userSampleCount, int subtreeSize,
                                  struct tempName *jsonTns[])
/* Make a button to view one subtree in Nextstrain.  idBase is a short string and
 * ix is 0-based subtree number. */
{
char buttonId[256];
safef(buttonId, sizeof buttonId, "%s%d", idBase, ix+1);
char buttonLabel[256];
safef(buttonLabel, sizeof buttonLabel, "view subtree %d in Nextstrain", ix+1);
struct dyString *dyMo = dyStringCreate("view subtree %d with %d of your sequences and %d other "
                                       "sequences from the phylogenetic tree for context",
                                       ix+1, userSampleCount, subtreeSize - userSampleCount);
makeNextstrainButton(buttonId, jsonTns[ix], buttonLabel, dyMo->string);
dyStringFree(&dyMo);
}

static void makeNsSingleTreeButton(struct tempName *tn)
/* Make a button to view single subtree (with all uploaded samples) in Nextstrain. */
{
makeNextstrainButton("viewNextstrainSingleSubtree", tn,
                     "view downsampled global tree in Nextstrain",
                     "view one subtree that includes all of your uploaded sequences plus "
                     SINGLE_SUBTREE_SIZE" randomly selected sequences from the global phylogenetic "
                     "tree for context");
}

static void makeButtonRow(struct tempName *singleSubtreeJsonTn, struct tempName *jsonTns[],
                          struct subtreeInfo *subtreeInfoList, int subtreeSize, boolean isFasta,
                          boolean offerCustomTrack)
/* Russ's suggestion: row of buttons at the top to view results in GB, Nextstrain, Nextclade. */
{
puts("<p>");
if (offerCustomTrack)
    cgiMakeButtonWithMsg("submit", "view in Genome Browser",
                         "view your uploaded sequences, their phylogenetic relationship and their "
                         "mutations along with many other datasets available in the Genome Browser");
if (nextstrainHost())
    {
    printf("&nbsp;");
    makeNsSingleTreeButton(singleSubtreeJsonTn);
    struct subtreeInfo *ti;
    int ix;
    for (ix = 0, ti = subtreeInfoList;  ti != NULL;  ti = ti->next, ix++)
        {
        int userSampleCount = slCount(ti->subtreeUserSampleIds);
        printf("&nbsp;");
        makeNextstrainButtonN("viewNextstrainTopRow", ix, userSampleCount, subtreeSize, jsonTns);
        }
    }
if (0 && isFasta)
    {
    printf("&nbsp;");
    struct dyString *js = dyStringCreate("window.open('https://master.clades.nextstrain.org/"
                                         "?input-fasta=%s');",
                                         "needATn");  //#*** TODO: save FASTA to file
    cgiMakeOnClickButton("viewNextclade", js->string, "view sequences in Nextclade");
    }
puts("</p>");
}

#define TOOLTIP(text) " <div class='tooltip'>(?)<span class='tooltiptext'>" text "</span></div>"

static void printSummaryHeader(boolean isFasta, boolean gotClades, boolean gotLineages)
/* Print the summary table header row with tooltips explaining columns. */
{
puts("<thead><tr>");
if (isFasta)
    puts("<th>Fasta Sequence</th>\n"
         "<th>Size"
         TOOLTIP("Length of uploaded sequence in bases, excluding runs of N bases at "
                 "beginning and/or end")
         "</th>\n<th>#Ns"
         TOOLTIP("Number of 'N' bases in uploaded sequence, excluding runs of N bases at "
                 "beginning and/or end")
         "</th>");
else
    puts("<th>VCF Sample</th>\n"
         "<th>#Ns"
         TOOLTIP("Number of no-call variants for this sample in uploaded VCF, "
                 "i.e. '.' used in genotype column")
         "</th>");
puts("<th>#Mixed"
     TOOLTIP("Number of IUPAC ambiguous bases, e.g. 'R' for 'A or G'")
     "</th>");
if (isFasta)
    puts("<th>Bases aligned"
         TOOLTIP("Number of bases aligned to reference NC_045512.2 Wuhan/Hu-1, including "
                 "matches and mismatches")
         "</th>\n<th>Inserted bases"
         TOOLTIP("Number of bases in aligned portion of uploaded sequence that are not present in "
                 "reference NC_045512.2 Wuhan/Hu-1")
         "</th>\n<th>Deleted bases"
         TOOLTIP("Number of bases in reference NC_045512.2 Wuhan/Hu-1 that are not "
                 "present in aligned portion of uploaded sequence")
         "</th>");
puts("<th>#SNVs used for placement"
     TOOLTIP("Number of single-nucleotide variants in uploaded sample "
             "(does not include N's or mixed bases) used by UShER to place sample "
             "in phylogenetic tree")
     "</th>\n<th>#Masked SNVs"
     TOOLTIP("Number of single-nucleotide variants in uploaded sample that are masked "
             "(not used for placement) because they occur at known "
             "<a href='https://virological.org/t/issues-with-sars-cov-2-sequencing-data/473/12' "
             "target=_blank>Problematic Sites</a>"));;
if (gotClades)
    puts("</th>\n<th>Nextstrain clade"
     TOOLTIP("The <a href='https://nextstrain.org/blog/2021-01-06-updated-SARS-CoV-2-clade-naming' "
             "target=_blank>Nextstrain clade</a> assigned to the sample by UShER"));
if (gotLineages)
    puts("</th>\n<th>Pango lineage"
     TOOLTIP("The <a href='https://cov-lineages.org/' "
             "target=_blank>Pango lineage</a> assigned to the sample by UShER"));
puts("</th>\n<th>Neighboring sample in tree"
     TOOLTIP("A sample already in the tree that is a child of the node at which the uploaded "
             "sample was placed, to give an example of a closely related sample")
     "</th>\n<th>Lineage of neighbor"
     TOOLTIP("The <a href='https://cov-lineages.org/' target=_blank>"
             "Pango lineage</a> assigned to the nearest neighboring sample already in the tree")
     "</th>\n<th>#Imputed values for mixed bases"
     TOOLTIP("If the uploaded sequence contains mixed/ambiguous bases, then UShER may assign "
             "values based on maximum parsimony")
     "</th>\n<th>#Maximally parsimonious placements"
     TOOLTIP("Number of potential placements in the tree with minimal parsimony score; "
             "the higher the number, the less confident the placement")
     "</th>\n<th>Parsimony score"
     TOOLTIP("Number of mutations/changes that must be added to the tree when placing the "
             "uploaded sample; the higher the number, the more diverged the sample")
     "</th>\n<th>Subtree number"
     TOOLTIP("Sequence number of subtree that contains this sample")
     "</th></tr></thead>");
}


// Default QC thresholds for calling an input sequence excellent/good/fair/bad [/fail]:
static int qcThresholdsMinLength[] = { 29750, 29500, 29000, 28000 };
static int qcThresholdsMaxNs[] = { 0, 5, 20, 100 };
static int qcThresholdsMaxMixed[] = { 0, 5, 20, 100 };
static int qcThresholdsMaxIndel[] = { 9, 18, 24, 36 };
static int qcThresholdsMaxSNVs[] = { 25, 35, 45, 55 };
static int qcThresholdsMaxMaskedSNVs[] = { 0, 1, 2, 3 };
static int qcThresholdsMaxImputed[] = { 0, 5, 20, 100 };
static int qcThresholdsMaxPlacements[] = { 1, 2, 3, 4 };
static int qcThresholdsMaxPScore[] = { 0, 2, 5, 10 };

static void wordsToQcThresholds(char **words, int *thresholds)
/* Parse words from file into thresholds array.  Caller must ensure words and thresholds each
 * have 4 items. */
{
int i;
for (i = 0;  i < 4;  i++)
    thresholds[i] = atoi(words[i]);
}

static void readQcThresholds(char *db)
/* If config.ra specifies a file with QC thresholds for excellent/good/fair/bad [/fail],
 * parse it and replace the default values in qcThresholds arrays.  */
{
char *qcThresholdsFile = phyloPlaceDbSettingPath(db, "qcThresholds");
if (isNotEmpty(qcThresholdsFile))
    {
    if (fileExists(qcThresholdsFile))
        {
        struct lineFile *lf = lineFileOpen(qcThresholdsFile, TRUE);
        char *line;
        while (lineFileNext(lf, &line, NULL))
            {
            char *words[16];
            int wordCount = chopTabs(line, words);
            lineFileExpectWords(lf, 5, wordCount);
            if (sameWord(words[0], "length"))
                wordsToQcThresholds(words+1, qcThresholdsMinLength);
            else if (sameWord(words[0], "nCount"))
                wordsToQcThresholds(words+1, qcThresholdsMaxNs);
            else if (sameWord(words[0], "mixedCount"))
                wordsToQcThresholds(words+1, qcThresholdsMaxMixed);
            else if (sameWord(words[0], "indelCount"))
                wordsToQcThresholds(words+1, qcThresholdsMaxIndel);
            else if (sameWord(words[0], "snvCount"))
                wordsToQcThresholds(words+1, qcThresholdsMaxSNVs);
            else if (sameWord(words[0], "maskedSnvCount"))
                wordsToQcThresholds(words+1, qcThresholdsMaxMaskedSNVs);
            else if (sameWord(words[0], "imputedBases"))
                wordsToQcThresholds(words+1, qcThresholdsMaxImputed);
            else if (sameWord(words[0], "placementCount"))
                wordsToQcThresholds(words+1, qcThresholdsMaxPlacements);
            else if (sameWord(words[0], "parsimony"))
                wordsToQcThresholds(words+1, qcThresholdsMaxPScore);
            else
                warn("qcThresholds file %s: unrecognized parameter '%s', skipping",
                     qcThresholdsFile, words[0]);
            }
        lineFileClose(&lf);
        }
    else
        warn("qcThresholds %s: file not found", qcThresholdsFile);
    }
}

static char *qcClassForIntMin(int n, int thresholds[])
/* Return {qcExcellent, qcGood, qcMeh, qcBad or qcFail} depending on how n compares to the
 * thresholds. Don't free result. */
{
if (n >= thresholds[0])
    return "qcExcellent";
else if (n >= thresholds[1])
    return "qcGood";
else if (n >= thresholds[2])
    return "qcMeh";
else if (n >= thresholds[3])
    return "qcBad";
else
    return "qcFail";
}

static char *qcClassForLength(int length)
/* Return qc class for length of sequence. */
{
return qcClassForIntMin(length, qcThresholdsMinLength);
}

static char *qcClassForIntMax(int n, int thresholds[])
/* Return {qcExcellent, qcGood, qcMeh, qcBad or qcFail} depending on how n compares to the
 * thresholds. Don't free result. */
{
if (n <= thresholds[0])
    return "qcExcellent";
else if (n <= thresholds[1])
    return "qcGood";
else if (n <= thresholds[2])
    return "qcMeh";
else if (n <= thresholds[3])
    return "qcBad";
else
    return "qcFail";
}

static char *qcClassForNs(int nCount)
/* Return qc class for #Ns in sample. */
{
return qcClassForIntMax(nCount, qcThresholdsMaxNs);
}

static char *qcClassForMixed(int mixedCount)
/* Return qc class for #ambiguous bases in sample. */
{
return qcClassForIntMax(mixedCount, qcThresholdsMaxMixed);
}

static char *qcClassForIndel(int indelCount)
/* Return qc class for #inserted or deleted bases. */
{
return qcClassForIntMax(indelCount, qcThresholdsMaxIndel);
}

static char *qcClassForSNVs(int snvCount)
/* Return qc class for #SNVs in sample. */
{
return qcClassForIntMax(snvCount, qcThresholdsMaxSNVs);
}

static char *qcClassForMaskedSNVs(int maskedCount)
/* Return qc class for #SNVs at problematic sites. */
{
return qcClassForIntMax(maskedCount, qcThresholdsMaxMaskedSNVs);
}

static char *qcClassForImputedBases(int imputedCount)
/* Return qc class for #ambiguous bases for which UShER imputed values based on placement. */
{
return qcClassForIntMax(imputedCount, qcThresholdsMaxImputed);
}

static char *qcClassForPlacements(int placementCount)
/* Return qc class for number of equally parsimonious placements. */
{
return qcClassForIntMax(placementCount, qcThresholdsMaxPlacements);
}

static char *qcClassForPScore(int parsimonyScore)
/* Return qc class for parsimonyScore. */
{
return qcClassForIntMax(parsimonyScore, qcThresholdsMaxPScore);
}

static void printTooltip(char *text)
/* Print a tooltip with explanatory text. */
{
printf(TOOLTIP("%s"), text);
}

static void appendExcludingNs(struct dyString *dy, struct seqInfo *si)
/* Append a note to dy about how many N bases and start and/or end are excluded from statistic. */
{
dyStringAppend(dy, "excluding ");
if (si->nCountStart)
    dyStringPrintf(dy, "%d N bases at start", si->nCountStart);
if (si->nCountStart && si->nCountEnd)
    dyStringAppend(dy, " and ");
if (si->nCountEnd)
    dyStringPrintf(dy, "%d N bases at end", si->nCountEnd);
}

static void printLineageTd(char *lineage, char *alt)
/* Print a table cell with lineage (& link to outbreak.info if not 'None') or alt if no lineage. */
{
if (lineage && differentString(lineage, "None"))
    printf("<td><a href='"OUTBREAK_INFO_URLBASE"%s' target=_blank>%s</a></td>", lineage, lineage);
else if (lineage)
    printf("<td>%s</td>", lineage);
else
    printf("<td>%s</td>", alt);
}

static void printSubtreeTd(struct subtreeInfo *subtreeInfoList, struct tempName *jsonTns[],
                           char *seqName)
/* Print a table cell with subtree (& link if possible) if found. */
{
int ix;
struct subtreeInfo *ti = subtreeInfoForSample(subtreeInfoList, seqName, &ix);
if (ix < 0)
    //#*** Probably an error.
    printf("<td>n/a</td>");
else
    {
    printf("<td>%d", ix+1);
    if (ti && nextstrainHost())
        {
        char *nextstrainUrl = nextstrainUrlFromTn(jsonTns[ix]);
        printf(" (<a href='%s' target=_blank>view in Nextstrain<a>)", nextstrainUrl);
        }
    printf("</td>");
    }
}

static void summarizeSequences(struct seqInfo *seqInfoList, boolean isFasta,
                               struct usherResults *ur, struct tempName *jsonTns[],
                               struct hash *sampleMetadata, struct dnaSeq *refGenome)
/* Show a table with composition & alignment stats for each sequence that passed basic QC. */
{
if (seqInfoList)
    {
    puts("<table class='seqSummary'>");
    boolean gotClades = FALSE, gotLineages = FALSE;
    lookForCladesAndLineages(ur->samplePlacements, &gotClades, &gotLineages);
    printSummaryHeader(isFasta, gotClades, gotLineages);
    puts("<tbody>");
    struct dyString *dy = dyStringNew(0);
    struct seqInfo *si;
    for (si = seqInfoList;  si != NULL;  si = si->next)
        {
        puts("<tr>");
        printf("<th>%s</td>", replaceChars(si->seq->name, "|", " | "));
        if (isFasta)
            {
            if (si->nCountStart || si->nCountEnd)
                {
                int effectiveLength = si->seq->size - (si->nCountStart + si->nCountEnd);
                dyStringClear(dy);
                dyStringPrintf(dy, "%d ", effectiveLength);
                appendExcludingNs(dy, si);
                dyStringPrintf(dy, " (original size %d)", si->seq->size);
                printf("<td class='%s'>%d", qcClassForLength(effectiveLength), effectiveLength);
                printTooltip(dy->string);
                printf("</td>");
                }
            else
                printf("<td class='%s'>%d</td>", qcClassForLength(si->seq->size), si->seq->size);
            }
        printf("<td class='%s'>%d",
               qcClassForNs(si->nCountMiddle), si->nCountMiddle);
        if (si->nCountStart || si->nCountEnd)
            {
            dyStringClear(dy);
            dyStringPrintf(dy, "%d Ns ", si->nCountMiddle);
            appendExcludingNs(dy, si);
            printTooltip(dy->string);
            }
        printf("</td><td class='%s'>%d ", qcClassForMixed(si->ambigCount), si->ambigCount);
        int alignedAmbigCount = 0;
        if (si->ambigCount > 0)
            {
            dyStringClear(dy);
            struct singleNucChange *snc;
            for (snc = si->sncList;  snc != NULL;  snc = snc->next)
                {
                if (isIupacAmbiguous(snc->newBase))
                    {
                    dyStringAppendSep(dy, ", ");
                    dyStringPrintf(dy, "%c%d%c", snc->refBase, snc->chromStart+1, snc->newBase);
                    alignedAmbigCount++;
                    }
                }
            if (isEmpty(dy->string))
                dyStringAppend(dy, "(Masked or not aligned to reference)");
            else if (alignedAmbigCount != si->ambigCount)
                dyStringPrintf(dy, " (%d masked or not aligned to reference)",
                               si->ambigCount - alignedAmbigCount);
            printTooltip(dy->string);
            }
        printf("</td>");
        if (isFasta)
            {
            struct psl *psl = si->psl;
            if (psl)
                {
                int aliCount = psl->match + psl->misMatch + psl->repMatch;
                printf("<td class='%s'>%d ", qcClassForLength(aliCount), aliCount);
                dyStringClear(dy);
                dyStringPrintf(dy, "bases %d - %d align to reference bases %d - %d",
                               psl->qStart+1, psl->qEnd, psl->tStart+1, psl->tEnd);
                printTooltip(dy->string);
                printf("</td><td class='%s'>%d ",
                       qcClassForIndel(si->insBases), si->insBases);
                if (si->insBases)
                    {
                    printTooltip(si->insRanges);
                    }
                printf("</td><td class='%s'>%d ",
                       qcClassForIndel(si->delBases), si->delBases);
                if (si->delBases)
                    {
                    printTooltip(si->delRanges);
                    }
                printf("</td>");
                }
            else
                printf("<td colspan=3 class='%s'> not alignable </td>",
                       qcClassForLength(0));
            }
        int snvCount = slCount(si->sncList) - alignedAmbigCount;
        printf("<td class='%s'>%d", qcClassForSNVs(snvCount), snvCount);
        if (snvCount > 0)
            {
            dyStringClear(dy);
            struct singleNucChange *snc;
            for (snc = si->sncList;  snc != NULL;  snc = snc->next)
                {
                if (!isIupacAmbiguous(snc->newBase))
                    {
                    dyStringAppendSep(dy, ", ");
                    dyStringPrintf(dy, "%c%d%c", snc->refBase, snc->chromStart+1, snc->newBase);
                    }
                }
            printTooltip(dy->string);
            }
        int maskedCount = slCount(si->maskedSncList);
        printf("</td><td class='%s'>%d", qcClassForMaskedSNVs(maskedCount), maskedCount);
        if (maskedCount > 0)
            {
            dyStringClear(dy);
            struct singleNucChange *snc;
            struct slRef *reasonsRef;
            for (snc = si->maskedSncList, reasonsRef = si->maskedReasonsList;  snc != NULL;
                 snc = snc->next, reasonsRef = reasonsRef->next)
                {
                dyStringAppendSep(dy, ", ");
                struct slName *reasonList = reasonsRef->val, *reason;
                replaceChar(reasonList->name, '_', ' ');
                dyStringPrintf(dy, "%c%d%c (%s",
                               snc->refBase, snc->chromStart+1, snc->newBase, reasonList->name);
                for (reason = reasonList->next;  reason != NULL;  reason = reason->next)
                    {
                    replaceChar(reason->name, '_', ' ');
                    dyStringPrintf(dy, ", %s", reason->name);
                    }
                dyStringAppendC(dy, ')');
                }
            printTooltip(dy->string);
            }
        printf("</td>");
        struct placementInfo *pi = hashFindVal(ur->samplePlacements, si->seq->name);
        if (pi)
            {
            if (gotClades)
                printf("<td>%s</td>", pi->nextClade ? pi->nextClade : "n/a");
            if (gotLineages)
                printLineageTd(pi->pangoLineage, "n/a");
            printf("<td>%s</td>",
                   pi->nearestNeighbor ? replaceChars(pi->nearestNeighbor, "|", " | ") : "?");
            printLineageTd(pi->neighborLineage, "?");
            int imputedCount = slCount(pi->imputedBases);
            printf("<td class='%s'>%d",
                   qcClassForImputedBases(imputedCount), imputedCount);
            if (imputedCount > 0)
                {
                dyStringClear(dy);
                struct baseVal *bv;
                for (bv = pi->imputedBases;  bv != NULL;  bv = bv->next)
                    {
                    dyStringAppendSep(dy, ", ");
                    dyStringPrintf(dy, "%d: %s", bv->chromStart+1, bv->val);
                    }
                printTooltip(dy->string);
                }
            printf("</td><td class='%s'>%d",
                   qcClassForPlacements(pi->bestNodeCount), pi->bestNodeCount);
            printf("</td><td class='%s'>%d",
                   qcClassForPScore(pi->parsimonyScore), pi->parsimonyScore);
            printf("</td>");
            }
        else
            {
            if (gotClades)
                printf("<td>n/a</td>");
            if (gotLineages)
                printf("<td>n/a</td>");
            printf("<td>n/a</td><td>n/a</td><td>n/a</td><td>n/a</td><td>n/a</td>");
            }
        printSubtreeTd(ur->subtreeInfoList, jsonTns, si->seq->name);
        puts("</tr>");
        }
    puts("</tbody></table><p></p>");
    }
}

static void summarizeSubtrees(struct slName *sampleIds, struct usherResults *results,
                              struct hash *sampleMetadata, struct tempName *jsonTns[],
                              struct mutationAnnotatedTree *bigTree)
/* Print a summary table of pasted/uploaded identifiers and subtrees */
{
boolean gotClades = FALSE, gotLineages = FALSE;
lookForCladesAndLineages(results->samplePlacements, &gotClades, &gotLineages);
puts("<table class='seqSummary'><tbody>");
puts("<tr><th>Sequence</th>");
if (gotClades)
    puts("<th>Nextstrain clade (UShER)"
     TOOLTIP("The <a href='https://nextstrain.org/blog/2021-01-06-updated-SARS-CoV-2-clade-naming' "
             "target=_blank>Nextstrain clade</a> "
             "assigned to the sequence by UShER according to its place in the phylogenetic tree")
         "</th>");
if (gotLineages)
    puts("<th>Pango lineage (UShER)"
         TOOLTIP("The <a href='https://cov-lineages.org/' "
                 "target=_blank>Pango lineage</a> "
                 "assigned to the sequence by UShER according to its place in the phylogenetic tree")
         "</th>");
puts("<th>Pango lineage (pangolin)"
     TOOLTIP("The <a href='https://cov-lineages.org/' target=_blank>"
             "Pango lineage</a> assigned to the sequence by "
             "<a href='https://github.com/cov-lineages/pangolin/' target=_blank>pangolin</a>")
     "</th>"
     "<th>subtree</th></tr>");
struct slName *si;
for (si = sampleIds;  si != NULL;  si = si->next)
    {
    puts("<tr>");
    printf("<th>%s</td>", replaceChars(si->name, "|", " | "));
    struct placementInfo *pi = hashFindVal(results->samplePlacements, si->name);
    if (pi)
        {
        if (gotClades)
            printf("<td>%s</td>", pi->nextClade ? pi->nextClade : "n/a");
        if (gotLineages)
            printLineageTd(pi->pangoLineage, "n/a");
        }
    else
        {
        if (gotClades)
            printf("<td>n/a</td>");
        if (gotLineages)
            printf("<td>n/a</td>");
        }
    // pangolin-assigned lineage
    char *lineage = lineageForSample(sampleMetadata, si->name);
    if (isNotEmpty(lineage))
        printf("<td><a href='"OUTBREAK_INFO_URLBASE"%s' target=_blank>%s</a></td>",
               lineage, lineage);
    else
        printf("<td>n/a</td>");
    // Maybe also #mutations with mouseover to show mutation path?
    printSubtreeTd(results->subtreeInfoList, jsonTns, si->name);
    }
puts("</tbody></table><p></p>");
}

static struct singleNucChange *sncListFromSampleMutsAndImputed(struct slName *sampleMuts,
                                                               struct baseVal *imputedBases,
                                                               struct seqWindow *gSeqWin)
/* Convert a list of "<ref><pos><alt>" names to struct singleNucChange list.
 * However, if <alt> is ambiguous, skip it because variantProjector doesn't like it.
 * Add imputed base predictions. */
{
struct singleNucChange *sncList = NULL;
struct slName *mut;
for (mut = sampleMuts;  mut != NULL;  mut = mut->next)
    {
    char ref = mut->name[0];
    if (ref < 'A' || ref > 'Z')
        errAbort("sncListFromSampleMuts: expected ref base value, got '%c' in '%s'",
                 ref, mut->name);
    int pos = atoi(&(mut->name[1]));
    if (pos < 1 || pos > chromSize)
        errAbort("sncListFromSampleMuts: expected pos between 1 and %d, got %d in '%s'",
                 chromSize, pos, mut->name);
    char alt = mut->name[strlen(mut->name)-1];
    if (alt < 'A' || alt > 'Z')
        errAbort("sncListFromSampleMuts: expected alt base value, got '%c' in '%s'",
                 alt, mut->name);
    if (isIupacAmbiguous(alt))
        continue;
    struct singleNucChange *snc;
    AllocVar(snc);
    snc->chromStart = pos-1;
    snc->refBase = snc->parBase = ref;
    snc->newBase = alt;
    slAddHead(&sncList, snc);
    }
struct baseVal *bv;
for (bv = imputedBases;  bv != NULL;  bv = bv->next)
    {
    char ref[2];
    seqWindowCopy(gSeqWin, bv->chromStart, 1, ref, sizeof ref);
    struct singleNucChange *snc;
    AllocVar(snc);
    snc->chromStart = bv->chromStart;
    snc->refBase = snc->parBase = ref[0];
    snc->newBase = bv->val[0];
    slAddHead(&sncList, snc);
    }
slReverse(&sncList);
return sncList;
}

enum spikeMutType
/* Some categories of Spike mutation are more concerning than others. */
{
    smtNone,        // Just a spike mutation.
    smtVoC,         // Thought to be the problematic mutation in a Variant of Concern.
    smtEscape,      // Implicated in Antibody Escape experiments.
    smtRbd,         // Receptor Binding Domain
    smtCleavage,    // Furin cleavage site
    smtD614G        // Went from rare to ~99% frequency in first half of 2020; old news.
};

static char *spikeMutTypeToString(enum spikeMutType smt)
/* Returns a string version of smt.  Do not free the returned value. */
{
char *string = NULL;
switch (smt)
    {
    case smtNone:
        string = "spike";
        break;
    case smtVoC:
        string = "VoC";
        break;
    case smtEscape:
        string = "escape";
        break;
    case smtRbd:
        string = "RBD";
        break;
    case smtCleavage:
        string = "cleavage";
        break;
    case smtD614G:
        string = "D614G";
        break;
    default:
        errAbort("spikeMutTypeToString: Invalid value %d", smt);
    }
return string;
}

struct aaMutInfo
// A protein change, who has it, and how important we think it is.
{
    char *name;                 // The name that people are used to seeing, e.g. "E484K', "N501Y"
    struct slName *sampleIds;   // The uploaded samples that have it
    int priority;               // For sorting; lower number means scarier.
    int pos;                    // 1-based position
    enum spikeMutType spikeType;// If spike mutation, what kind?
    char oldAa;                 // Reference AA
    char newAa;                 // Alt AA
};

int aaMutInfoCmp(const void *a, const void *b)
/* Compare aaMutInfo priority for sorting. */
{
const struct aaMutInfo *amiA = *(struct aaMutInfo * const *)a;
const struct aaMutInfo *amiB = *(struct aaMutInfo * const *)b;
int dif = amiA->priority - amiB->priority;
if (dif == 0)
    dif = amiA->pos - amiB->pos;
return dif;
}

// For now, hardcode SARS-CoV-2 Spike RBD coords and antibody escape positions (1-based).
static int rbdStart = 319;
static int rbdEnd = 541;
static boolean *escapeMutPos = NULL;

static void initEscapeMutPos()
/* Allocate excapeMutPos and set positions implicated by at least a couple experiments from Bloom Lab
 * or that appear in Whelan, Rappuoli or McCoy tracks. */
{
AllocArray(escapeMutPos, rbdEnd);
escapeMutPos[332] = TRUE;
escapeMutPos[334] = TRUE;
escapeMutPos[335] = TRUE;
escapeMutPos[337] = TRUE;
escapeMutPos[339] = TRUE;
escapeMutPos[340] = TRUE;
escapeMutPos[345] = TRUE;
escapeMutPos[346] = TRUE;
escapeMutPos[348] = TRUE;
escapeMutPos[352] = TRUE;
escapeMutPos[357] = TRUE;
escapeMutPos[359] = TRUE;
escapeMutPos[361] = TRUE;
escapeMutPos[362] = TRUE;
escapeMutPos[363] = TRUE;
escapeMutPos[365] = TRUE;
escapeMutPos[366] = TRUE;
escapeMutPos[367] = TRUE;
escapeMutPos[369] = TRUE;
escapeMutPos[370] = TRUE;
escapeMutPos[371] = TRUE;
escapeMutPos[372] = TRUE;
escapeMutPos[373] = TRUE;
escapeMutPos[374] = TRUE;
escapeMutPos[376] = TRUE;
escapeMutPos[378] = TRUE;
escapeMutPos[383] = TRUE;
escapeMutPos[384] = TRUE;
escapeMutPos[385] = TRUE;
escapeMutPos[386] = TRUE;
escapeMutPos[408] = TRUE;
escapeMutPos[417] = TRUE;
escapeMutPos[441] = TRUE;
escapeMutPos[444] = TRUE;
escapeMutPos[445] = TRUE;
escapeMutPos[445] = TRUE;
escapeMutPos[447] = TRUE;
escapeMutPos[449] = TRUE;
escapeMutPos[450] = TRUE;
escapeMutPos[452] = TRUE;
escapeMutPos[455] = TRUE;
escapeMutPos[456] = TRUE;
escapeMutPos[458] = TRUE;
escapeMutPos[472] = TRUE;
escapeMutPos[473] = TRUE;
escapeMutPos[474] = TRUE;
escapeMutPos[476] = TRUE;
escapeMutPos[477] = TRUE;
escapeMutPos[478] = TRUE;
escapeMutPos[479] = TRUE;
escapeMutPos[483] = TRUE;
escapeMutPos[484] = TRUE;
escapeMutPos[485] = TRUE;
escapeMutPos[486] = TRUE;
escapeMutPos[487] = TRUE;
escapeMutPos[489] = TRUE;
escapeMutPos[490] = TRUE;
escapeMutPos[494] = TRUE;
escapeMutPos[499] = TRUE;
escapeMutPos[504] = TRUE;
escapeMutPos[514] = TRUE;
escapeMutPos[517] = TRUE;
escapeMutPos[522] = TRUE;
escapeMutPos[525] = TRUE;
escapeMutPos[526] = TRUE;
escapeMutPos[527] = TRUE;
escapeMutPos[528] = TRUE;
escapeMutPos[529] = TRUE;
escapeMutPos[530] = TRUE;
escapeMutPos[531] = TRUE;
}

static int spikePriority(int pos, char newAa, enum spikeMutType *pSmt)
/* Lower number for scarier spike mutation, per spike mutations track / RBD. */
{
if (escapeMutPos == NULL)
    initEscapeMutPos();
int priority = 0;
enum spikeMutType smt = smtNone;
if (pos >= rbdStart && pos <= rbdEnd)
    {
    // Receptor binding domain
    priority = 100;
    smt = smtRbd;
    // Antibody escape mutation in Variant of Concern/Interest
    if (pos == 484)
        {
        priority = 10;
        smt = smtVoC;
        }
    else if (pos == 501)
        {
        priority = 15;
        smt = smtVoC;
        }
    else if (pos == 452)
        {
        priority = 20;
        smt = smtVoC;
        }
    // Other antibody escape mutations
    else if (pos == 439 || pos == 477)
        {
        priority = 25;
        smt = smtEscape;
        }
    else if (escapeMutPos[pos])
        {
        priority = 50;
        smt = smtEscape;
        }
    }
else if (pos >= 675 && pos <= 681)
    {
    // Furin cleavage site; circumstantial evidence for Q677{H,P} spread in US.
    // Interesting threads on SPHERES 2021-02-26 about P681H tradeoff between infectivity vs
    // range of cell types that can be infected and other observations about that region.
    priority = 110;
    smt = smtCleavage;
    }
else if (pos == 614 && newAa == 'G')
    {
    // Old hat
    priority = 1000;
    smt = smtD614G;
    }
else
    // Somewhere else in Spike
    priority = 500;
if (pSmt != NULL)
    *pSmt = smt;
return priority;
}

static void addSpikeChange(struct hash *spikeChanges, char *aaMutStr, char *sampleId)
/* Tally up an observance of a S gene change in a sample. */
{
// Parse oldAa, pos, newAA out of aaMutStr.  Need a more elegant way of doing this;
// this is a rush job to get something usable out there asap.
char oldAa = aaMutStr[0];
int pos = atoi(aaMutStr+1);
char newAa = aaMutStr[strlen(aaMutStr)-1];
struct hashEl *hel = hashLookup(spikeChanges, aaMutStr);
if (hel == NULL)
    {
    struct aaMutInfo *ami;
    AllocVar(ami);
    ami->name = cloneString(aaMutStr);
    slNameAddHead(&ami->sampleIds, sampleId);
    ami->priority = spikePriority(pos, newAa, &ami->spikeType);
    ami->pos = pos;
    ami->oldAa = oldAa;
    ami->newAa = newAa;
    hashAdd(spikeChanges, aaMutStr, ami);
    }
else
    {
    struct aaMutInfo *ami = hel->val;
    slNameAddHead(&ami->sampleIds, sampleId);
    }
}

static void writeOneTsvRow(FILE *f, char *sampleId, struct usherResults *results,
                           struct hash *seqInfoHash, struct geneInfo *geneInfoList,
                           struct seqWindow *gSeqWin, struct hash *spikeChanges)
/* Write one row of tab-separate summary for sampleId.  Accumulate S gene AA change info. */
{
struct placementInfo *info = hashFindVal(results->samplePlacements, sampleId);
if (info)
    {
    // sample name / ID
    fprintf(f, "%s\t", sampleId);
    // nucleotide mutations
    struct slName *mut;
    for (mut = info->sampleMuts;  mut != NULL;  mut = mut->next)
        {
        if (mut != info->sampleMuts)
            fputc(',', f);
        fputs(mut->name, f);
        }
    fputc('\t', f);
    // AA mutations
    struct singleNucChange *sncList = sncListFromSampleMutsAndImputed(info->sampleMuts,
                                                                      info->imputedBases, gSeqWin);
    struct slPair *geneAaMutations = getAaMutations(sncList, geneInfoList, gSeqWin);
    struct slPair *geneAaMut;
    boolean first = TRUE;
    for (geneAaMut = geneAaMutations;  geneAaMut != NULL;  geneAaMut = geneAaMut->next)
        {
        struct slName *aaMut;
        for (aaMut = geneAaMut->val;  aaMut != NULL;  aaMut = aaMut->next)
            {
            if (first)
                first = FALSE;
            else
                fputc(',', f);
            fprintf(f, "%s:%s", geneAaMut->name, aaMut->name);
            if (sameString(geneAaMut->name, "S"))
                addSpikeChange(spikeChanges, aaMut->name, sampleId);
            }
        }
    fputc('\t', f);
    // imputed bases (if any)
    struct baseVal *bv;
    for (bv = info->imputedBases;  bv != NULL;  bv = bv->next)
        {
        if (bv != info->imputedBases)
            fputc(',', f);
        fprintf(f, "%d%s", bv->chromStart+1, bv->val);
        }
    fputc('\t', f);
    // path through tree to sample
    printVariantPathNoNodeNames(f, info->variantPath);
    // number of equally parsimonious placements
    fprintf(f, "\t%d", info->bestNodeCount);
    // parsimony score
    fprintf(f, "\t%d", info->parsimonyScore);
    struct seqInfo *si = hashFindVal(seqInfoHash, sampleId);
    if (si)
        {
        if (si->psl)
            {
            // length
            fprintf(f, "\t%d", si->seq->size);
            struct psl *psl = si->psl;
            // aligned bases, indel counts & ranges
            int aliCount = psl->match + psl->misMatch + psl->repMatch;
            fprintf(f, "\t%d\t%d\t%s\t%d\t%s",
                    aliCount, si->insBases, emptyForNull(si->insRanges),
                    si->delBases, emptyForNull(si->delRanges));
            }
        else
            fprintf(f, "\tn/a\tn/a\tn/a\tn/a\tn/a\tn/a");
        // SNVs that were masked (Problematic Sites track), not used in placement
        fputc('\t', f);
        struct singleNucChange *snc;
        for (snc = si->maskedSncList;  snc != NULL;  snc = snc->next)
            {
            if (snc != si->maskedSncList)
                fputc(',', f);
            fprintf(f, "%c%d%c", snc->refBase, snc->chromStart+1, snc->newBase);
            }
        }
    else
        {
        warn("writeOneTsvRow: no sequenceInfo for sample '%s'", sampleId);
        fprintf(f, "\tn/a\tn/a\tn/a\tn/a\tn/a\tn/a\tn/a");
        }
    fprintf(f, "\t%s", isNotEmpty(info->nearestNeighbor) ? info->nearestNeighbor : "n/a");
    fprintf(f, "\t%s", isNotEmpty(info->neighborLineage) ? info->neighborLineage : "n/a");
    fprintf(f, "\t%s", isNotEmpty(info->nextClade) ? info->nextClade : "n/a");
    fprintf(f, "\t%s", isNotEmpty(info->pangoLineage) ? info->pangoLineage : "n/a");
    fputc('\n', f);
    }
}

static void rWriteTsvSummaryTreeOrder(struct phyloTree *node, FILE *f, struct usherResults *results,
                                      struct hash *seqInfoHash, struct geneInfo *geneInfoList,
                                      struct seqWindow *gSeqWin, struct hash *spikeChanges)
/* As we encounter leaves (user-uploaded samples) in depth-first search order, write out a line
 * of TSV summary for each one. */
{
if (node->numEdges)
    {
    int i;
    for (i = 0;  i < node->numEdges;  i++)
        rWriteTsvSummaryTreeOrder(node->edges[i], f, results, seqInfoHash, geneInfoList, gSeqWin,
                                  spikeChanges);
    }
else
    {
    writeOneTsvRow(f, node->ident->name, results, seqInfoHash, geneInfoList, gSeqWin, spikeChanges);
    }
}


static struct hash *hashFromSeqInfoList(struct seqInfo *seqInfoList)
/* Hash sequence name to seqInfo for quick lookup. */
{
struct hash *hash = hashNew(0);
struct seqInfo *si;
for (si = seqInfoList;  si != NULL;  si = si->next)
    hashAdd(hash, si->seq->name, si);
return hash;
}

static struct tempName *writeTsvSummary(struct usherResults *results, struct phyloTree *sampleTree,
                                        struct slName *sampleIds, struct hash *seqInfoHash,
                                        struct geneInfo *geneInfoList, struct seqWindow *gSeqWin,
                                        struct hash *spikeChanges, int *pStartTime)
/* Write a tab-separated summary file for download.  If the user uploaded enough samples to make
 * a tree, then write out samples in tree order; otherwise use sampleIds list.
 * Accumulate S gene changes. */
{
struct tempName *tsvTn = NULL;
AllocVar(tsvTn);
trashDirFile(tsvTn, "ct", "usher_samples", ".tsv");
FILE *f = mustOpen(tsvTn->forCgi, "w");
fprintf(f, "name\tnuc_mutations\taa_mutations\timputed_bases\tmutation_path"
        "\tplacement_count\tparsimony_score_increase\tlength\taligned_bases"
        "\tins_bases\tins_ranges\tdel_bases\tdel_ranges\tmasked_mutations"
        "\tnearest_neighbor\tneighbor_lineage\tnextstrain_clade\tpango_lineage"
        "\n");
if (sampleTree)
    {
    rWriteTsvSummaryTreeOrder(sampleTree, f, results, seqInfoHash, geneInfoList, gSeqWin,
                              spikeChanges);
    }
else
    {
    struct slName *sample;
    for (sample = sampleIds;  sample != NULL;  sample = sample->next)
        writeOneTsvRow(f, sample->name, results, seqInfoHash, geneInfoList, gSeqWin, spikeChanges);
    }
carefulClose(&f);
reportTiming(pStartTime, "write tsv summary");
return tsvTn;
}

static struct tempName *writeSpikeChangeSummary(struct hash *spikeChanges, int totalSampleCount)
/* Write a tab-separated summary of S (Spike) gene changes for download. */
{
struct tempName *tsvTn = NULL;
AllocVar(tsvTn);
trashDirFile(tsvTn, "ct", "usher_S_muts", ".tsv");
FILE *f = mustOpen(tsvTn->forCgi, "w");
fprintf(f, "aa_mutation\tsample_count\tsample_frequency\tsample_ids\tcategory"
        "\n");
struct aaMutInfo *sChanges[spikeChanges->elCount];
struct hashCookie cookie = hashFirst(spikeChanges);
int ix = 0;
struct hashEl *hel;
while ((hel = hashNext(&cookie)) != NULL)
    {
    if (ix >= spikeChanges->elCount)
        errAbort("writeSpikeChangeSummary: hash elCount is %d but got more elements",
                 spikeChanges->elCount);
    sChanges[ix++] = hel->val;
    }
if (ix != spikeChanges->elCount)
    errAbort("writeSpikeChangeSummary: hash elCount is %d but got fewer elements (%d)",
             spikeChanges->elCount, ix);
qsort(sChanges, ix, sizeof(sChanges[0]), aaMutInfoCmp);
for (ix = 0;  ix < spikeChanges->elCount;  ix++)
    {
    struct aaMutInfo *ami = sChanges[ix];
    int sampleCount = slCount(ami->sampleIds);
    fprintf(f, "S:%s\t%d\t%f",
            ami->name, sampleCount, (double)sampleCount / (double)totalSampleCount);
    slReverse(&ami->sampleIds);
    fprintf(f, "\t%s", ami->sampleIds->name);
    struct slName *sample;
    for (sample = ami->sampleIds->next;  sample != NULL;  sample = sample->next)
        fprintf(f, ",%s", sample->name);
    fprintf(f, "\t%s", spikeMutTypeToString(ami->spikeType));
    fputc('\n', f);
    }
carefulClose(&f);
return tsvTn;
}

static struct tempName *makeSubtreeZipFile(struct usherResults *results, struct tempName *jsonTns[],
                                           struct tempName *singleSubtreeJsonTn, int *pStartTime)
/* Make a zip archive file containing all of the little subtree Newick and JSON files so
 * user doesn't have to click on each one. */
{
struct tempName *zipTn;
AllocVar(zipTn);
trashDirFile(zipTn, "ct", "usher_subtrees", ".zip");
int subtreeCount = slCount(results->subtreeInfoList);
char *cmd[10 + 2*(subtreeCount+1)];
char **cmds[] = { cmd, NULL };
int cIx = 0, sIx = 0;
cmd[cIx++] = "zip";
cmd[cIx++] = "-j";
cmd[cIx++] = zipTn->forCgi;
cmd[cIx++] = singleSubtreeJsonTn->forCgi;
cmd[cIx++] = results->singleSubtreeInfo->subtreeTn->forCgi;
struct subtreeInfo *ti;
for (ti = results->subtreeInfoList;  ti != NULL;  ti = ti->next, sIx++)
    {
    cmd[cIx++] = jsonTns[sIx]->forCgi;
    cmd[cIx++] = ti->subtreeTn->forCgi;
    }
cmd[cIx++] = NULL;
struct pipeline *pl = pipelineOpen(cmds, pipelineRead, NULL, NULL, 0);
pipelineClose(&pl);
reportTiming(pStartTime, "make subtree zipfile");
return zipTn;
}

static struct slName **getProblematicSites(char *db)
/* If config.ra specfies maskFile them return array of lists (usually NULL) of reasons that
 * masking is recommended, one per position in genome; otherwise return NULL. */
{
struct slName **pSites = NULL;
char *pSitesFile = phyloPlaceDbSettingPath(db, "maskFile");
if (isNotEmpty(pSitesFile) && fileExists(pSitesFile))
    {
    AllocArray(pSites, chromSize);
    struct bbiFile *bbi = bigBedFileOpen(pSitesFile);
    struct lm *lm = lmInit(0);
    struct bigBedInterval *bb, *bbList = bigBedIntervalQuery(bbi, chrom, 0, chromSize, 0, lm);
    for (bb = bbList;  bb != NULL;  bb = bb->next)
        {
        char *extra = bb->rest;
        char *reason = nextWord(&extra);
        int i;
        for (i = bb->start;  i < bb->end;  i++)
            slNameAddHead(&pSites[i], reason);
        }
    bigBedFileClose(&bbi);
    }
return pSites;
}

static void downloadsRow(char *treeFile, char *sampleSummaryFile, char *spikeSummaryFile,
                         char *subtreeZipFile)
/* Make a row of quick download file links, to appear between the button row & big summary table. */
{
printf("<p><b>Downloads:</b> | ");
printf("<a href='%s' download>Global phylogenetic tree with your sequences</a> | ", treeFile);
printf("<a href='%s' download>TSV summary of sequences and placements</a> | ", sampleSummaryFile);
printf("<a href='%s' download>TSV summary of Spike mutations</a> | ", spikeSummaryFile);
printf("<a href='%s' download>ZIP file of subtree JSON and Newick files</a> | ", subtreeZipFile);
puts("</p>");
}

static int subTreeInfoUserSampleCmp(const void *pa, const void *pb)
/* Compare subtreeInfo by number of user sample IDs (highest number first). */
{
struct subtreeInfo *tiA = *(struct subtreeInfo **)pa;
struct subtreeInfo *tiB = *(struct subtreeInfo **)pb;
return slCount(tiB->subtreeUserSampleIds) - slCount(tiA->subtreeUserSampleIds);
}

static void getProtobufMetadataSource(char *db, char *protobufFile, char **retProtobufPath,
                                      char **retMetadataFile, char **retSource, char **retAliasFile)
/* If the config file specifies a list of tree choices, and protobufFile is a valid choice, then
 * set ret* to the files associated with that choice.  Otherwise fall back on older conf settings.
 * Return the selected treeChoice if there is one. */
{
struct treeChoices *treeChoices = loadTreeChoices(db);
if (treeChoices)
    {
    *retProtobufPath = protobufFile;
    if (isEmpty(*retProtobufPath))
        *retProtobufPath = treeChoices->protobufFiles[0];
    int i;
    for (i = 0;  i < treeChoices->count;  i++)
        if (sameString(treeChoices->protobufFiles[i], *retProtobufPath))
            {
            *retMetadataFile = treeChoices->metadataFiles[i];
            *retSource = treeChoices->sources[i];
            *retAliasFile = treeChoices->aliasFiles[i];
            break;
            }
    if (i == treeChoices->count)
        {
        *retProtobufPath = treeChoices->protobufFiles[0];
        *retMetadataFile = treeChoices->metadataFiles[0];
        *retSource = treeChoices->sources[0];
        *retAliasFile = treeChoices->aliasFiles[0];
        }
    }
else
    {
    // Fall back on old settings
    *retProtobufPath = getUsherAssignmentsPath(db, TRUE);
    *retMetadataFile = phyloPlaceDbSettingPath(db, "metadataFile");
    *retSource = "GISAID";
    *retAliasFile = NULL;
    }
}

static void addModVersion(struct hash *nameHash, char *id, char *fullName)
/* Map id to fullName.  If id has .version, strip that (modifying id!) and map versionless id
 * to fullName. */
{
hashAdd(nameHash, id, fullName);
char *p = strchr(id, '.');
if (p)
    {
    *p = '\0';
    hashAdd(nameHash, id, fullName);
    }
}

static void maybeAddIsolate(struct hash *nameHash, char *name, char *fullName)
/* If name looks like country/isolate/year and isolate looks sufficiently distinctive
 * then also map isolate to fullName. */
{
regmatch_t substrs[2];
if (regexMatchSubstr(name, "^[A-Za-z _]+/(.*)/[0-9]{4}$", substrs, ArraySize(substrs)))
    {
    char isolate[substrs[1].rm_eo - substrs[1].rm_so + 1];
    regexSubstringCopy(name, substrs[1], isolate, sizeof isolate);
    if (! isInternalNodeName(isolate, 0) &&
        (regexMatch(isolate, "[A-Za-z0-9]{4}") ||
         regexMatch(isolate, "[A-Za-z0-9][A-Za-z0-9]+[-_][A-Za-z0-9][A-Za-z0-9]+")))
        {
        hashAdd(nameHash, isolate, fullName);
        }
    }
}
static void addNameMaybeComponents(struct hash *nameHash, char *fullName, boolean addComponents)
/* Add entries to nameHash mapping fullName to itself, and components of fullName to fullName.
 * If addComponents and fullName is |-separated name|ID|date, add name and ID to nameHash. */
{
char *fullNameHashStored = hashStoreName(nameHash, fullName);
// Now that we have hash storage for fullName, make it point to itself.
struct hashEl *hel = hashLookup(nameHash, fullName);
if (hel == NULL)
    errAbort("Can't look up '%s' right after adding it", fullName);
hel->val = fullNameHashStored;
if (addComponents)
    {
    char *words[4];
    char copy[strlen(fullName)+1];
    safecpy(copy, sizeof copy, fullName);
    int wordCount = chopString(copy, "|", words, ArraySize(words));
    if (wordCount == 3)
        {
        // name|ID|date
        hashAdd(nameHash, words[0], fullNameHashStored);
        maybeAddIsolate(nameHash, words[0], fullNameHashStored);
        addModVersion(nameHash, words[1], fullNameHashStored);
        }
    else if (wordCount == 2)
        {
        // ID|date
        addModVersion(nameHash, words[0], fullNameHashStored);
        // ID might be a COG-UK country/isolate/year
        maybeAddIsolate(nameHash, words[0], fullNameHashStored);
        }
    }
}

static void rAddLeafNames(struct phyloTree *node, struct hash *condensedNodes, struct hash *nameHash,
                          boolean addComponents)
/* Recursively descend tree, adding leaf node names to nameHash (including all names of condensed
 * leaf nodes).  Also map components of full names (country/isolate/year|ID|date) to full names. */
{
if (node->numEdges == 0)
    {
    char *leafName = node->ident->name;
    struct slName *nodeList = hashFindVal(condensedNodes, leafName);
    if (nodeList)
        {
        struct slName *sample;
        for (sample = nodeList;  sample != NULL;  sample = sample->next)
            addNameMaybeComponents(nameHash, sample->name, addComponents);
        }
    else
        addNameMaybeComponents(nameHash, leafName, addComponents);
    }
else
    {
    int i;
    for (i = 0;  i < node->numEdges;  i++)
        rAddLeafNames(node->edges[i], condensedNodes, nameHash, addComponents);
    }
}

static void addAliases(struct hash *nameHash, char *aliasFile)
/* If there is an aliasFile, then add its mappings of ID/alias to full tree name to nameHash. */
{
if (isNotEmpty(aliasFile) && fileExists(aliasFile))
    {
    struct lineFile *lf = lineFileOpen(aliasFile, TRUE);
    int missCount = 0;
    char *missExample = NULL;
    char *line;
    while (lineFileNextReal(lf, &line))
        {
        char *words[3];
        int wordCount = chopTabs(line, words);
        lineFileExpectWords(lf, 2, wordCount);
        char *fullName = hashFindVal(nameHash, words[1]);
        if (fullName)
            hashAdd(nameHash, words[0], fullName);
        else
            {
            missCount++;
            if (missExample == NULL)
                missExample = cloneString(words[1]);
            }
        }
    lineFileClose(&lf);
    if (missCount > 0)
        fprintf(stderr, "aliasFile %s: %d values in second column were not found in tree, "
                "e.g. '%s'", aliasFile, missCount, missExample);
    }
}

static struct hash *getTreeNames(struct mutationAnnotatedTree *bigTree, char *aliasFile,
                                 boolean addComponents)
/* Make a hash of full names of leaves of bigTree; if addComponents, also map components of those
 * names to the full names in case the user gives us partial names/IDs. */
{
int nodeCount = bigTree->nodeHash->elCount;
struct hash *nameHash = hashNew(digitsBaseTwo(nodeCount) + 3);
rAddLeafNames(bigTree->tree, bigTree->condensedNodes, nameHash, addComponents);
addAliases(nameHash, aliasFile);
return nameHash;
}

static char *matchName(struct hash *nameHash, char *name)
/* Look for a possibly partial name or ID provided by the user in nameHash.  Return the result,
 * possibly NULL.  If the full name doesn't match, try components of the name. */
{
name = trimSpaces(name);
// GISAID fasta headers all have hCoV-19/country/isolate/year|EPI_ISL_#|date; strip the hCoV-19
// because Nextstrain strips it in nextmeta/nextfasta download files, and so do I when building
// UCSC's tree.
if (startsWithNoCase("hCoV-19/", name))
    name += strlen("hCoV-19/");
char *match = hashFindVal(nameHash, name);
int minWordSize=5;
if (match == NULL && strchr(name, '|'))
    {
    // GISAID fasta headers have name|ID|date, and so do our tree IDs; try ID and name separately.
    char *words[4];
    char copy[strlen(name)+1];
    safecpy(copy, sizeof copy, name);
    int wordCount = chopString(copy, "|", words, ArraySize(words));
    if (wordCount == 3)
        {
        // name|ID|date; try ID first.
        if (strlen(words[1]) > minWordSize)
            match = hashFindVal(nameHash, words[1]);
        if (match == NULL && strlen(words[0]) > minWordSize)
            {
            match = hashFindVal(nameHash, words[0]);
            // Sometimes country/isolate names have spaces... strip out if present.
            if (match == NULL && strchr(words[0], ' '))
                {
                stripChar(words[0], ' ');
                match = hashFindVal(nameHash, words[0]);
                }
            }
        }
    else if (wordCount == 2)
        {
        // ID|date
        if (strlen(words[0]) > minWordSize)
             match = hashFindVal(nameHash, words[0]);
        }
    }
else if (match == NULL && strchr(name, ' '))
    {
    // GISAID sequence names may include spaces, in both country names ("South Korea") and
    // isolate names.  That messes up FASTA headers, so Nextstrain strips out spaces when
    // making the nextmeta and nextfasta download files for GISAID.  Try stripping out spaces:
    char copy[strlen(name)+1];
    safecpy(copy, sizeof copy, name);
    stripChar(copy, ' ');
    match = hashFindVal(nameHash, copy);
    }
return match;
}

static struct slName *readSampleIds(struct lineFile *lf, struct mutationAnnotatedTree *bigTree,
                                    char *aliasFile)
/* Read a file of sample names/IDs from the user; typically these will not be exactly the same
 * as the protobuf's (UCSC protobuf names are typically country/isolate/year|ID|date), so attempt
 * to find component matches if an exact match isn't found. */
{
struct slName *sampleIds = NULL;
struct slName *unmatched = NULL;
struct hash *nameHash = getTreeNames(bigTree, aliasFile, TRUE);
char *line;
while (lineFileNext(lf, &line, NULL))
    {
    // If tab-sep or comma-sep, just try first word in line
    char *tab = strchr(line, '\t');
    if (tab)
        *tab = '\0';
    else
        {
        char *comma = strchr(line, ',');
        if (comma)
            *comma = '\0';
        }
    char *match = matchName(nameHash, line);
    if (match)
        slNameAddHead(&sampleIds, match);
    else
        slNameAddHead(&unmatched, line);
    }
if (unmatched)
    {
    struct dyString *firstFew = dyStringNew(0);
    int maxExamples = 5;
    struct slName *example;
    int i;
    for (i = 0, example = unmatched;  example != NULL && i < maxExamples;
         i++, example = example->next)
        {
        dyStringAppendSep(firstFew, ", ");
        dyStringPrintf(firstFew, "'%s'", example->name);
        }
    warn("Unable to find %d of your sequences in the tree, e.g. %s",
         slCount(unmatched), firstFew->string);
    dyStringFree(&firstFew);
    }
else if (sampleIds == NULL)
    warn("Could not find any names in input; empty file uploaded?");
slNameFreeList(&unmatched);
return sampleIds;
}

char *phyloPlaceSamples(struct lineFile *lf, char *db, char *defaultProtobuf,
                        boolean doMeasureTiming, int subtreeSize, int fontHeight,
                        boolean *retSuccess)
/* Given a lineFile that contains either FASTA, VCF, or a list of sequence names/ids:
 * If FASTA/VCF, then prepare VCF for usher; if that goes well then run usher, report results,
 * make custom track files and return the top-level custom track file.
 * If list of seq names/ids, then attempt to find their full names in the protobuf, run matUtils
 * to make subtrees, show subtree results, and return NULL.  Set retSuccess to TRUE if we were
 * able to get at least some results for the user's input. */
{
char *ctFile = NULL;
if (retSuccess)
    *retSuccess = FALSE;
measureTiming = doMeasureTiming;
int startTime = clock1000();
struct tempName *vcfTn = NULL;
struct slName *sampleIds = NULL;
char *usherPath = getUsherPath(TRUE);
char *protobufPath = NULL;
char *source = NULL;
char *metadataFile = NULL;
char *aliasFile = NULL;
getProtobufMetadataSource(db, defaultProtobuf, &protobufPath, &metadataFile, &source, &aliasFile);
struct mutationAnnotatedTree *bigTree = parseParsimonyProtobuf(protobufPath);
reportTiming(&startTime, "parse protobuf file");
if (! bigTree)
    {
    warn("Problem parsing %s; can't make subtree subtracks.", protobufPath);
    }
lineFileCarefulNewlines(lf);
struct slName **maskSites = getProblematicSites(db);
boolean *informativeBases = informativeBasesFromTree(bigTree->tree, maskSites);
struct dnaSeq *refGenome = hChromSeq(db, chrom, 0, chromSize);
boolean isFasta = FALSE;
boolean subtreesOnly = FALSE;
struct seqInfo *seqInfoList = NULL;
if (lfLooksLikeFasta(lf))
    {
    struct slPair *failedSeqs;
    struct slPair *failedPsls;
    struct hash *treeNames = getTreeNames(bigTree, NULL, FALSE);
    vcfTn = vcfFromFasta(lf, db, refGenome, informativeBases, maskSites, treeNames,
                         &sampleIds, &seqInfoList, &failedSeqs, &failedPsls, &startTime);
    if (failedSeqs)
        {
        puts("<p>");
        struct slPair *fail;
        for (fail = failedSeqs;  fail != NULL;  fail = fail->next)
            printf("%s<br>\n", fail->name);
        puts("</p>");
        }
    if (failedPsls)
        {
        puts("<p>");
        struct slPair *fail;
        for (fail = failedPsls;  fail != NULL;  fail = fail->next)
            printf("%s<br>\n", fail->name);
        puts("</p>");
        }
    if (seqInfoList == NULL)
        printf("<p>Sorry, could not align any sequences to reference well enough to place in "
               "the phylogenetic tree.</p>\n");
    isFasta = TRUE;
    }
else if (lfLooksLikeVcf(lf))
    {
    vcfTn = checkAndSaveVcf(lf, refGenome, maskSites, &seqInfoList, &sampleIds);
    reportTiming(&startTime, "check uploaded VCF");
    }
else
    {
    subtreesOnly = TRUE;
    sampleIds = readSampleIds(lf, bigTree, aliasFile);
    }
lineFileClose(&lf);
struct hash *seqInfoHash = hashFromSeqInfoList(seqInfoList);
if (sampleIds == NULL)
    {
    return ctFile;
    }
struct usherResults *results = NULL;
if (vcfTn)
    {
    fflush(stdout);
    results = runUsher(usherPath, protobufPath, vcfTn->forCgi,
                       subtreeSize, sampleIds, bigTree->condensedNodes,
                       &startTime);
    if (results)
        addSampleMutsFromSeqInfo(results->samplePlacements, seqInfoHash);
    }
else if (subtreesOnly)
    {
    char *matUtilsPath = getMatUtilsPath(TRUE);
    results = runMatUtilsExtractSubtrees(matUtilsPath, protobufPath, subtreeSize,
                                         sampleIds, bigTree->condensedNodes,
                                         &startTime);
    }
if (results && results->singleSubtreeInfo)
    {
    if (retSuccess)
        *retSuccess = TRUE;
    puts("<p></p>");
    readQcThresholds(db);
    int subtreeCount = slCount(results->subtreeInfoList);
    // Sort subtrees by number of user samples (largest first).
    slSort(&results->subtreeInfoList, subTreeInfoUserSampleCmp);
    // Make Nextstrain/auspice JSON file for each subtree.
    char *bigGenePredFile = phyloPlaceDbSettingPath(db, "bigGenePredFile");
    struct geneInfo *geneInfoList = getGeneInfoList(bigGenePredFile, refGenome);
    struct seqWindow *gSeqWin = chromSeqWindowNew(db, chrom, 0, chromSize);
    struct hash *sampleMetadata = getSampleMetadata(metadataFile);
    struct hash *sampleUrls = hashNew(0);
    struct tempName *jsonTns[subtreeCount];
    struct subtreeInfo *ti;
    int ix;
    for (ix = 0, ti = results->subtreeInfoList;  ti != NULL;  ti = ti->next, ix++)
        {
        AllocVar(jsonTns[ix]);
        char subtreeName[512];
        safef(subtreeName, sizeof(subtreeName), "subtreeAuspice%d", ix+1);
        trashDirFile(jsonTns[ix], "ct", subtreeName, ".json");
        treeToAuspiceJson(ti, db, geneInfoList, gSeqWin, sampleMetadata, NULL,
                          jsonTns[ix]->forCgi, source);
        // Add a link for every sample to this subtree, so the single-subtree JSON can
        // link to subtree JSONs
        char *subtreeUrl = nextstrainUrlFromTn(jsonTns[ix]);
        struct slName *sample;
        for (sample = ti->subtreeUserSampleIds;  sample != NULL;  sample = sample->next)
            hashAdd(sampleUrls, sample->name, subtreeUrl);
        }
    struct tempName *singleSubtreeJsonTn;
    AllocVar(singleSubtreeJsonTn);
    trashDirFile(singleSubtreeJsonTn, "ct", "singleSubtreeAuspice", ".json");
    treeToAuspiceJson(results->singleSubtreeInfo, db, geneInfoList, gSeqWin, sampleMetadata,
                      sampleUrls, singleSubtreeJsonTn->forCgi, source);
    struct subtreeInfo *subtreeInfoForButtons = results->subtreeInfoList;
    if (subtreeCount > MAX_SUBTREE_BUTTONS)
        subtreeInfoForButtons = NULL;
    makeButtonRow(singleSubtreeJsonTn, jsonTns, subtreeInfoForButtons, subtreeSize, isFasta,
                  !subtreesOnly);
    printf("<p>If you have metadata you wish to display, click a 'view subtree in "
           "Nextstrain' button, and then you can drag on a CSV file to "
           "<a href='"NEXTSTRAIN_DRAG_DROP_DOC"' target=_blank>add it to the tree view</a>."
           "</p>\n");
    puts("<p><em>Note: "
         "The Nextstrain subtree views, and Download files below, are temporary files and will "
         "expire within two days.  "
         "Please download the Nextstrain subtree JSON files if you will want to view them "
         "again in the future.  The JSON files can be drag-dropped onto "
         "<a href='https://auspice.us/' target=_blank>https://auspice.us/</a>."
         "</em></p>");

    struct tempName *tsvTn = NULL, *sTsvTn = NULL;
    struct tempName *zipTn = makeSubtreeZipFile(results, jsonTns, singleSubtreeJsonTn,
                                                &startTime);
    struct tempName *ctTn = NULL;
    if (subtreesOnly)
        {
        summarizeSubtrees(sampleIds, results, sampleMetadata, jsonTns, bigTree);
        reportTiming(&startTime, "describe subtrees");
        }
    else
        {
        findNearestNeighbors(results->samplePlacements, sampleMetadata, bigTree);

        // Make custom tracks for uploaded samples and subtree(s).
        struct phyloTree *sampleTree = NULL;
        ctTn = writeCustomTracks(vcfTn, results, sampleIds, bigTree,
                                 source, fontHeight, &sampleTree, &startTime);

        // Make a sample summary TSV file and accumulate S gene changes
        struct hash *spikeChanges = hashNew(0);
        tsvTn = writeTsvSummary(results, sampleTree, sampleIds, seqInfoHash,
                                                 geneInfoList, gSeqWin, spikeChanges, &startTime);
        sTsvTn = writeSpikeChangeSummary(spikeChanges, slCount(sampleIds));
        downloadsRow(results->bigTreePlusTn->forHtml, tsvTn->forHtml, sTsvTn->forHtml,
                     zipTn->forHtml);

        int seqCount = slCount(seqInfoList);
        if (seqCount <= MAX_SEQ_DETAILS)
            {
            summarizeSequences(seqInfoList, isFasta, results, jsonTns, sampleMetadata, refGenome);
            reportTiming(&startTime, "write summary table (including reading in lineages)");
            for (ix = 0, ti = results->subtreeInfoList;  ti != NULL;  ti = ti->next, ix++)
                {
                int subtreeUserSampleCount = slCount(ti->subtreeUserSampleIds);
                printf("<h3>Subtree %d: ", ix+1);
                if (subtreeUserSampleCount > 1)
                    printf("%d related samples", subtreeUserSampleCount);
                else if (subtreeCount > 1)
                    printf("Unrelated sample");
                printf("</h3>\n");
                makeNextstrainButtonN("viewNextstrainSub", ix, subtreeUserSampleCount, subtreeSize,
                                      jsonTns);
                puts("<br>");
                // Make a sub-subtree with only user samples for display:
                struct phyloTree *subtree = phyloOpenTree(ti->subtreeTn->forCgi);
                subtree = phyloPruneToIds(subtree, ti->subtreeUserSampleIds);
                describeSamplePlacements(ti->subtreeUserSampleIds, results->samplePlacements,
                                         subtree, sampleMetadata, source);
                }
            reportTiming(&startTime, "describe placements");
            }
        else
            printf("<p>(Skipping details; "
                   "you uploaded %d sequences, and details are shown only when "
                   "you upload at most %d sequences.)</p>\n",
                   seqCount, MAX_SEQ_DETAILS);
        }

    puts("<h3>Downloads</h3>");
    if (! subtreesOnly)
        {
        puts("<ul>");
        // Offer big tree w/new samples for download
        printf("<li><a href='%s' download>SARS-CoV-2 phylogenetic tree "
               "with your samples (Newick file)</a>\n", results->bigTreePlusTn->forHtml);
        printf("<li><a href='%s' download>TSV summary of sequences and placements</a>\n",
               tsvTn->forHtml);
        printf("<li><a href='%s' download>TSV summary of S (Spike) gene changes</a>\n",
               sTsvTn->forHtml);
        }
    printf("<li><a href='%s' download>ZIP archive of subtree Newick and JSON files</a>\n",
           zipTn->forHtml);
    // For now, leave in the individual links so I don't break anybody's pipeline that's
    // scraping this page...
    for (ix = 0, ti = results->subtreeInfoList;  ti != NULL;  ti = ti->next, ix++)
        {
        int subtreeUserSampleCount = slCount(ti->subtreeUserSampleIds);
        printf("<li><a href='%s' download>Subtree with %s", ti->subtreeTn->forHtml,
               ti->subtreeUserSampleIds->name);
        if (subtreeUserSampleCount > 10)
            printf(" and %d other samples", subtreeUserSampleCount - 1);
        else
            {
            struct slName *sln;
            for (sln = ti->subtreeUserSampleIds->next;  sln != NULL;  sln = sln->next)
                printf(", %s", sln->name);
            }
        puts(" (Newick file)</a>");
        printf("<li><a href='%s' download>Auspice JSON for subtree with %s",
               jsonTns[ix]->forHtml, ti->subtreeUserSampleIds->name);
        if (subtreeUserSampleCount > 10)
            printf(" and %d other samples", subtreeUserSampleCount - 1);
        else
            {
            struct slName *sln;
            for (sln = ti->subtreeUserSampleIds->next;  sln != NULL;  sln = sln->next)
                printf(", %s", sln->name);
            }
        puts(" (JSON file)</a>");
        }
    puts("</ul>");

    if (!subtreesOnly)
        {
        // Notify in opposite order of custom track creation.
        puts("<h3>Custom tracks for viewing in the Genome Browser</h3>");
        printf("<p>Added custom track of uploaded samples.</p>\n");
        if (subtreeCount > 0 && subtreeCount <= MAX_SUBTREE_CTS)
            printf("<p>Added %d subtree custom track%s.</p>\n",
                   subtreeCount, (subtreeCount > 1 ? "s" : ""));
        ctFile = urlFromTn(ctTn);
        }
    }
return ctFile;
}
