#!/usr/bin/env perl

use strict;
use warnings;
use File::Basename;

my $argc = scalar(@ARGV);
if ($argc != 1) {
  printf STDERR "mkSymLinks.pl [two column name list]\n";
  printf STDERR "e.g.: mkSymLinks.pl vgp.primary.assemblies.tsv\n";
  printf STDERR "the name list is found in \$HOME/kent/src/hg/makeDb/doc/asmHubs/\n";
  printf STDERR "\nthe two columns are 1: asmId (accessionId_assemblyName)\n";
  printf STDERR "column 2: common name for species, columns separated by tab\n";
  printf STDERR "the result will create symLinks from the build direcory\n";
  printf STDERR "into the appropriate /asmHubs/GC[AF]/.../ release directory\n";
  printf STDERR "hierarchy.  The output to stderr is merely a progress report.\n";
  exit 255;
}

my $home = $ENV{'HOME'};
my $toolsDir = "$home/kent/src/hg/makeDb/doc/asmHubs";

my $inputList = shift;
my $orderList = $inputList;
if ( ! -s "$orderList" ) {
  $orderList = $toolsDir/$inputList;
}

my %commonName;	# key is asmId, value is common name
my @orderList;	# asmId of the assemblies in order from the *.list files
# the order to read the different .list files:
my $assemblyCount = 0;

open (FH, "<${orderList}") or die "can not read ${orderList}";
while (my $line = <FH>) {
  next if ($line =~ m/^#/);
  chomp $line;
  my ($asmId, $commonName) = split('\t', $line);
  if (defined($commonName{$asmId})) {
    printf STDERR "ERROR: duplicate asmId: '%s'\n", $asmId;
    printf STDERR "previous name: '%s'\n", $commonName{$asmId};
    printf STDERR "duplicate name: '%s'\n", $commonName;
    exit 255;
  }
  $commonName{$asmId} = $commonName;
  push @orderList, $asmId;
  ++$assemblyCount;
}
close (FH);

my $destDir = "/hive/data/genomes/asmHubs";

my $buildDone = 0;
my $orderIndex = 0;
foreach my $asmId (@orderList) {
  ++$orderIndex;
  if ($asmId !~ m/^GC/) {
    printf STDERR "# not an assembly hub: %s\n", $asmId;
    next;
  }
  my ($gcPrefix, $accession, undef) = split('_', $asmId);
  my $accessionId = sprintf("%s_%s", $gcPrefix, $accession);
  my $accessionDir = substr($asmId, 0 ,3);
  $accessionDir .= "/" . substr($asmId, 4 ,3);
  $accessionDir .= "/" . substr($asmId, 7 ,3);
  $accessionDir .= "/" . substr($asmId, 10 ,3);
  $destDir = "/hive/data/genomes/asmHubs/$accessionDir/$accessionId";
  my $buildDir = "/hive/data/genomes/asmHubs/refseqBuild/$accessionDir/$asmId";
  if ($gcPrefix eq "GCA") {
     $buildDir = "/hive/data/genomes/asmHubs/genbankBuild/$accessionDir/$asmId";
  }
  my $trackDb = "$buildDir/$asmId.trackDb.txt";
  if ( ! -s "${trackDb}" ) {
    printf STDERR "# %03d not built yet: %s\n", $orderIndex, $asmId;
    printf STDERR "# missing tdb: '%s'\n", $trackDb;
    next;
  }
  ++$buildDone;
  printf STDERR "# %03d symlinks %s %s\n", $buildDone, $accessionId, $asmId;
#  printf STDERR "%s\n", $buildDir;
#  printf STDERR "%s\n", $destDir;
  if ( ! -d "${destDir}" ) {
    `mkdir -p "${destDir}"`;
  }
  `rm -f "${destDir}/bbi"`;
  `rm -f "${destDir}/genes"`;
  `rm -f "${destDir}/ixIxx"`;
  `rm -f "${destDir}/genesGtf"`;
  `rm -f "${destDir}/liftOver"`;
  `rm -fr "${destDir}/html"`;
  `mkdir -p "${destDir}/html"`;
  `rm -f "${destDir}/${accessionId}.2bit"`;
  `rm -f "${destDir}/${accessionId}.untrans.gfidx"`;
  `rm -f "${destDir}/${accessionId}.trans.gfidx"`;
  `rm -f "${destDir}/${accessionId}.agp.gz"`;
  `rm -f "${destDir}/${accessionId}.chrom.sizes"`;
  `rm -f "${destDir}/${accessionId}.chrom.sizes.txt"`;
  `rm -f "${destDir}/${accessionId}.chromAlias.txt"`;
  `rm -f "${destDir}/${accessionId}_assembly_report.txt"`;
  `rm -f "${destDir}/${accessionId}.repeatMasker.out.gz"`;
  `rm -f "${destDir}/${accessionId}.userTrackDb.txt"`;
  `rm -f "${destDir}/trackDb.txt"`;
  `rm -f "${destDir}/genomes.txt"`;
  `rm -f "${destDir}/download.genomes.txt"`;
  `rm -f "${destDir}/hub.txt"`;
  `rm -f "${destDir}/download.hub.txt"`;
  `rm -f "${destDir}/groups.txt"`;
  `ln -s "${buildDir}/bbi" "${destDir}/bbi"` if (-d "${buildDir}/bbi");
  `ln -s "${buildDir}/genes" "${destDir}/genes"` if (-d "${buildDir}/genes");
  `ln -s "${buildDir}/ixIxx" "${destDir}/ixIxx"` if (-d "${buildDir}/ixIxx");
  `ln -s "${buildDir}/genesGtf" "${destDir}/genesGtf"` if (-d "${buildDir}/genesGtf");
  `ln -s "${buildDir}/liftOver" "${destDir}/liftOver"` if (-d "${buildDir}/liftOver");
  `ln -s ${buildDir}/html/*.html "${destDir}/html/"` if (-d "${buildDir}/html");
   my $jpgFiles =`ls ${buildDir}/html/*.jpg 2> /dev/null | wc -l`;
   chomp $jpgFiles;
   if ($jpgFiles > 0) {
    `rm -f ${destDir}/html/*.jpg`;
    `ln -s ${buildDir}/html/*.jpg "${destDir}/html/"`;
   }
#  `ln -s ${buildDir}/html/*.png "${destDir}/genomes/${asmId}/html/"`;
  `ln -s "${buildDir}/${asmId}.2bit" "${destDir}/${accessionId}.2bit"` if (-s "${buildDir}/${asmId}.2bit");
   if (-s "${buildDir}/${accessionId}.untrans.gfidx") {
      if (-s "${buildDir}/${accessionId}.trans.gfidx") {
        `rm -f "${buildDir}/${asmId}.untrans.gfidx"`;
        `rm -f "${buildDir}/${asmId}.trans.gfidx"`;
        `ln -s "${buildDir}/${accessionId}.untrans.gfidx" "${destDir}/${accessionId}.untrans.gfidx"`;
        `ln -s "${buildDir}/${accessionId}.trans.gfidx" "${destDir}/${accessionId}.trans.gfidx"`;
      }
   }
  `ln -s "${buildDir}/${asmId}.agp.gz" "${destDir}/${accessionId}.agp.gz"` if (-s "${buildDir}/${asmId}.agp.gz");
  `ln -s "${buildDir}/${asmId}.chrom.sizes" "${destDir}/${accessionId}.chrom.sizes.txt"` if (-s "${buildDir}/${asmId}.chrom.sizes");
  `ln -s "${buildDir}/${asmId}.chromAlias.txt" "${destDir}/${accessionId}.chromAlias.txt"` if (-s "${buildDir}/${asmId}.chromAlias.txt");
  `ln -s "${buildDir}/${asmId}.repeatMasker.out.gz" "${destDir}/${accessionId}.repeatMasker.out.gz"` if (-s "${buildDir}/${asmId}.repeatMasker.out.gz");
  `ln -s "${buildDir}/download/${asmId}_assembly_report.txt" "${destDir}/${accessionId}_assembly_report.txt"` if (-s "${buildDir}/download/${asmId}_assembly_report.txt");
  # trackDb.txt still needed for use by top-level genomes.txt file
  `ln -s "${buildDir}/${asmId}.trackDb.txt" "${destDir}/trackDb.txt"` if (-s "${buildDir}/${asmId}.trackDb.txt");
  # genomes.txt obsolete now with single file
#   `ln -s "${buildDir}/${asmId}.genomes.txt" "${destDir}/genomes.txt"` if (-s "${buildDir}/${asmId}.genomes.txt");
  `ln -s "${buildDir}/${asmId}.download.genomes.txt" "${destDir}/download.genomes.txt"` if (-s "${buildDir}/${asmId}.download.genomes.txt");
   if (-s "${buildDir}/${asmId}.singleFile.hub.txt") {
    `ln -s "${buildDir}/${asmId}.singleFile.hub.txt" "${destDir}/hub.txt"`;
    `ln -s "${buildDir}/${asmId}.download.hub.txt" "${destDir}/download.hub.txt"` if (-s "${buildDir}/${asmId}.download.hub.txt");
   } else {
    `ln -s "${buildDir}/${asmId}.hub.txt" "${destDir}/hub.txt"` if (-s "${buildDir}/${asmId}.hub.txt");
   }
  `ln -s "${buildDir}/${asmId}.groups.txt" "${destDir}/groups.txt"` if (-s "${buildDir}/${asmId}.groups.txt");
  `ln -s "${buildDir}/${asmId}.userTrackDb.txt" "${destDir}/${accessionId}.userTrackDb.txt"` if ( -s "${buildDir}/${asmId}.userTrackDb.txt");
   if (-s "${buildDir}/${asmId}.bigMaf.trackDb.txt") {
     `rm -f "${destDir}/${asmId}.bigMaf.trackDb.txt"`;
     `ln -s "${buildDir}/${asmId}.bigMaf.trackDb.txt" "${destDir}/${asmId}.bigMaf.trackDb.txt"`;
   }
}
