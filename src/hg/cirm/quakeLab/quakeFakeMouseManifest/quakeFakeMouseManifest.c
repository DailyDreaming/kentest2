/* quakeFakeMouseManifest - Create a fake manifest file for pilot mouse data. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "models.h"

char *chipsFile = "7NOV2014.CIRM.chips.csv";
char *sampleFile = "7NOV2014.CIRM.samples.csv";
char *chipDirPatterns = "*_*_*_*";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "quakeFakeMouseManifest - Create a fake manifest file for pilot mouse data.\n"
  "from quake lab\n"
  "usage:\n"
  "   quakeFakeMouseManifest inDir manifest.tab err.txt chip.ra\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

/* Command line validation table. */
static struct optionSpec options[] = {
   {NULL, 0},
};

struct hash *barcodeToChipHash(struct quakeChip *chipList)
/* Return hash full of chips keyed by barcodes. */
{
struct quakeChip *chip;
struct hash *hash = hashNew(0);
for (chip = chipList; chip != NULL; chip = chip->next)
    hashAddUnique(hash, chip->captureArrayBarcode, chip);
return hash;
}

struct quakeSheet *readSheet(char *fileName, FILE *err)
/* Read in comma separated sheet file */
{
struct lineFile *lf = lineFileMayOpen(fileName, TRUE);
if (lf == NULL)
    {
    fprintf(err, "Can't open %s\n", fileName);
    return NULL;
    }
char *line;
lineFileNeedNext(lf, &line, NULL);
struct quakeSheet *sheetList = NULL;
if (!startsWith("FCID,Lane,SampleID,", line))
    errAbort("%s not in sheet format that we understand", fileName);
char *row[QUAKESHEET_NUM_COLS];
while (lineFileNextCharRow(lf, ',', row, ArraySize(row)))
     {
     struct quakeSheet *sheet = quakeSheetLoad(row);
     slAddTail(&sheetList, sheet);
     }
lineFileClose(&lf);
return sheetList;
}

char *sampleMetaName(struct quakeSample *sample)
/* Return name that will be unique for this sample and not too cryptic */
{
static char buf[128];
safef(buf, sizeof(buf), "%s_%u_%s", sample->fluidPlateAcc, sample->unknown1, sample->sample);
return buf;
}

void quakeFakeMouseManifest(char *inDir, char *outFile, char *errFile, char *metaFile)
/* quakeFakeMouseManifest - Create a fake manifest file for pilot mouse data. */
{
int inDirSize = strlen(inDir) + 1;  // We'll add a / at end
/* Load up metadata about what loaded on each microfluidics chip */
char chipsPath[PATH_LEN];
safef(chipsPath, sizeof(chipsPath), "%s/%s", inDir, chipsFile);
struct quakeChip *chipList = quakeChipLoadAllByChar(chipsPath, ',');
verbose(1, "Loaded %d from %s\n", slCount(chipList), chipsPath);
struct hash *chipHash = barcodeToChipHash(chipList);
verbose(1, "chipHash has %d elements\n", chipHash->elCount);

/* Load up metadata about each productive cell sample. */
char samplePath[PATH_LEN];
safef(samplePath, sizeof(samplePath), "%s/%s", inDir, sampleFile);
struct quakeSample *sampleList = quakeSampleLoadAllByChar(samplePath, '\t');
verbose(1, "Loaded %d from %s\n", slCount(sampleList), samplePath);

/* Open manifest file and write out header */
FILE *f = mustOpen(outFile, "w");
fprintf(f, "#file\tformat\toutput\tmeta\tenriched_in\tucsc_db\tpaired_end\tfile_part\tlab_quake_fluidics_chip\tlab_quake_fluidics_cell\n");

int fileCount = 0;
FILE *err = mustOpen(errFile, "w");

struct hash *fileHash = hashNew(0);

/* Loop through samples */
struct quakeSample *sample;
for (sample = sampleList; sample != NULL; sample = sample->next)
    {
    /* Find chip associated with this sample. */
    struct quakeChip *chip = hashMustFindVal(chipHash, sample->captureArrayBarcode);

    /* Get local metadata file, which will have one item for each set of paired fastqs */
    char subdir[PATH_LEN];
    safef(subdir, sizeof(subdir), "%s/%s/Sample_%s", inDir, sample->subdir, sample->sample);
    char sheetPath[PATH_LEN];
    safef(sheetPath, sizeof(sheetPath), "%s/%s", subdir, "SampleSheet.csv");
    verbose(2, "%s\n", sheetPath);
    struct quakeSheet *sheet, *sheetList = readSheet(sheetPath, err);
    int sheetCount = slCount(sheetList);
    if (sheetCount == 0)
        continue;
    if (sheetCount != 1 && sheetCount != 2)
         errAbort("%d items in %s, expecting 1 or 2\n", sheetCount, sheetPath);
 
    for (sheet = sheetList; sheet != NULL; sheet = sheet->next)
        {
	int part;
	boolean gotAllParts = FALSE;
	for (part = 1; part <= 10; ++part)
	    {
	    int mate;
	    for (mate=1; mate <= 2; ++mate)
		{
		char fastqPath[PATH_LEN];
		safef(fastqPath, sizeof(fastqPath), "%s/%s_%s_L%03d_R%d_%03d.fastq.gz"
		    , subdir, sample->sample, sheet->seqIndex, sheet->lane, mate, part);
		if (!fileExists(fastqPath))
		    {
		    if (mate != 1)
			{
		        fprintf(err, "Got one read pair but not other at %s\n", fastqPath);
			continue;
			}
		    if (part <= 1)
		        errAbort("Couldn't find %s", fastqPath);
		    gotAllParts = TRUE;
		    break;
		    }
		char *fastqFile = fastqPath + inDirSize;
		if (!hashLookup(fileHash, fastqFile))
		    {
		    hashAdd(fileHash, fastqFile, NULL);
		    ++fileCount;
		    fprintf(f, "%s\tfastq\treads\t%s\texon\tmm10\t%d\t%d\t%s\t%s\n", 
			fastqFile, sampleMetaName(sample), mate, part, 
			chip->captureArrayBarcode, sample->cellPos);
		    }
		}
	    if (gotAllParts)
	        break;
	    }
	}
    }
verbose(1, "Got %d files\n", fileCount);
carefulClose(&f);
carefulClose(&err);

FILE *meta = mustOpen(metaFile, "w");
struct quakeChip *chip;
char *root = "quakeMouseLung";
fprintf(meta, "data_set_id %s\n", root);
fprintf(meta, "title Single cell RNA sequencing of the embryonic mouse lung\n");
fprintf(meta, "lab quake\n");
fprintf(meta, "pmid 24739965\n");
fprintf(meta, "assay long-RNA-seq\n");
fprintf(meta, "\n");
for (chip = chipList; chip != NULL; chip = chip->next)
    {
    char *indent = "    ";
    fprintf(meta, "%smeta chip%s\n", indent, chip->captureArrayBarcode);
    fprintf(meta, "%ssubmitter %s\n", indent, chip->user);
    fprintf(meta, "%slab %s\n", indent, chip->lab);
    fprintf(meta, "%sbiosample_date %s\n", indent, chip->date);
    fprintf(meta, "%slab_quake_sample_type %s\n", indent, chip->sampleType);
    fprintf(meta, "%sbody_part %s\n", indent, chip->tissue);
    fprintf(meta, "%sspecies %s\n", indent, chip->species);
    fprintf(meta, "%sstrain %s\n", indent, chip->strain);
    fprintf(meta, "%slab_quake_age %s\n", indent, chip->age);
    fprintf(meta, "%slab_quake_target_cell %s\n", indent, chip->targetCell);
    fprintf(meta, "%slab_quake_sort_method %s\n", indent, chip->sortMethod);
    fprintf(meta, "%slab_quake_markers %s\n", indent, chip->markers);
    fprintf(meta, "\n");
    for (sample = sampleList; sample != NULL; sample = sample->next)
        {
	if (sameString(sample->captureArrayBarcode, chip->captureArrayBarcode))
	    {
	    char *indent = "       ";
	    fprintf(meta, "%smeta %s\n", indent, sampleMetaName(sample));
	    fprintf(meta, "%sseq_sample %s\n", indent, sampleMetaName(sample));
	    fprintf(meta, "%slab_quake_sample %s\n", indent, sample->sample);
	    fprintf(meta, "%slab_quake_cell_pos %s\n", indent, sample->cellPos);
	    fprintf(meta, "%ssequencer %s\n", indent, sample->sequencer);
	    fprintf(meta, "%slab_quake_seq_format %s\n", indent, sample->seqFormat);
	    fprintf(meta, "\n");
	    }
	}
    }
carefulClose(&meta);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 5)
    usage();
quakeFakeMouseManifest(argv[1], argv[2], argv[3], argv[4]);
return 0;
}
