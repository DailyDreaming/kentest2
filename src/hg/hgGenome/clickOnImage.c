/* clickOnImage - handle click on image. Calculate position in genome
 * and forward to genome browser. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hash.h"
#include "jksql.h"
#include "cheapcgi.h"
#include "chromGraph.h"
#include "genoLay.h"
#include "binRange.h"
#include "hdb.h"
#include "hgGenome.h"


struct genoLayChrom *genoLayChromAt(struct genoLay *gl, int x, int y)
/* Return chromosome if any at x,y */
{
struct genoLayChrom *chrom;
for (chrom = gl->chromList; chrom != NULL; chrom = chrom->next)
    {
    if (chrom->x <= x && x < chrom->x + chrom->width
       && chrom->y <= y && y < chrom->y + chrom->height)
       break;
    }
return chrom;
}

void clickOnImage(struct sqlConnection *conn)
/* Handle click on image - calculate position in forward to genome browser. */
{
int x = cartInt(cart, hggClickX);
int y = cartInt(cart, hggClickY);
struct genoLay *gl = ggLayout(conn, linesOfGraphs(), graphsPerLine());
struct genoLayChrom *chrom = genoLayChromAt(gl, x, y);
if (chrom != NULL)
    {
    int base = gl->basesPerPixel*(x - chrom->x);
    int start = base-500000, end=base+500000;
    if (start<0) start=0;
    printf("Location: ../cgi-bin/hgTracks?db=%s&%s&position=%s:%d-%d&hgGenomeClick=image\r\n\r\n",
    	database, cartSidUrlString(cart), chrom->fullName, start+1, end);
    }
else
    {
    hggDoUsualHttp();
    }
}
