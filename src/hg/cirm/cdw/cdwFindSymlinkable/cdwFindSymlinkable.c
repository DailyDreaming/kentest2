/* cdwFindSymlinkable - Find files under wrangle dir that could be symlinked to the file under cdw/ to save space. */
#include <utime.h>
#include "common.h"
#include "linefile.h"
#include "filePath.h"
#include "portable.h"
#include "md5.h"
#include "hash.h"
#include "options.h"
#include "cdw.h"
#include "cdwLib.h"

boolean fix = FALSE;
char *idList = NULL;
char *submitDirFilter = NULL;
boolean inodeReport = FALSE;
boolean fixCdwMtimes = FALSE;

struct hash *hash = NULL;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "cdwFindSymlinkable - Find files under wrangle dir that could be symlinked to the file under cdw/ to save space.\n"
  "usage:\n"
  "   cdwFindSymlinkable find \n"
  "       does a dry run looking for symlinkable files \n"
  "   cdwFindSymlinkable fix \n"
  "       replaces files under wrangle or submit with symlinks to files under /data/cirm/cdw/.\n"
  "options:\n"
  "   -idList=290,291 limits operation to a comma-separated list of submitId's.\n"
  "   -submitDir=/my/submitDir limits operation to submits belonging to submitDir.\n"
  "\n"
  "   -inodeReport causes it to track and report on inodes used by multiple submitFileName paths.\n"
  "     Creates inodeReport.log in the current directory. Handy for debugging symlinking issues.\n"
  "     Only use with \"find\" comand.\n"
  "\n"
  "   -fixCdwMtimes causes it to set mtime on cdwFile.cdwFileName using cdwFile.updateTime.\n"
  "     Current implementation requires that this be run as root which makes it fast and preserves original ownership.\n"
  );
}

/* Command line validation table. */
static struct optionSpec options[] = {
   {"idList", OPTION_STRING},
   {"submitDir", OPTION_STRING},
   {"inodeReport", OPTION_BOOLEAN},
   {"fixCdwMtimes", OPTION_BOOLEAN},
   {NULL, 0},
};

void showInodeReport()
/* print out the inode report, find inode collisions */
{
// want to loop through all members of the hash, skipping things that have single count.
struct hashEl *el, *list = hashElListHash(hash);
slSort(&list, hashElCmp);
char *inodeName = "";
FILE *f = mustOpen("inodeReport.log", "w");
verbose(1, "inodes list length=%d\n", slCount(list));
int uniqueInodes = 0;
for (el = list; el != NULL; el = el->next)
    {
    if (sameString(inodeName, el->name))
	continue;    // skip until unique name found
    ++uniqueInodes;
    inodeName = el->name;
    struct hashEl *hEl;
    struct slRef *pathList = NULL, *r = NULL;
    char *lastVal = "";
    for (hEl = hashLookup(hash, inodeName); hEl; hEl = hashLookupNext(hEl))
	{
	// because if a file is edited and resubmitted, it will get a new cdwFile record,
        // modifed and resubmitted files (like some html files) will appear multiple times,
        // filter out the false duplicates
	if (!sameString(lastVal, hEl->val))
	    refAdd(&pathList, hEl->val);
	lastVal = hEl->val;
	}
    // only want inodes that are shared by 2 or more files.
    if (slCount(pathList) >= 2)  
	{
	if (slCount(pathList) > 3)
	    {
	    verbose(1, "inode %s with %d paths found.\n", inodeName, slCount(pathList));
	    }
      	
	fprintf(f, "%s", inodeName);
	for (r=pathList;r;r=r->next)
	    {
	    fprintf(f, " %s",(char *)r->val);
	    }
	fprintf(f, "\n");
	}
    slFreeList(&pathList);
    }
hashElFreeList(&list);
hashFree(&hash);
carefulClose(&f);
verbose(1, "unique inodes count=%d\n", uniqueInodes);
}

int fixMTimeForCdwFile(char *path, long long updateTime, boolean fix)
/* Fix the modified time of the cdw path to match the updateTime */
{
int result = 0;
struct stat buf;
if (stat(path, &buf) != 0)
    errnoAbort("stat failure on %s", path);
if (updateTime == buf.st_mtime)
    {
    verbose(1, "mtime already correct on %s\n", path);
    }
else
    {
    if (fix)
	{
	struct utimbuf puttime;
	puttime.modtime = updateTime;
	puttime.actime  = buf.st_atime;
	if (utime(path, &puttime) != 0)
	    errnoAbort("utime failure on %s", path);
	verbose(1, "mtime fixed on %s to %lld\n", path, updateTime);
	}
    else
	{
	verbose(1, "fix command not specified, skipping mtime fix on %s from %lld to %lld\n", 
	    path,  (long long)buf.st_mtime, updateTime);
	}
    result = 1;
    }
return result;
}

char *findSubmitSymlinkExtra(char *submitFileName, char *submitDir, char *oldPath, char **pPath)
/* Find the last symlink in the chain from submitDir/submitFileName.
 * This is useful for when target of symlink in cdw/ gets renamed 
 * (e.g. license plate after passes validation), or removed (e.g. cdwReallyRemove* commands). */
{
char *lastPath = NULL;
char *path = NULL;
int symlinkLevels = 0;

int result = findSubmitSymlinkExt(submitFileName, submitDir, &path, &lastPath, &symlinkLevels);
if (result == -1)  // path does not exist
    {
    warn("path=[%s] does not exist following submitDir/submitFileName through symlinks.", path);
    return NULL;
    }
if (symlinkLevels < 1)
    {
    warn("Too few symlinks followed: %d symlinks. Where is the symlink created by cdwSubmit?", symlinkLevels);
    return NULL;
    }
*pPath = path;   // WE RETURN THIS EXTRA INFO
return lastPath;
}

void cdwFindSymlinkable()
/* Find files that have not been symlinked to cdw/ yet to save space. */
{
struct sqlConnection *conn = cdwConnectReadWrite();
char query[512];

sqlSafef(query, sizeof(query), "select id from cdwSubmit");

// optional filtering
if (idList && submitDirFilter)
    errAbort("You can use -idList or -submitDir, but not both at the same time.");

if (idList)
    {
    sqlSafefAppend(query, sizeof(query), " where id in (");
    unsigned *idArray = NULL; 
    int idCount = 0;
    sqlUnsignedDynamicArray(idList, &idArray, &idCount);
    int i;
    for (i=0; i < idCount; ++i)
	{
	if (i > 0)
	    sqlSafefAppend(query, sizeof(query), ",");
	sqlSafefAppend(query, sizeof(query), "%d", idArray[i]);
	}
    sqlSafefAppend(query, sizeof(query), ")");
    freeMem(idArray);
    }

if (submitDirFilter)
    {
    char query2[512];
    sqlSafef(query2, sizeof(query2), "select id from cdwSubmitDir where url='%s'", submitDirFilter);
    int submitDirId = sqlQuickNum(conn, query2);
    if (submitDirId <= 0)
	errAbort("%s not found", submitDirFilter);
    sqlSafefAppend(query, sizeof(query), " where submitDirId = %d", submitDirId);
    }

struct slInt *submitIdList = sqlQuickNumList(conn, query);
verbose(1, "%d submits\n", slCount(submitIdList));

if (inodeReport)
    {
    hash = newHash(12);
    }


char *newPath = NULL;
int filesMTimed = 0;
int filesSymlinked = 0;
off_t fileSpaceSaved = 0;
struct stat sb;
struct slInt *sel;
for (sel = submitIdList; sel != NULL; sel = sel->next)
    {
    verbose(1, "-----------------------------------------\n"); 
    verbose(1, "%d\n", sel->val); 

    struct cdwSubmit *es = cdwSubmitFromId(conn, sel->val);
    verbose(1, "url=%s submitDirId=%d manifestFileId=%d metaFileId=%d \n", 
	es->url, es->submitDirId, es->manifestFileId, es->metaFileId);
    
    struct cdwSubmitDir *ed = cdwSubmitDirFromId(conn, es->submitDirId);
    if (!ed)
	{
	verbose(1, "submitDir record missing id=%d\n", es->submitDirId);
	continue;
	}
    verbose(1, "submitDir url=%s\n", ed->url);

    verbose(1, "files:\n");
    sqlSafef(query, sizeof(query), "select id from cdwFile where submitId=%d", sel->val);
    struct slInt *fileIdList = sqlQuickNumList(conn, query);
    verbose(1, "%d files in submission\n", slCount(fileIdList));
    struct slInt *el;
    for (el = fileIdList; el != NULL; el = el->next)
	{
	verbose(1, "fileId %d\n", el->val); 
	struct cdwFile *ef = cdwFileFromId(conn, el->val);
	if (!ef->cdwFileName)
	    {
	    verbose(1, "ef->cdwFileName is null for file id=%d\n", el->val);
	    continue;
	    }
	char *path = cdwPathForFileId(conn, el->val);
	verbose(1, "id=%u, submitFileName=%s path=%s cdwFileName=%s\n", 
	    ef->id, ef->submitFileName, path, ef->cdwFileName);

	if (!fileExists(path))
	    {
	    verbose(1, "cdwFile path %s does not exist.\n", path);
	    continue;
	    }

	if (fixCdwMtimes)
	    {
	    filesMTimed += fixMTimeForCdwFile(path, ef->updateTime, fix);
	    continue;
	    }

	// skip if meta or manifest which do not get symlink
	if ((el->val == es->manifestFileId) || (el->val == es->metaFileId))
	    continue;

	// apply submitFileName to submitDir, giving an absolute path
	freeMem(newPath);
        newPath = mustExpandRelativePath(ed->url, ef->submitFileName);

	if (!fileExists(newPath))
	    {
	    verbose(1, "Original submit path %s does not exist or is a broken symlink.\n", newPath);
	    continue;
	    }

	if (inodeReport)
	    {
	    if (stat(newPath, &sb) == -1) 
		errnoAbort("stat failure on %s", newPath);
	    unsigned long long x = sb.st_ino;
	    char key[256];
	    char *val = cloneString(newPath);
	    safef(key, sizeof key, "%llu", x);
	    verbose(4, "newPath=%s on inode=%s\n", newPath, key);
	    hashAdd(hash, key, val);
	    }

	// compare file times
	long long fmtp = fileModTime(path);
	long long fmtnp = fileModTime(newPath);
	if (!(fmtp == ef->updateTime && fmtnp == ef->updateTime))
	    verbose(1, "File Modified Times do not match: updateTime=%lld cdwFile=%lld submitFile=%lld\n", 
		ef->updateTime, fmtp, fmtnp);

        // test for already-symlinked case
	char *lastPath = NULL;
	char *actualTarget = NULL;
	lastPath = findSubmitSymlinkExtra(ef->submitFileName, ed->url, path, &actualTarget);
	if (lastPath)
	    {
	    if (!sameString(actualTarget, path))
		{
		// check if file was edited and newer submission of it exists.
		char query4[256];
		sqlSafef(query4, sizeof query4, "select count(*) from cdwFile"
		    " where submitDirId=%d"
		    " and submitFileName='%s'"
		    " and cdwFileName='%s'"
		    " and id > %d"
		    , ef->submitDirId, ef->submitFileName, ef->cdwFileName, ef->submitId);
		if (sqlQuickNum(conn, query4) > 0) 
		    {
		    verbose(1, "skipping %s since it was modified and re-submitted.\n", newPath);
		    continue;
		    }
		else
		    {
		    warn("Found symlinks point to %s, expecting to find symlink pointing to old path %s", actualTarget, path);
		    }

		}
	    else
		{
		verbose(1, "Already symlinked to file under cdw/ lastPath=%s path=%s\n", lastPath, path);
		freeMem(lastPath);
		continue;
		}
	    }

	// Check that the target is not already a symlink.
        // If so, report it and skip it. This is an unexpected case.
        // Cannot be certain that it does not through some chain of links point back to the original file.
	if (lstat(path, &sb) == -1) 
	    errnoAbort("lstat failure on %s", path);
	if ((sb.st_mode & S_IFMT) != S_IFREG) // regular file?
	    {
	    char *ftype = "";
	    if ((sb.st_mode & S_IFMT) == S_IFLNK) ftype = "symlink";
	    if ((sb.st_mode & S_IFMT) == S_IFDIR) ftype = "directory";
	    if ((sb.st_mode & S_IFMT) == S_IFDIR) ftype = "special file";  // some other special file type
	    verbose(1, "skipping since target %s is not a regular file, but a %s.\n", path, ftype);
	    continue;
	    }


	// check if old failed validation but a newer valid one exists.
	if (ef->errorMessage && !sameString(ef->errorMessage,""))
	    {
	    char query3[256];
	    sqlSafef(query3, sizeof query3, "select count(*) from cdwFile"
		" where submitDirId=%d"
		" and submitFileName='%s'"
		" and (cdwFile.errorMessage = '' or cdwFile.errorMessage is null)"
		, ef->submitDirId, ef->submitFileName);
	    if (sqlQuickNum(conn, query3) > 0) 
		{
		verbose(1, "skipping %s since it failed validation but a newer valid one was submitted.\n", newPath);
	        continue;
		}
	    }


	// check if file sizes match since it is quick
	off_t subFSize = fileSize(newPath);
	off_t cdwFSize = fileSize(path);
	if (subFSize != cdwFSize)
	    {
	    verbose(1, "skipping since file sizes do not match between %s (%llu) and %s (%llu)\n", 
		newPath, (long long unsigned)subFSize, path, (long long unsigned)cdwFSize);
	    continue;
	    }

	if (!inodeReport)
	    {
	    verbose(1, "calculating md5 for %s\n", newPath);
	    char *md5 = md5HexForFile(newPath);
	    if (!sameString(md5, ef->md5))
		{
		verbose(1, "skipping since md5 does not match between %s and %s\n", newPath, path);
		continue;
		}
	    }

	// save space by finding the last real file in symlinks chain
	// and replace IT with a symlink to the submitted file under cdwFileName. 

	if (fix)
	    replaceOriginalWithSymlink(ef->submitFileName, ed->url, path);
	else
	    verbose(1, "fix command not specified,  skipping symlink from %s to %s\n", newPath, path);
	    

	filesSymlinked++;
	fileSpaceSaved += fileSize(newPath);

	}
    }

verbose(1, "\n\n%d files %ssymlinked.\n", filesSymlinked, fix ? "":"would have been ");
verbose(1, "\n%llu space %ssaved.\n", (unsigned long long) fileSpaceSaved, fix ? "":"would have been ");
if (fixCdwMtimes)
    verbose(1, "\n\n%d cdw file mtimes %sfixed.\n", filesMTimed, fix ? "":"would have been ");

if (!fix)
    verbose(1, "\nfix command was not specified. Dry run only.\n\n");


if (inodeReport)
    {
    showInodeReport();
    verbose(1, "\nmd5 was skipped for speed for inodeReport.\n\n");
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 2)
    usage();
char *command = argv[1];
verbose(1, "command=%s\n", command);
if (sameString(command, "fix"))
    fix = TRUE;
else if (sameString(command, "find"))
    fix = FALSE;
else
   usage(); 

idList = optionVal("idList", NULL);
submitDirFilter = optionVal("submitDir", NULL);
inodeReport = optionExists("inodeReport");
fixCdwMtimes = optionExists("fixCdwMtimes");

if (inodeReport & fix)
    {
    warn("cannot use -inodeReport with fix, only allowed with find.");
    usage();
    }

cdwFindSymlinkable();

return 0;
}
