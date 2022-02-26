#!/usr/bin/env perl

use strict;
use warnings;
use File::Basename;

my $argc = scalar(@ARGV);
if ($argc != 3) {
  printf STDERR "mkGenomes.pl blatHost blatPort [two column name list] > .../hub/genomes.txt\n";
  printf STDERR "e.g.: mkGenomes.pl dynablat-01 4040 vgp.primary.assemblies.tsv > .../vgp/genomes.txt\n";
  printf STDERR "e.g.: mkGenomes.pl hgwdev 4040 vgp.primary.assemblies.tsv > .../vgp/download.genomes.txt\n";
  printf STDERR "the name list is found in \$HOME/kent/src/hg/makeDb/doc/asmHubs/\n";
  printf STDERR "\nthe two columns are 1: asmId (accessionId_assemblyName)\n";
  printf STDERR "column 2: common name for species, columns separated by tab\n";
  printf STDERR "result will write a local asmId.genomes.txt file for each hub\n";
  printf STDERR "and a local asmId.hub.txt file for each hub\n";
  printf STDERR "and a local asmId.groups.txt file for each hub\n";
  printf STDERR "and the output to stdout will be the overall genomes.txt\n";
  printf STDERR "index file for all genomes in the given list\n";
  exit 255;
}

my $downloadHost = "hgwdev";
my @blatHosts = qw( dynablat-01 dynablat-01 );
my @blatPorts = qw( 4040 4040 );
my $blatHostDomain = ".soe.ucsc.edu";

################### writing out hub.txt file, twice ##########################
sub singleFileHub($$$$$$$$$$) {
  my ($fh1, $fh2, $accessionId, $orgName, $descr, $asmId, $defPos, $taxId, $trackDb, $accessionDir) = @_;
  my @fhN;
  push @fhN, $fh1;
  push @fhN, $fh2;

  my $fileCount = 0;
  my @tdbLines;
  open (TD, "<$trackDb") or die "can not read trackDb: $trackDb";
  while (my $tdbLine = <TD>) {
     chomp $tdbLine;
     push @tdbLines, $tdbLine;
  }
  close (TD);
  foreach my $fh (@fhN) {
    printf $fh "hub %s genome assembly\n", $accessionId;
    printf $fh "shortLabel %s\n", $orgName;
    printf $fh "longLabel %s/%s/%s genome assembly\n", $orgName, $descr, $asmId;
    printf $fh "useOneFile on\n";
    printf $fh "email hclawson\@ucsc.edu\n";
    printf $fh "descriptionUrl html/%s.description.html\n", $asmId;
    printf $fh "\n";
    printf $fh "genome %s\n", $accessionId;
    printf $fh "taxId %s\n", $taxId if (length($taxId) > 1);
    printf $fh "groups groups.txt\n";
    printf $fh "description %s\n", $orgName;
    printf $fh "twoBitPath %s.2bit\n", $accessionId;
    printf $fh "chromSizes %s.chrom.sizes.txt\n", $accessionId;
    printf $fh "chromAlias %s.chromAlias.txt\n", $accessionId;
    printf $fh "organism %s\n", $descr;
    printf $fh "defaultPos %s\n", $defPos;
    printf $fh "scientificName %s\n", $descr;
    printf $fh "htmlPath html/%s.description.html\n", $asmId;
    # until blat server host is ready for hgdownload, avoid these lines
    if ($blatHosts[$fileCount] ne $downloadHost) {
      printf $fh "blat %s%s %s dynamic $accessionDir/$accessionId\n", $blatHosts[$fileCount], $blatHostDomain, $blatPorts[$fileCount];
      printf $fh "transBlat %s%s %s dynamic $accessionDir/$accessionId\n", $blatHosts[$fileCount], $blatHostDomain, $blatPorts[$fileCount];
      printf $fh "isPcr %s%s %s dynamic $accessionDir/$accessionId\n", $blatHosts[$fileCount], $blatHostDomain, $blatPorts[$fileCount];
    }
    printf $fh "\n";
    foreach my $tdbLine (@tdbLines) {
      printf $fh "%s\n", $tdbLine;
    }
    ++$fileCount;
  }
}

##############################################################################
my $home = $ENV{'HOME'};
my $toolsDir = "$home/kent/src/hg/makeDb/doc/asmHubs";

my $blatHost = shift;
my $blatPort = shift;
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
  if (!defined($commonName)) {
    printf STDERR "ERROR: missing tab sep common name:\n'%s'\n", $line;
    exit 255;
  }
  if (defined($commonName{$asmId})) {
    printf STDERR "ERROR: duplicate asmId: '%s'\n", $asmId;
    printf STDERR "previous name: '%s'\n", $commonName{$asmId};
    printf STDERR "duplicate name: '%s'\n", $commonName;
    exit 255;
  }
  $commonName{$asmId} = $commonName;
  push @orderList, $asmId;
  printf STDERR "orderList[$assemblyCount] = $asmId\n";
  ++$assemblyCount;
}
close (FH);

my $buildDone = 0;
my $orderKey = 0;
foreach my $asmId (@orderList) {
  ++$orderKey;
  next if ($asmId !~ m/^GC/);
  my ($gcPrefix, $accession, undef) = split('_', $asmId);
  my $accessionId = sprintf("%s_%s", $gcPrefix, $accession);
  my $accessionDir = substr($asmId, 0 ,3);
  $accessionDir .= "/" . substr($asmId, 4 ,3);
  $accessionDir .= "/" . substr($asmId, 7 ,3);
  $accessionDir .= "/" . substr($asmId, 10 ,3);
  my $buildDir = "/hive/data/genomes/asmHubs/refseqBuild/$accessionDir/$asmId";
  my $destDir = "/hive/data/genomes/asmHubs/$accessionDir/$accessionId";
  if ($gcPrefix eq "GCA") {
     $buildDir = "/hive/data/genomes/asmHubs/genbankBuild/$accessionDir/$asmId";
  }
  if ( ! -s "${buildDir}/${asmId}.chrom.sizes" ) {
    printf STDERR "# ERROR: missing ${asmId}.chrom.sizes in\n# ${buildDir}\n";
    next;
  }
  if ( ! -s "${buildDir}/${asmId}.chromAlias.txt" ) {
    printf STDERR "# ERROR: missing ${asmId}.chromAlias.txt in\n# ${buildDir}\n";
    next;
  }
  my $asmReport="$buildDir/download/${asmId}_assembly_report.txt";
  my $trackDb = "$buildDir/$asmId.trackDb.txt";
  if ( ! -s "${trackDb}" ) {
    printf STDERR "# %03d not built yet: %s\n", $orderKey, $asmId;
    printf STDERR "# '%s'\n", $trackDb;
    next;
  }
  if ( ! -s "${asmReport}" ) {
    printf STDERR "# %03d missing assembly_report: %s\n", $orderKey, $asmId;
    next;
  }
  ++$buildDone;
printf STDERR "# %03d genomes.txt %s/%s\n", $buildDone, $accessionDir, $accessionId;
  my $taxId=`grep -i "taxid:" $asmReport | head -1 | awk '{printf \$(NF)}' | tr -d \$'\\r'`;
  chomp $taxId;
  my $descr=`grep -i "organism name:" $asmReport | head -1 | sed -e 's#.*organism name: *##i; s# (.*\$##;'`;
  chomp $descr;
  my $orgName=`grep -i "organism name:" $asmReport | head -1 | sed -e 's#.* name: .* (##; s#).*##;'`;
  chomp $orgName;
  if (defined($commonName{$asmId})) {
     $orgName = $commonName{$asmId};
  }

  printf "genome %s\n", $accessionId;
  printf "taxId %s\n", $taxId if (length($taxId) > 1);
  printf "trackDb ../%s/%s/trackDb.txt\n", $accessionDir, $accessionId;
  printf "groups groups.txt\n";
  printf "description %s\n", $orgName;
  printf "twoBitPath ../%s/%s/%s.2bit\n", $accessionDir, $accessionId, $accessionId;
  printf "chromSizes ../%s/%s/%s.chrom.sizes.txt\n", $accessionDir, $accessionId, $accessionId;
  printf "chromAlias ../%s/%s/%s.chromAlias.txt\n", $accessionDir, $accessionId, $accessionId;
  printf "organism %s\n", $descr;
  my $chrName=`head -1 $buildDir/$asmId.chrom.sizes | awk '{print \$1}'`;
  chomp $chrName;
  my $bigChrom=`head -1 $buildDir/$asmId.chrom.sizes | awk '{print \$NF}'`;
  chomp $bigChrom;
  my $oneThird = int($bigChrom/3);
  my $tenK = $oneThird + 10000;
  $tenK = $bigChrom if ($tenK > $bigChrom);
  my $defPos="${chrName}:${oneThird}-${tenK}";
  if ( -s "$asmId/defaultPos.txt" ) {
    $defPos=`cat "$asmId/defaultPos.txt"`;
    chomp $defPos;
  }
  printf "defaultPos %s\n", $defPos;
  printf "orderKey %d\n", $buildDone;
  printf "scientificName %s\n", $descr;
  printf "htmlPath ../%s/%s/html/%s.description.html\n", $accessionDir, $accessionId, $asmId;
  # until blat server host is ready for hgdownload, avoid these lines
  if ($blatHost ne $downloadHost) {
    if ( -s "${destDir}/$accessionId.trans.gfidx" ) {
      printf "blat $blatHost$blatHostDomain $blatPort dynamic $accessionDir/$accessionId\n";
    printf "transBlat $blatHost$blatHostDomain $blatPort dynamic $accessionDir/$accessionId\n";
      printf "isPcr $blatHost$blatHostDomain $blatPort dynamic $accessionDir/$accessionId\n";
    }
  }
  printf "\n";

  # the original multi-file system:
  my $localHubTxt = "$buildDir/${asmId}.hub.txt";
  open (HT, ">$localHubTxt") or die "can not write to $localHubTxt";
  printf HT "hub %s genome assembly\n", $accessionId;
  printf HT "shortLabel %s\n", $orgName;
  printf HT "longLabel %s/%s/%s genome assembly\n", $orgName, $descr, $asmId;
  printf HT "genomesFile genomes.txt\n";
  printf HT "email hclawson\@ucsc.edu\n";
  printf HT "descriptionUrl html/%s.description.html\n", $asmId;
  close (HT);

  # try creating single file hub.txt, one for hgwdev, one for hgdownload
  my $downloadHubTxt = "$buildDir/${asmId}.download.hub.txt";
  open (DL, ">$downloadHubTxt") or die "can not write to $downloadHubTxt";
  $localHubTxt = "$buildDir/${asmId}.singleFile.hub.txt";
  open (HT, ">$localHubTxt") or die "can not write to $localHubTxt";

  singleFileHub(\*HT, \*DL, $accessionId, $orgName, $descr, $asmId,
	$defPos, $taxId, $trackDb, $accessionDir);

  my $localGenomesFile = "$buildDir/${asmId}.genomes.txt";
  open (GF, ">$localGenomesFile") or die "can not write to $localGenomesFile";
  printf GF "genome %s\n", $accessionId;
  printf GF "taxId %s\n", $taxId if (length($taxId) > 1);
  printf GF "trackDb trackDb.txt\n";
  printf GF "groups groups.txt\n";
  printf GF "description %s\n", $orgName;
  printf GF "twoBitPath %s.2bit\n", $accessionId;
  printf GF "chromSizes %s.chrom.sizes.txt\n", $accessionId;
  printf GF "chromAlias %s.chromAlias.txt\n", $accessionId;
  printf GF "organism %s\n", $descr;
  printf GF "defaultPos %s\n", $defPos;
  printf GF "scientificName %s\n", $descr;
  printf GF "htmlPath html/%s.description.html\n", $asmId;
  # until blat server host is ready for hgdownload, avoid these lines
  if ($blatHost ne $downloadHost) {
    if ( -s "${destDir}/$accessionId.trans.gfidx" ) {
      printf GF "blat $blatHost$blatHostDomain $blatPort dynamic $accessionDir/$accessionId\n";
      printf GF "transBlat $blatHost$blatHostDomain $blatPort dynamic $accessionDir/$accessionId\n";
     printf GF "isPcr $blatHost$blatHostDomain $blatPort dynamic $accessionDir/$accessionId\n";
    }
  }
  close (GF);

  my $localGroups = "$buildDir/${asmId}.groups.txt";
  open (GR, ">$localGroups") or die "can not write to $localGroups";
  print GR <<_EOF_
name user
label Custom Tracks
priority 1
defaultIsClosed 1

name map
label Mapping and Sequencing
priority 2
defaultIsClosed 0

name genes
label Genes and Gene Predictions
priority 3
defaultIsClosed 0

name rna
label mRNA and EST
priority 4
defaultIsClosed 0

name regulation
label Regulation
priority 5
defaultIsClosed 0

name compGeno
label Comparative Genomics
priority 6
defaultIsClosed 0

name varRep
label Variation
priority 7
defaultIsClosed 0

name x
label Experimental
priority 10
defaultIsClosed 1
_EOF_
   ;
   close (GR);
}

__END__

description Mastacembelus armatus
twoBitPath GCA_900324485.2_fMasArm1.2/trackData/addMask/GCA_900324485.2_fMasArm1.2.masked.2bit
organism Zig-Zag eel
defaultPos LR535842.1:14552035-14572034
orderKey 1
scientificName Mastacembelus armatus
htmlPath GCA_900324485.2_fMasArm1.2/html/GCA_900324485.2_fMasArm1.2.description.html

# head -25 GCA_002180035.3_HG00514_prelim_3.0_assembly_report.txt

# Assembly name:  HG00514_prelim_3.0
# Organism name:  Homo sapiens (human)
# Isolate:  HG00514
# Sex:  female
# Taxid:          9606
# BioSample:      SAMN04229552
# BioProject:     PRJNA300843
# Submitter:      The Genome Institute at Washington University School of Medicine
# Date:           2018-05-22
# Assembly type:  haploid
# Release type:   major
# Assembly level: Chromosome
# Genome representation: full
# WGS project:    NIOH01
# Assembly method: Falcon v. November 2016
# Expected final version: no
# Genome coverage: 80.0x
# Sequencing technology: PacBio RSII
# GenBank assembly accession: GCA_002180035.3
#
## Assembly-Units:
## GenBank Unit Accession       RefSeq Unit Accession   Assembly-Unit name
## GCA_002180045.3              Primary Assembly
