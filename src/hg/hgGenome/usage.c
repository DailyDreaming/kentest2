/* usage - functions that display helpful usage info text go here. -jk)*/

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hPrint.h"


void printMainHelp()
/* Put up main page help info. */
{
hPrintf("%s",

"<P>Genome Graphs is a tool for displaying genome-wide data sets such\n"
"as the results of genome-wide SNP association studies, linkage studies\n"
"and homozygosity mapping. This section provides line-by-line descriptions\n"
"of the controls on this page. For more detailed instructions, see the\n"
"<A HREF=\"../goldenPath/help/hgGenomeHelp.html\" TARGET=\"_blank\">Genome\n"
"Graphs User's Guide</A>.\n"
"        <UL>\n"
"        <LI><B>clade: </B>Specifies which clade the organism is in."
"        </LI>\n"
"        <LI><B>genome: </B>Specifies which organism data to use."
"        </LI>\n"
"        <LI><B>assembly: </B>Specifies which version of the organism's"
"        genome sequence to use.\n"
"        </LI>\n"
"        <LI><B>graph ... in ...:</B> Selects which graph(s) to display in "
"        which color."
"        </LI>\n"
"        <LI><B>upload: </B> Takes you to a page where you can upload your "
"        own data.\n"
"        </LI>\n"
"        <LI><B>import: </B> Takes you to a page where you can import "
"        database tables from this assembly.\n"
"        </LI>\n"
"        <LI><B>configure: </B> Takes you to a page where you can control "
"        many aspects of the display including the overall size of the image, "
"        how many graphs can be drawn, and the chromosome layout."
"        </LI>\n"
"        <LI><B>correlate: </B> If more than one graph is selected, takes you "
"        to a page listing the Pearson's correlation coefficient for each "
"        pair of graphs."
"        </LI>\n"
"        <LI><B>significance threshold: </B> Values above this threshold will "
"        be considered significant. A light blue line will be drawn across "
"        the graphs at the significance threshold of the first graph. Regions "
"        above the threshold will be included in the region list you get with "
"        the <EM>browse regions</EM> button."
"        </LI>\n"
"        <LI><B>browse regions: </B> Takes you to a page with a list of all "
"        regions above the <EM>significance threshold</EM> on the left, and a "
"        Genome Browser on the right.  Clicking on a region will move the "
"        browser's window to that region."
"        </LI>\n"
"        <LI><B>sort genes: </B> Opens the Gene Sorter with a filter to "
"        display only those genes in regions that are above the "
"        <EM>significance threshold</EM>. If the graph was uploaded using"
"        markers, then a custom Gene Sorter column with the same name as the "
"        graph will be created.  This column will list all markers for each "
"        gene that contain values above the significance threshold."
"        </LI>\n"
"        </UL>\n");
}
