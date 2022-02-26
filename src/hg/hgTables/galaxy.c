/* galaxy - stuff related to galaxy simple queries and db info. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "jksql.h"
#include "cheapcgi.h"
#include "trackDb.h"
#include "customTrack.h"
#include "hdb.h"
#include "web.h"
#include "portable.h"
#include "hui.h"
#include "featureBits.h"
#include "hgTables.h"
#include "obscure.h"
#include "cart.h"
#include "grp.h"
#include "net.h"
#include "htmlPage.h"
#include "wiggle.h"
#include "trashDir.h"
#include "hgConfig.h"


char *getGalaxyUrl()
/* Returns the url for the galaxy cgi; default can be overriden by cart var GALAXY_URL.
 * Don't free result. */
{
char *url = NULL;
/* use parameter if available */
if (cartVarExists(cart, "GALAXY_URL"))
    url = cartString(cart, "GALAXY_URL");
else 
    {
    if (cfgOption("galaxyUrl"))
        url = cfgOption("galaxyUrl");
    else
        url = GALAXY_URL_APP;
    }
return url;
}

void galaxyHandler (char *format, va_list args)
/* error Handler that passes error on to Galaxy */
{
char msg[512];
sprintf(msg, format, args);
noWarnAbort();
}

void sendParamsToGalaxy(char *doParam, char *paramVal)
/* intermediate page for formats printed directly from top form */
{
char *shortLabel = curTable;
char *output = cartString(cart, hgtaOutputType);

if (curTrack != NULL)
    shortLabel = curTrack->shortLabel;
htmlOpen("Output %s as %s", "results to Galaxy", shortLabel);
if (sameString(output, outPrimaryTable) || sameString(output, outSelectedFields))
    {
    if (anySubtrackMerge(database, curTable))
        errAbort("Can't do all fields output when subtrack merge is on. "
        "Please go back and select another output type (BED or custom track is good), or clear the subtrack merge.");
    if (anyIntersection())
        errAbort("Can't do all fields output when intersection is on. "
        "Please go back and select another output type (BED or custom track is good), or clear the intersection.");

    }
if (isMafTable(database, curTrack, curTable))
    {
    hPrintf("You are about to download a VERY large dataset to Galaxy. "
            "For most usage scenarios it is not necessary as Galaxy already "
            "stores alignments locally. Click here ("
            "<A HREF=\"http://wiki.galaxyproject.org/Main/MAF%%20Analysis\" TARGET=_blank>http://wiki.galaxyproject.org/Main/MAF%%20Analysis</A>"
            ") for more information on how to analyze multiple alignments "
            "through Galaxy.\n<BR><BR>");
    }
startGalaxyForm();
/* send the hgta_do parameter that won't be in the cart */
cgiMakeHiddenVar(doParam, paramVal);
/* need to send sessionId */
cgiMakeHiddenVar(cartSessionVarName(cart), cartSessionId(cart));
printGalaxySubmitButtons();
htmlClose();
}

void startGalaxyForm ()
/* start form to send parameters to Galaxy, also send required params */
{
char selfUrl[256];
char *hguid = cartUserId(cart);

hPrintf("<FORM ACTION=\"%s\" METHOD=POST>\n", getGalaxyUrl());
/* copy cart parameters into hidden fields to send to Galaxy */
hPrintf("\n"); /* make more readable */
/* Galaxy requires tool_id and URL */
/* set default if no tool_id for Galaxy */
if (!cartVarExists(cart, "tool_id"))
    cgiMakeHiddenVar("tool_id", "ucsc_table_direct1");
else
    cgiMakeHiddenVar("tool_id", cartString(cart, "tool_id"));
safef(selfUrl, sizeof(selfUrl), "http://%s%s", cgiServerNamePort(), cgiScriptName());
cgiMakeHiddenVar("URL", selfUrl);
hPrintf("\n");
/* forward user parameters */
if (hguid)
    {
    cgiMakeHiddenVar("hguid", hguid);
    hPrintf("\n");
    }
/* send database and organism and table for Galaxy's info */
cgiMakeHiddenVar("db", database);
if (cartVarExists(cart, "org"))
    cgiMakeHiddenVar("org", cartString(cart, "org"));
cgiMakeHiddenVar("hgta_table", curTable);
cgiMakeHiddenVar(hgtaTrack, cartString(cart, hgtaTrack));
if (cartVarExists(cart, "hgta_regionType"))
    cgiMakeHiddenVar("hgta_regionType", cartString(cart, "hgta_regionType"));
if (cartVarExists(cart, "hgta_outputType"))
    cgiMakeHiddenVar("hgta_outputType", cartString(cart, "hgta_outputType"));
if (cartVarExists(cart, "position"))
    cgiMakeHiddenVar("position", cartString(cart, "position"));
}

void printGalaxySubmitButtons ()
/* print submit button to send query results to Galaxy */
{
cgiMakeButton(hgtaDoGalaxyQuery, "Send query to Galaxy");
hPrintf("</FORM>\n");
hPrintf(" ");
/* new form as action is different */
hPrintf("<FORM ACTION=\"%s\" METHOD=GET>\n", cgiScriptName());
cgiMakeButton(hgtaDoMainPage, "Cancel");
hPrintf("</FORM>\n");
}

boolean doGalaxy ()
/* has the send to Galaxy checkbox been selected? */
{
return cartUsualBoolean(cart, "sendToGalaxy", FALSE);
}
