/* gtexEqtlClusterTrack - Track of clustered expression QTL's from GTEx */

/* Copyright (C) 2017 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "dystring.h"
#include "hgTracks.h"
#include "gtexInfo.h"
#include "gtexTissue.h"
#include "gtexUi.h"
#include "gtexEqtlCluster.h"

struct gtexEqtlClusterTrack
/* Track info for filtering (maybe later for draw, map) */
{
    struct gtexTissue *tissues; /*  Tissue names, descriptions */
    struct hash *tissueHash;    /* Tissue info, keyed by UCSC name, filtered by UI */
    boolean doTissueColor;      /* Display tissue color (for single-tissue eQTL's) */
    double minEffect;           /* Effect size filter (abs value) */
    double minProb;             /* Probability filter */
};

/* Utility functions */

static struct gtexEqtlCluster *loadOne(char **row)
/* Load up gtexEqtlCluster from array of strings. */
{
return gtexEqtlClusterLoad(row);
}

static boolean filterTissuesFromCart(struct track *track, struct gtexEqtlClusterTrack *extras)
/* Check cart for tissue selection. Populate track tissues hash */
{
char *version = gtexVersionSuffix(track->table);
extras->tissues = gtexGetTissues(version);
extras->tissueHash = hashNew(0);
struct gtexTissue *tis = NULL;
for (tis = extras->tissues; tis != NULL; tis = tis->next)
    hashAdd(extras->tissueHash, tis->name, tis);

// if all tissues included, return full hash
if (!cartListVarExistsAnyLevel(cart, track->tdb, FALSE, GTEX_TISSUE_SELECT))
    return FALSE;
struct slName *selectedValues = cartOptionalSlNameListClosestToHome(cart, track->tdb,
                                                    FALSE, GTEX_TISSUE_SELECT);
if (selectedValues == NULL || slCount(selectedValues) == slCount(extras->tissues))
    return FALSE;

// create tissue hash with only included tissues
struct hash *tisHash = hashNew(0);

struct slName *name;
for (name = selectedValues; name != NULL; name = name->next)
    {
    tis = (struct gtexTissue *)hashFindVal(extras->tissueHash, name->name);
    if (tis != NULL)
        hashAdd(tisHash, name->name, tis);
    }
extras->tissueHash = tisHash;
return TRUE;
}

static void eqtlExcludeTissue(struct gtexEqtlCluster *eqtl, int i)
/* Mark the tissue to exclude from display */
{
eqtl->expScores[i] = 0.0;
}

static boolean eqtlIsExcludedTissue(struct gtexEqtlCluster *eqtl, int i)
/* Check if eQTL is excluded */
{
return (eqtl->expScores[i] == 0.0);
}

static boolean eqtlIncludeFilter(struct track *track, void *item)
/* Apply all filters (except gene) to eQTL item. Invoked by filterTissues method */
{
int i;
int excluded = 0;
double maxEffect = 0.0;
double maxProb = 0.0;
struct gtexEqtlCluster *eqtl = (struct gtexEqtlCluster *)item;
struct gtexEqtlClusterTrack *extras = (struct gtexEqtlClusterTrack *)track->extraUiData;
for (i=0; i<eqtl->expCount; i++)
    {
    if (hashFindVal(extras->tissueHash, eqtl->expNames[i]))
        {
        double effect = eqtl->expScores[i];
        double prob = eqtl->expProbs[i];
        maxEffect = fmax(maxEffect, fabs(effect));
        maxProb = fmax(maxProb, fabs(prob));
        if (effect < extras->minEffect || prob < extras->minProb)
            {
            eqtlExcludeTissue(eqtl, i);
            excluded++;
            }
        }
    else
        {
        eqtlExcludeTissue(eqtl, i);
        excluded++;
        }
    }
// exclude if no tissues match selector
if (excluded == eqtl->expCount || 
    // or if variant has no selected tissue where effect size is above cutoff
    maxEffect < extras->minEffect || 
    // or if variant has no selected tissue where probability is above cutoff
    maxProb < extras->minProb)
        return FALSE;
return TRUE;
}

static int eqtlTissueCount(struct gtexEqtlCluster *eqtl)
    /* Return count of non-excluded tissues in the item */
{
int included = 0;
int i;
for (i=0; i<eqtl->expCount; i++)
    if (!eqtlIsExcludedTissue(eqtl, i))
        included++;
return included;
}

static int eqtlTissueIndex(struct gtexEqtlCluster *eqtl)
/* Return index of first non-excluded tissue in an eQTL cluster. Used for single-tissue items. */
{
int i;
for (i=0; i<eqtl->expCount; i++)
    if (!eqtlIsExcludedTissue(eqtl, i))
        return i;
return -1;
}

static struct rgbColor eqtlTissueColor(struct track *track, struct gtexEqtlCluster *eqtl)
/* Return tissue color for single-tissue item, or NULL if none found */
{
int i = eqtlTissueIndex(eqtl);
assert(i>=0);
struct gtexEqtlClusterTrack *extras = (struct gtexEqtlClusterTrack *)track->extraUiData;
struct gtexTissue *tis = (struct gtexTissue *)hashFindVal(extras->tissueHash, eqtl->expNames[i]);
assert (tis);
return (struct rgbColor){.r=COLOR_32_BLUE(tis->color), .g=COLOR_32_GREEN(tis->color), 
                .b=COLOR_32_RED(tis->color)};
}

static char *eqtlSourcesLabel(struct gtexEqtlCluster *eqtl)
/* Right label is tissue (or number of tissues if >1) */
{
int ct = eqtlTissueCount(eqtl);
if (ct == 1)
    {
    int i = eqtlTissueIndex(eqtl);
    if (i<0)
        errAbort("GTEx eQTL %s/%s track tissue index is negative", eqtl->name, eqtl->target);
    return eqtl->expNames[i];
    }
struct dyString *ds = dyStringNew(0);
dyStringPrintf(ds, "%d tissues", ct);
return dyStringCannibalize(&ds);
}

/* Track methods */

static void gtexEqtlClusterLoadItems(struct track *track)
/* Load items in window and prune those that don't pass filter */
{
/* initialize track info */
struct gtexEqtlClusterTrack *extras;
AllocVar(extras);
track->extraUiData = extras;
char cartVar[64];

// UI settings
safef(cartVar, sizeof cartVar, "%s.%s", track->track, GTEX_EQTL_TISSUE_COLOR);
extras->doTissueColor = cartUsualBoolean(cart, cartVar, GTEX_EQTL_TISSUE_COLOR_DEFAULT);
boolean isFiltered = FALSE;

// filter by gene via SQL
safef(cartVar, sizeof cartVar, "%s.%s", track->track, GTEX_EQTL_GENE);
char *gene = cartNonemptyString(cart, cartVar);
char *where = NULL;
if (gene)
    {
    struct dyString *ds = dyStringNew(0);
    sqlDyStringPrintfFrag(ds, "%s = '%s'", GTEX_EQTL_GENE_FIELD, gene); 
    where = dyStringCannibalize(&ds);
    isFiltered = TRUE;
    }
bedLoadItemWhere(track, track->table, where, (ItemLoader)loadOne);

// more filtering
safef(cartVar, sizeof cartVar, "%s.%s", track->track, GTEX_EQTL_EFFECT);
extras->minEffect = fabs(cartUsualDouble(cart, cartVar, GTEX_EQTL_EFFECT_DEFAULT));
safef(cartVar, sizeof cartVar, "%s.%s", track->track, GTEX_EQTL_PROBABILITY);
extras->minProb = cartUsualDouble(cart, cartVar, GTEX_EQTL_PROBABILITY_DEFAULT);
boolean hasTissueFilter = filterTissuesFromCart(track, extras);
if (hasTissueFilter || extras->minEffect != 0.0 || extras->minProb != 0.0)
    {
    isFiltered = TRUE;
    filterItems(track, eqtlIncludeFilter, "include");
    }
if (isFiltered)
    labelTrackAsFiltered(track);
}

static char *gtexEqtlClusterItemName(struct track *track, void *item)
/* Left label is gene name */
{
struct gtexEqtlCluster *eqtl = (struct gtexEqtlCluster *)item;
return eqtl->target;
}

// blues
#define EQTL_COLOR_HIGH_NEGATIVE   MAKECOLOR_32(0x0, 0x0, 0xff)
#define EQTL_COLOR_LOW_NEGATIVE    MAKECOLOR_32(0xa0, 0xa0, 0xff)
// reds
#define EQTL_COLOR_HIGH_POSITIVE   MAKECOLOR_32(0xff, 0x0, 0x0)
#define EQTL_COLOR_LOW_POSITIVE    MAKECOLOR_32(0xff, 0xa0, 0xa0)
// gray
#define EQTL_COLOR_MIXED           MAKECOLOR_32(0x69, 0x69, 0x69)

static Color gtexEqtlClusterItemColor(struct track *track, void *item, struct hvGfx *hvg)
/* Color by highest effect in list (blue -, red +), grayed for lower effect. Gray if mixed effect*/
{
struct gtexEqtlCluster *eqtl = (struct gtexEqtlCluster *)item;
double maxEffect = 0.0;
double minEffect = 0.0;
int i;
for (i=0; i<eqtl->expCount; i++)
    {
    if (eqtlIsExcludedTissue(eqtl, i))
        continue;
    double effect = eqtl->expScores[i];
    if (effect > maxEffect)
        maxEffect = effect;
    else if (effect < minEffect)
        minEffect = effect;
    }
if (minEffect < 0.0 && maxEffect > 0.0)
    // mixed effect
    return EQTL_COLOR_MIXED;

double cutoff = 2.0;
if (minEffect < 0.0)
    {
    // down-regulation displayed as blue
    if (minEffect < 0.0 - cutoff)
        return EQTL_COLOR_HIGH_NEGATIVE;
    return EQTL_COLOR_LOW_NEGATIVE;
    }
// up-regulation displayed as red
if (maxEffect > cutoff)
    return EQTL_COLOR_HIGH_POSITIVE;
return EQTL_COLOR_LOW_POSITIVE;
}

/* Helper macros */

#define tissueColorPatchSpacer          (max(2, tl.nWidth/4))
#define tissueColorPatchWidth           (tl.nWidth * .8)

static int gtexEqtlClusterItemRightPixels(struct track *track, void *item)
/* Return number of pixels we need to the right of an item (for sources label). */
{
struct gtexEqtlCluster *eqtl = (struct gtexEqtlCluster *)item;
int ret = mgFontStringWidth(tl.font, eqtlSourcesLabel(eqtl));
if (eqtlTissueCount(eqtl) == 1)
    ret += tissueColorPatchWidth + tissueColorPatchSpacer;
return ret;
}

static void gtexEqtlClusterDrawItemAt(struct track *track, void *item, 
	struct hvGfx *hvg, int xOff, int y, 
	double scale, MgFont *font, Color color, enum trackVisibility vis)
/* Draw GTEx eQTL cluster with right label indicating source(s) */
{
bedDrawSimpleAt(track, item, hvg, xOff, y, scale, font, color, vis);
if (vis != tvFull && vis != tvPack)
    return;

/* Draw text to the right */
struct gtexEqtlCluster *eqtl = (struct gtexEqtlCluster *)item;
struct gtexEqtlClusterTrack *extras = (struct gtexEqtlClusterTrack *)track->extraUiData;
int x2 = round((double)((int)eqtl->chromEnd-winStart)*scale) + xOff;
int x = x2 + tl.mWidth/2;
char *label = eqtlSourcesLabel(eqtl);
int w = mgFontStringWidth(font, label);
hvGfxTextCentered(hvg, x, y, w, track->heightPer, MG_BLACK, font, label);
if (eqtlTissueCount(eqtl) == 1 && extras->doTissueColor)
    {
    // tissue color patch (box)
    x += w;
    int h = w = tissueColorPatchWidth;
    struct rgbColor tisColor = eqtlTissueColor(track, eqtl);
    int ix = hvGfxFindColorIx(hvg, tisColor.r, tisColor.g, tisColor.b);
    hvGfxBox(hvg, x + tissueColorPatchSpacer, y + (tl.fontHeight - h)/2 + tl.fontHeight/10, w, h, ix);
    }
}

static void gtexEqtlClusterMapItem(struct track *track, struct hvGfx *hvg, void *item, 
                                char *itemName, char *mapItemName, int start, int end, 
                                int x, int y, int width, int height)
/* Create a map box on item and label with list of tissues with colors and effect size */
{
char *title = itemName;
if (track->limitedVis != tvDense)
    {
    struct gtexEqtlCluster *eqtl = (struct gtexEqtlCluster *)item;
    struct dyString *ds = dyStringNew(0);
    dyStringPrintf(ds, "%s/%s: ", eqtl->name, eqtl->target);
    int i;
    for (i=0; i<eqtl->expCount; i++)
        {
        if (eqtlIsExcludedTissue(eqtl, i))
            continue;
        double effect= eqtl->expScores[i];
        dyStringPrintf(ds, "%s(%s%0.2f)%s", eqtl->expNames[i], effect < 0 ? "" : "+", effect, 
                        i < eqtl->expCount - 1 ? ", " : "");
        }
    title = dyStringCannibalize(&ds);
    }
int w = width + gtexEqtlClusterItemRightPixels(track, item);
genericMapItem(track, hvg, item, title, itemName, start, end, x, y, w, height);
}

void gtexEqtlClusterMethods(struct track *track)
/* BED5+5 with target, expNames,expScores, expProbs */
{
track->loadItems = gtexEqtlClusterLoadItems;
track->itemName  = gtexEqtlClusterItemName;
track->itemColor = gtexEqtlClusterItemColor;
track->itemRightPixels = gtexEqtlClusterItemRightPixels;
track->drawItemAt = gtexEqtlClusterDrawItemAt;
track->mapItem = gtexEqtlClusterMapItem;
}
