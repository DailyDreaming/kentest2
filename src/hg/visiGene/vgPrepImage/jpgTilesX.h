/* Copyright (C) 2010 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

/****************************************************
** 
** FILE:   	jpgTilesX.h 
** CREATED:	6th February 2006
** AUTHOR: 	Galt Barber
**
** PURPOSE:	Convert input RGB data rows to jpg tile-set and thumbnail and optionally 
**              a full-size jpg for downloading as well.
**		Call jpgTiles with image width and height and outdir and outpath 
**              Quality settings can also be optionally specified.
**                 [-quality N|-variable 50 60 70 80 85]
**              Currently just operates on one output image tileset and stops,
**              so run the program again for each input image.
**
** NOTES:
**      Keeps the same size image as input and uses RGB 3-band 24-bit color
**      with default quality 75%. Caller must create a subdir under outdir with the root name of
**      the infile, i.e. infile/  and it puts the tiles for levels 0-4 there, 
**      each higher level zooms out by 2 in both dimensions.
**      N is % quality, from 3 to 95 and is optional, otherwise uses libjpeg default (75%).
**
*******************************************************/

// This is all that client needs to know to use it,
// a minimal include reduces conflicts when including multiple external libraries

void jpgTiles(int nWidth, int nHeight,
    char *outFullDir, 
    char *outFullRoot, 
    char *outThumbPath, 
    unsigned char *(*readScanline)(), 
    int *inQuality, 
    int makeFullSize   // boolean
    );
/* encode jpg tiles and thumbnail for given image, 
 * optionally create fullsize image too (e.g. when input is from jp2 source)
 * optionally specify jpg quality for each of 5 levels in int array */

