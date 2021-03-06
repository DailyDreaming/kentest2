/* cdwFakeManifestFromSubmit - Create a fake submission based on a real one that is in the warehouse. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "portable.h"
#include "cdw.h"
#include "cdwLib.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "cdwFakeManifestFromSubmit - Create a fake submission based on a real one that is in the warehouse\n"
  "usage:\n"
  "   cdwFakeManifestFromSubmit submitId outDir\n"
  "This will create an out directory populated with manifest.txt and validated.txt\n"
  "and with symbolic links back to cdw files.\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

/* Command line validation table. */
static struct optionSpec options[] = {
   {NULL, 0},
};

void fprint2(FILE *f1, FILE *f2, char *format, ...)
/* Print out to two files */
#if defined(__GNUC__)
__attribute__((format(printf, 3, 4)))
#endif
;

void fprint2(FILE *f1, FILE *f2, char *format, ...)
/* Print out to two files */
{
va_list args;
va_start(args, format);
vfprintf(f1, format, args);
va_end(args);
va_start(args, format);
vfprintf(f2, format, args);
va_end(args);
}


void printSharedHeader(FILE *f)
/* Print out part of header shared by manifest and validated.txt */
{
fprintf(f, "#file_name\tformat\toutput_type\texperiment\tenriched_in\tucsc_db\treplicate\tpart\tpaired_end");
}

void cdwFakeManifestFromSubmit(char *submitIdString, char *outDir)
/* cdwFakeManifestFromSubmit - Create a fake submission based on a real one that is in the warehouse. */
{
struct sqlConnection *conn = cdwConnect();
char query[512];
sqlSafef(query, sizeof(query), "select * from cdwSubmit where id=%s", submitIdString);
struct cdwSubmit *submit = cdwSubmitLoadByQuery(conn, query);
if (submit == NULL)
    errAbort("Can't find submission %s", submitIdString);

uglyf("%d files in query\n", submit->newFiles);
sqlSafef(query, sizeof(query), "select * from cdwFile where submitId=%s", submitIdString);
struct cdwFile *ef, *efList = cdwFileLoadByQuery(conn, query);

FILE *maniF = NULL, *valiF = NULL;
for (ef = efList; ef != NULL; ef = ef->next)
    {
    struct cdwValidFile *vf = cdwValidFileFromFileId(conn, ef->id);
    if (vf != NULL)
        {
	/* First time through create out directory and open output files. */
	if (maniF == NULL)
	    {
	    char *fakeVersion = "##validateManifest version 1.7";
	    makeDirsOnPath(outDir);
	    setCurrentDir(outDir);
	    maniF = mustOpen("manifest.txt", "w");
	    printSharedHeader(maniF);
	    fprintf(maniF, "\n");
	    fprintf(maniF, "%s\n", fakeVersion);
	    valiF = mustOpen("validated.txt", "w");
	    printSharedHeader(valiF);
	    fprintf(valiF, "\tmd5_sum\tsize\tmodified\tvalid_key\n");
	    fprintf(valiF, "%s\n", fakeVersion);
	    }

	/* Figure out file names */
	char cdwPath[PATH_LEN], rootName[FILENAME_LEN], ext[FILEEXT_LEN];
	safef(cdwPath, sizeof(cdwPath), "%s%s", cdwRootDir, ef->cdwFileName);
	splitPath(ef->cdwFileName, NULL, rootName, ext);
	char localPath[PATH_LEN];
	safef(localPath, sizeof(localPath), "%s%s", rootName, ext);

	/* Create sym-linked file and write to manifest */
	symlink(cdwPath, localPath);
	fprint2(maniF, valiF, "%s", localPath);

	/* Write other columns shared between manifest and validated */
	fprint2(maniF, valiF, "\t%s", vf->format);
	fprint2(maniF, valiF, "\t%s", naForEmpty(vf->outputType));
	fprint2(maniF, valiF, "\t%s", naForEmpty(vf->experiment));
	fprint2(maniF, valiF, "\t%s", naForEmpty(vf->enrichedIn));
	fprint2(maniF, valiF, "\t%s", naForEmpty(vf->ucscDb));
	fprint2(maniF, valiF, "\t%s", naForEmpty(vf->replicate));
	fprint2(maniF, valiF, "\t%s", naForEmpty(vf->part));
	fprint2(maniF, valiF, "\t%s", naForEmpty(vf->pairedEnd));
	fprintf(maniF, "\n");

	/* Print out remaining fields in validated.txt */
	fprintf(valiF, "\t%s\t%lld\t%lld\n", ef->md5, ef->size, ef->updateTime);
	}
    }
carefulClose(&maniF);
carefulClose(&valiF);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
cdwFakeManifestFromSubmit(argv[1], argv[2]);
return 0;
}
