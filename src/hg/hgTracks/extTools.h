/* extTools -- code to parse list of external tools and redirect the user to them */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef EXTTOOLS_H
#define EXTTOOLS_H

struct extTool
/* describes an external tool */
{
    struct extTool *next; /* next in singly linked list */
    char *tool;  	  /* internal ID of an external tool, e.g. primer3  */
    char *shortLabel;  	  /* shown in the drop down menu */
    char *longLabel;      /* label for mouse over */
    char *url;            /* target URL of the tool CGI */
    char *email;          /* email address of external tool */
    bool isHttpGet;      /* default is a POST request. Set to 1/on/true to make it a http GET request */
    int maxSize;          /* maximum size in bp that can be handled by tool or 0 for any */
    struct slPair *params;	          /* CGI vars we have to send to the tool */
    struct slName *dbs;	          /* if not null: list of DBS to show the entry */
    struct slName *notDbs;	          /* if not null: list of DBS where entry should NOT be shown */
};

struct extTool *readExtToolRa(char *raFileName);
/* parse the extTools.ra file. */

void extToolRedirect(struct cart *cart, char *tool);
/* redirect to an external tool, sending the data specified in the ext tools config file */

boolean extToolsEnabled();
/* Return TRUE if we can display the external tools menu. */

void printExtMenuData();
/* print the external tools aka "send to" menu entries as JSON to stdout */
#endif
