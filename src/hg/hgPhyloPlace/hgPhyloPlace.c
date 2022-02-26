/* hgPhyloPlace - Upload SARS-CoV-2 sequence for placement in phylo tree. */

/* Copyright (C) 2020 The Regents of the University of California */

#include "common.h"
#include "botDelay.h"
#include "cart.h"
#include "cgiApoptosis.h"
#include "cheapcgi.h"
#include "hCommon.h"
#include "hash.h"
#include "hui.h"
#include "jsHelper.h"
#include "knetUdc.h"
#include "linefile.h"
#include "net.h"
#include "options.h"
#include "phyloPlace.h"
#include "portable.h"
#include "trackLayout.h"
#include "udc.h"
#include "web.h"

/* Global Variables */
struct cart *cart = NULL;      // CGI and other variables
struct hash *oldVars = NULL;   // Old contents of cart before it was updated by CGI
boolean measureTiming = FALSE; // Print out how long things take
char *leftLabelWidthForLongNames = "55";// Leave plenty of room for tree and long virus strain names

/* for botDelay call, 10 second for warning, 20 second for immediate exit */
#define delayFraction   0.25
static boolean issueBotWarning = FALSE;
static long enteredMainTime = 0;

#define seqFileVar "sarsCoV2File"
#define pastedIdVar "namesOrIds"
#define remoteFileVar "remoteFile"

static struct lineFile *lineFileFromFileInput(struct cart *cart, char *fileVar)
/* Return a lineFile on data from an uploaded file with cart variable name fileVar.
 * If the file is binary, attempt to decompress it.  Return NULL if no data are found
 * or if there is a problem decompressing binary data.  If retFileName is not NULL */
{
struct lineFile *lf = NULL;
// Depending on whether the file is plain text or binary, different cart variables are present.
char *filePlainContents = cartOptionalString(cart, fileVar);
char cartVar[2048];
safef(cartVar, sizeof cartVar, "%s__binary", fileVar);
char *fileBinaryCoords = cartOptionalString(cart, cartVar);
// Also get the file name for error reporting.
safef(cartVar, sizeof cartVar, "%s__filename", fileVar);
char *fileName = cartOptionalString(cart, cartVar);
if (fileName == NULL)
    fileName = "<uploaded data>";
if (isNotEmpty(filePlainContents))
    {
    lf = lineFileOnString(fileName, TRUE, cloneString(trimSpaces(filePlainContents)));
    }
else if (isNotEmpty(fileBinaryCoords))
    {
    fprintf(stderr, "%s=%s fileBinaryCoords=%s\n", cartVar, fileName, fileBinaryCoords);
    char *binInfo = cloneString(fileBinaryCoords);
    char *words[2];
    char *mem;
    unsigned long size;
    chopByWhite(binInfo, words, ArraySize(words));
    mem = (char *)sqlUnsignedLong(words[0]);
    size = sqlUnsignedLong(words[1]);
    lf = lineFileDecompressMem(TRUE, mem, size);
    }
return lf;
}

static void newPageStartStuff()
{
// Copied these from hgGtexTrackSettings.c which says "// NOTE: This will likely go to web.c".
puts("<link rel='stylesheet' href='../style/gb.css'>");
puts("<link rel='stylesheet' href='../style/hgGtexTrackSettings.css'>");

//#*** TODO: move this out to a CSS (hardcoding for now because we're doing a standalone push
//#*** independent of the release cycle).
puts("<style>\n"
"#warnBox {\n"
"    border: 3px ridge DarkRed;\n"
"    width:640px;\n"
"    padding:10px; \n"
"    margin:10px;\n"
"    text-align:left;\n"
"}\n"
"\n"
"#warnHead {\n"
"    color: DarkRed;\n"
"}\n"
".readableWidth {\n"
"    max-width: 70em;\n"
"}\n"
"table.seqSummary, table.seqSummary th, table.seqSummary td {\n"
"    border: 1px gray solid;\n"
"    padding: 5px;\n"
"}\n"
".tooltip {\n"
"    position: relative;\n"
"    display: inline-block;\n"
"    border-bottom: 1px dotted black;\n"
"}\n"
"\n"
".tooltip .tooltiptext {\n"
"    visibility: hidden;\n"
"    background-color: lightgray;\n"
"    text-align: center;\n"
"    position: absolute;\n"
"    z-index: 1;\n"
"    opacity: 0;\n"
"    width: 220px;\n"
"    padding: 5px;\n"
"    left: 105%;\n"
"    transition: opacity .6s;\n"
"    line-height: 1em;\n"
"}\n"
"\n"
".tooltip:hover .tooltiptext {\n"
"    visibility: visible;\n"
"    opacity: .9;\n"
"}\n"
"td.qcExcellent {\n"
"    background-color: #44ff44;\n"
"}\n"
"td.qcGood {\n"
"    background-color: #88ff88;\n"
"}\n"
"td.qcMeh {\n"
"    background-color: #ffcc44;\n"
"}\n"
"td.qcBad {\n"
"    background-color: #ff8888;\n"
"}\n"
"td.qcFail {\n"
"    background-color: #ff6666;\n"
"}\n"
".gbSectionBannerLarge {\n"
"    padding: 10px;\n"
"    margin-top: 6px;\n"
"    margin-right: 0;\n"
"    background-color: #4c759c;  /* light blue */\n"
"    color: white;\n"
"    font-weight: bold;\n"
"    font-size: 22px;\n"
"}\n"
"h2 { font-size: 18px; }\n"
"h3 { font-size: 16px; }\n"
"</style>\n"
     );



// Container for bootstrap grid layout
puts(
"<div class='container-fluid'>\n");
}

static void newPageEndStuff()
{
puts(
"</div>");
jsIncludeFile("utils.js", NULL);
webIncludeFile("inc/gbFooter.html");
webEndJWest();
}

#define CHECK_FILE_OR_PASTE_INPUT_JS(fileVarName, pasteVarName) \
    "{ var $fileInput = $('input[name="fileVarName"]');" \
    "  var $pasteInput = $('textarea[name="pasteVarName"]');" \
    "  if ($fileInput && $fileInput[0] && $fileInput[0].files && !$fileInput[0].files.length &&" \
    "      $pasteInput && !$pasteInput.val()) {" \
    "     alert('Please either choose a file or paste in sequence names/IDs first, ' +" \
    "           'and then click the upload button.');" \
    "     return false; " \
    "   } else if ($fileInput && $fileInput[0] && $fileInput[0].files && " \
    "              !!$fileInput[0].files.length &&" \
    "              $pasteInput && !!$pasteInput.val()) {" \
    "     alert('Sorry, unable to process both a file and pasted-in sequence names/IDs at the ' +" \
    "            'same time.  Please clear one or the other and then click the upload button.');" \
    "     return false; " \
    "   } else { loadingImage.run(); return true; } }"

static void inputForm()
/* Ask the user for FASTA or VCF. */
{
printf("<form action='%s' name='mainForm' method=POST enctype='multipart/form-data'>\n\n",
       "hgPhyloPlace");
cartSaveSession(cart);
char *db = "wuhCor1";
cgiMakeHiddenVar("db", db);
puts("<div class='readableWidth'>");
puts("  <div class='gbControl col-md-12'>");
puts("<div style='font-size: 20px; font-weight: 500; margin-top: 15px; margin-bottom: 10px;'>"
     "Place your SARS-CoV-2 sequences in a global phylogenetic tree</div>");
printf("<p>Select your FASTA, VCF or list of sequence names/IDs: ");
printf("<input type='file' id='%s' name='%s'>",
       seqFileVar, seqFileVar);
printf("</p><p>or paste in sequence names/IDs:<br>\n");
cgiMakeTextArea(pastedIdVar, "", 10, 70);
struct treeChoices *treeChoices = loadTreeChoices(db);
if (treeChoices)
    {
    puts("</p><p>");
    printf("Phylogenetic tree version: ");
    char *phyloPlaceTree = cartOptionalString(cart, "phyloPlaceTree");
    cgiMakeDropListWithVals("phyloPlaceTree", treeChoices->descriptions, treeChoices->protobufFiles,
                            treeChoices->count, phyloPlaceTree);
    }
puts("</p><p>");
printf("Number of samples per subtree showing sample placement: ");
int subtreeSize = cartUsualInt(cart, "subtreeSize", 50);
cgiMakeIntVarWithLimits("subtreeSize", subtreeSize,
                        "Number of samples in subtree showing neighborhood of placement",
                        5, 10, 5000);
puts("</p><p>");
cgiMakeOnClickSubmitButton(CHECK_FILE_OR_PASTE_INPUT_JS(seqFileVar, pastedIdVar),
                           "submit", "Upload");
puts("&nbsp;&nbsp;");
cgiMakeOnClickSubmitButton("{ loadingImage.run(); return true; }",
                           "exampleButton", "Upload Example File");
puts("&nbsp;&nbsp;");
puts("<a href='https://github.com/russcd/USHER_DEMO/' target=_blank>More example files</a>");
puts("</p>");
// Add a loading image to reassure people that we're working on it when they upload a big file
printf("<div><img id='loadingImg' src='../images/loading.gif' />\n");
printf("<span id='loadingMsg'></span></div>\n");
jsInline("$(document).ready(function() {\n"
         "    loadingImage.init($('#loadingImg'), $('#loadingMsg'), "
         "'<p style=\"color: red; font-style: italic;\">Uploading and processing your sequences "
         "may take some time. Please leave this window open while we work on your sequences.</p>');"
         "});\n");

puts("  </div>");
puts("</div>");
puts("<div class='readableWidth'>");
puts("  <div class='gbControl col-md-12'>");
puts("<h2>More information</h2>");
puts("<p>Upload your SARS-CoV-2 sequence (FASTA or VCF file) to find the most similar\n"
     "complete, high-coverage samples from \n"
     "<a href='https://www.gisaid.org/' target='_blank'>GISAID</a>\n"
     "or from public sequence databases ("
     "<a href='https://www.ncbi.nlm.nih.gov/labs/virus/vssi/#/virus?SeqType_s=Nucleotide&VirusLineage_ss=SARS-CoV-2,%20taxid:2697049' "
     "target=_blank>NCBI Virus / GenBank</a>,\n"
     "<a href='https://www.cogconsortium.uk/data/' target=_blank>COG-UK</a> and the\n"
     "<a href='https://bigd.big.ac.cn/ncov/release_genome' "
     "target=_blank>China National Center for Bioinformation</a>), "
     "and your sequence's placement in the phylogenetic tree generated by the\n"
     "<a href='https://github.com/roblanf/sarscov2phylo' target='_blank'>sarscov2phylo</a>\n"
     "pipeline.\n"
     "Placement is performed by\n"
     "<a href='https://github.com/yatisht/usher' target=_blank>"
     "Ultrafast Sample placement on Existing tRee (UShER)</a> "
     "(<a href='https://www.nature.com/articles/s41588-021-00862-7' target=_blank>"
     "Turakhia <em>et al.</em></a>).  UShER also generates local subtrees to show samples "
     "in the context of the most closely related sequences.  The subtrees can be visualized "
     "as Genome Browser custom tracks and/or using "
     "<a href='https://nextstrain.org' target=_blank>Nextstrain</a>'s interactive display "
     "which supports "
     "<a href='"NEXTSTRAIN_DRAG_DROP_DOC"' "
     "target=_blank>drag-and-drop</a> of local metadata that remains on your computer.</p>\n");
puts("<p>\n"
     "GISAID data displayed in the Genome Browser are subject to GISAID's\n"
     "<a href='https://www.gisaid.org/registration/terms-of-use/' target=_blank>"
     "Terms and Conditions</a>.\n"
     "SARS-CoV-2 genome sequences and metadata are available for download from\n"
     "<a href='https://gisaid.org' target=_blank>GISAID</a> EpiCoV&trade;.\n"
     "</p>");
puts("<p>\n"
     "<a href='/covid19.html'>COVID-19 Pandemic Resources at UCSC</a></p>\n");
puts("</div>");
puts("</div>");
puts("<div class='readableWidth'>");
puts("  <div class='gbControl col-md-12'>");
puts("<h2>Privacy and sharing</h2>");
puts("<h3>Please do not upload "
     "<a href='https://en.wikipedia.org/wiki/Protected_health_information#United_States' "
     "target=_blank>Protected Health Information (PHI)</a>.</h3>\n"
     "If even virus sequence files must remain local on your computer, then you can try "
     "<a href='https://shusher.gi.ucsc.edu/' target=_blank>ShUShER</a> "
     "which runs entirely in your web browser so that no files leave your computer."
     "</p>\n"
     "<p>We do not store your information "
     "(aside from the information necessary to display results)\n"
     "and will not share it with others unless you choose to share your Genome Browser view.</p>\n"
     "<p>In order to enable rapid progress in SARS-CoV-2 research and genomic contact tracing,\n"
     "please share your SARS-CoV-2 sequences by submitting them to an "
     "<a href='https://ncbiinsights.ncbi.nlm.nih.gov/2020/08/17/insdc-covid-data-sharing/' "
     "target=_blank>INSDC</a> member institution\n"
     "(<a href='https://submit.ncbi.nlm.nih.gov/sarscov2/' target=_blank>NCBI</a>,\n"
     "<a href='https://www.covid19dataportal.org/submit-data' target=_blank>EMBL-EBI</a>\n"
     "or <a href='https://www.ddbj.nig.ac.jp/ddbj/websub.html' target=_blank>DDBJ</a>)\n"
     "and <a href='https://www.gisaid.org/' target=_blank>GISAID</a>.\n"
     "</p>\n");
puts("</div>");
puts("  </div>");
puts("<div class='readableWidth'>");
puts("<div class='gbControl col-md-12'>");
puts("<h2>Tutorial</h2>");
puts("<iframe width='267' height='150' src='https://www.youtube.com/embed/humQ1NyZOUM' "
     "frameborder='0' allow='accelerometer; autoplay; clipboard-write; encrypted-media; "
     "gyroscope; picture-in-picture' allowfullscreen></iframe>\n"
     "<h3><a href='https://www.cdc.gov/amd/pdf/slidesets/ToolkitModule_3.3-508C.pdf' "
     "target=_blank>Slides for tutorial</a></h3>\n"
     "<h3><a href='https://www.cdc.gov/amd/training/covid-19-gen-epi-toolkit.html' target=_blank>"
     "More tutorials from CDC COVID-19 Genomic Epidemiology Toolkit</a></h3>\n"
     "</p>"
     );
puts("</div>");
puts("</div>");
puts("</form>");
}

static void mainPage(char *db)
{
// Start web page with new-style header
webStartGbNoBanner(cart, db, "UShER: Upload");
jsIncludeFile("jquery.js", NULL);
jsIncludeFile("ajax.js", NULL);
newPageStartStuff();

puts("<div class='row'>"
     "  <div class='row gbSectionBannerLarge'>\n"
     "    <div class='col-md-11'>UShER: Ultrafast Sample placement on Existing tRee</div>\n"
     "    <div class='col-md-1'></div>\n"
     "  </div>\n"
     "</div>\n"
     "<div class='row'>\n");
if (hgPhyloPlaceEnabled())
    {
    inputForm();
    }
else
    {
    puts("  <div class='gbControl col-md-12'>");
    puts("  Sorry, this server is not configured to perform phylogenetic placement.");
    puts("  </div>");
    }
puts("</div>\n");

newPageEndStuff();
}

static void resultsPage(char *db, struct lineFile *lf)
/* QC the user's uploaded sequence(s) or VCF; if input looks valid then run usher
 * and display results. */
{
webStartGbNoBanner(cart, db, "UShER: Results");
jsIncludeFile("jquery.js", NULL);
jsIncludeFile("ajax.js", NULL);
newPageStartStuff();

if (issueBotWarning)
    {
    char *ip = getenv("REMOTE_ADDR");
    botDelayMessage(ip, botDelayMillis);
    }

// Allow 10 minutes for big sets of sequences
lazarusLives(15 * 60);

puts("<div class='row'>"
     "  <div class='row gbSectionBannerLarge'>\n"
     "    <div class='col-md-11'>UShER: Ultrafast Sample placement on Existing tRee</div>\n"
     "    <div class='col-md-1'></div>\n"
     "  </div>\n"
     "</div>\n"
     "<div class='row'>\n");
// Form submits subtree custom tracks to hgTracks
printf("<form action='%s' name='resultsForm' method=%s>\n\n",
       hgTracksName(), cartUsualString(cart, "formMethod", "POST"));
cartSaveSession(cart);
puts("  <div class='gbControl col-md-12'>");
fflush(stdout);

if (lf != NULL)
    {
    // Use trackLayout to get hgTracks parameters relevant to displaying trees:
    struct trackLayout tl;
    trackLayoutInit(&tl, cart);
    // Do our best to place the user's samples, make custom tracks if successful:
    char *phyloPlaceTree = cartOptionalString(cart, "phyloPlaceTree");
    int subtreeSize = cartUsualInt(cart, "subtreeSize", 50);
    boolean success = FALSE;
    char *ctFile = phyloPlaceSamples(lf, db, phyloPlaceTree, measureTiming, subtreeSize,
                                     tl.fontHeight, &success);
    if (ctFile)
        {
        cgiMakeHiddenVar(CT_CUSTOM_TEXT_VAR, ctFile);
        if (tl.leftLabelWidthChars < 0 || tl.leftLabelWidthChars == leftLabelWidthDefaultChars)
            cgiMakeHiddenVar(leftLabelWidthVar, leftLabelWidthForLongNames);
        cgiMakeButton("submit", "view in Genome Browser");
        puts("  </div>");
        puts("</form>");
        }
    else if (! success)
        {
        puts("<p></p>");
        puts("  </div>");
        puts("</form>");
        // Let the user upload something else and try again:
        inputForm();
        }
    }
else
    {
    warn("Unable to read your uploaded data - please choose a file and try again, or click the "
         "&quot;try example&quot; button.");
    // Let the user try again:
    puts("  </div>");
    puts("</form>");
    inputForm();
    }
puts("</div>\n");

newPageEndStuff();
}

static void doMiddle(struct cart *theCart)
/* Set up globals and make web page */
{
cart = theCart;
char *db = NULL, *genome = NULL, *clade = NULL;
getDbGenomeClade(cart, &db, &genome, &clade, oldVars);

int timeout = cartUsualInt(cart, "udcTimeout", 300);
if (udcCacheTimeout() < timeout)
    udcSetCacheTimeout(timeout);
knetUdcInstall();

measureTiming = cartUsualBoolean(cart, "measureTiming", measureTiming);

char *submitLabel = cgiOptionalString("submit");
char *newExampleButton = cgiOptionalString("exampleButton");
if ((submitLabel && sameString(submitLabel, "try example")) ||
    (newExampleButton && sameString(newExampleButton, "Upload Example File")))
    {
    char *exampleFile = phyloPlaceDbSettingPath(db, "exampleFile");
    struct lineFile *lf = lineFileOpen(exampleFile, TRUE);
    resultsPage(db, lf);
    }
else if (cgiOptionalString(remoteFileVar))
    {
    char *url = cgiString(remoteFileVar);
    struct lineFile *lf = netLineFileOpen(url);
    resultsPage(db, lf);
    }
else if (isNotEmpty(trimSpaces(cgiOptionalString(pastedIdVar))))
    {
    char *pastedIds = cgiString(pastedIdVar);
    struct lineFile *lf = lineFileOnString("pasted names/IDs", TRUE, pastedIds);
    resultsPage(db, lf);
    }
else if (cgiOptionalString(seqFileVar) || cgiOptionalString(seqFileVar "__filename"))
    {
    struct lineFile *lf = lineFileFromFileInput(cart, seqFileVar);
    resultsPage(db, lf);
    }
else
    mainPage(db);
}

#define LD_LIBRARY_PATH "LD_LIBRARY_PATH"

static void addLdLibraryPath()
/* usher requires a tbb lib that is not in the yum package tbb-devel, so for now
 * I'm adding the .so files to hgPhyloPlaceData.  Set environment variable LD_LIBRARY_PATH
 * to pick them up from there. */
{
char *oldValue = getenv(LD_LIBRARY_PATH);
struct dyString *dy = dyStringNew(0);
if (startsWith("/", PHYLOPLACE_DATA_DIR))
    dyStringAppend(dy, PHYLOPLACE_DATA_DIR);
else
    {
    char cwd[4096];
    getcwd(cwd, sizeof cwd);
    dyStringPrintf(dy, "%s/%s", cwd, PHYLOPLACE_DATA_DIR);
    }
if (isNotEmpty(oldValue))
    dyStringPrintf(dy, ":%s", oldValue);
setenv(LD_LIBRARY_PATH, dyStringCannibalize(&dy), TRUE);
}

int main(int argc, char *argv[])
/* Process command line. */
{
/* Null terminated list of CGI Variables we don't want to save to cart */
char *excludeVars[] = {"submit", "Submit",
                       seqFileVar, seqFileVar "__binary", seqFileVar "__filename",
                       pastedIdVar,
                       NULL};
enteredMainTime = clock1000();
issueBotWarning = earlyBotCheck(enteredMainTime, "hgPhyloPlace", delayFraction, 0, 0, "html");

cgiSpoof(&argc, argv);
oldVars = hashNew(10);
addLdLibraryPath();
cartEmptyShellNoContent(doMiddle, hUserCookie(), excludeVars, oldVars);
cgiExitTime("hgPhyloPlace", enteredMainTime);
return 0;
}
