#!/usr/bin/env perl

use strict;
use warnings;
use File::Basename;

sub usage() {
  printf STDERR "usage: asmHubChainNetTrackDb.pl <buildDir>\n";
  printf STDERR "expecting to find directories: buildDir/trackData/lastz.*/\n";
  printf STDERR "where each /lastz.*/ directory is one completed lastz/chainNet\n";
  printf STDERR "and basename(buildDir) is the 'target' sequence name\n";
  exit 255;
}

my $argc = scalar(@ARGV);

if ($argc != 1) {
  usage;
}

my $buildDir = shift;
my $targetDb = basename($buildDir);
my @queryList;

open (DL, "ls -d $buildDir/trackData/lastz.*|") or die "can not list $buildDir/trackData/lastz.*";
while (my $lastzDir = <DL>) {
  chomp $lastzDir;
  my $queryDb = basename($lastzDir);
  $queryDb =~ s/lastz.//;
  push @queryList, $queryDb;
}
close (DL);

# foreach my $queryDb (@queryList) {
#  printf "%s vs. %s\n", $queryDb, $targetDb;
#}

##### begin trackDb output ######
printf "track %sChainNet\n", $targetDb;
printf "compositeTrack on
shortLabel Chain/Net
longLabel Chain and Net alignments to target sequence: %s\n", $targetDb;
printf "subGroup1 view Views chain=Chains net=Nets\n";
printf "subGroup2 species Species";
my $N = 0;
foreach my $queryDb (@queryList) {
  printf " s%03d=%s", $N++, $queryDb;
}
printf "\n";
printf "subGroup3 clade Clade c00=human\n";
printf "dragAndDrop subTracks\n";
printf "visibility pack
group compGeno
noInherit on
color 0,0,0
altColor 255,255,0
type bed 3
chainLinearGap loose
chainMinScore 5000
dimensions dimensionX=clade dimensionY=species
sortOrder species=+ view=+ clade=+
configurable on\n";
printf "html %sChainNet\n", $targetDb;

printf "
    track %sChainNetViewchain
    shortLabel  Chains
    view chain
    visibility pack
    subTrack %sChainNet
    spectrum on
", $targetDb, $targetDb;

$N = 0;
foreach my $queryDb (@queryList) {
  my $QueryDb = ucfirst($queryDb);
  `rm -f $buildDir/bbi/$targetDb.chain${QueryDb}.bb`;
  `rm -f $buildDir/bbi/$targetDb.chain${QueryDb}Link.bb`;
  `ln -s ../trackData/lastz.$queryDb/axtChain/chain${QueryDb}.bb  $buildDir/bbi/$targetDb.chain${QueryDb}.bb`;
  `ln -s ../trackData/lastz.$queryDb/axtChain/chain${QueryDb}Link.bb  $buildDir/bbi/$targetDb.chain${QueryDb}Link.bb`;
  my $queryDate = "some date";
  if ( $queryDb !~ m/^GC/ ) {
    $queryDate = `hgsql -N -e 'select description from dbDb where name="$queryDb"' hgcentraltest | sed -e 's/ (.*//;'`;
    chomp $queryDate;
  }
  printf "
        track chain%s
        subTrack %sChainNetViewchain off
        subGroups view=chain species=s%03d clade=c00
        shortLabel %s Chain
        longLabel %s (%s) Chained Alignments
        type bigChain %s
        bigDataUrl bbi/%s.chain%s.bb
        linkDataUrl bbi/%s.chain%sLink.bb
        otherDb %s
", $QueryDb, $targetDb, $N, $queryDb, $queryDb, $queryDate, $queryDb, $targetDb,
     $QueryDb, $targetDb, $QueryDb, $queryDb;

  if ( -s "$buildDir/trackData/lastz.$queryDb/axtChain/chainSyn${QueryDb}.bb" ) {
    `rm -f $buildDir/bbi/$targetDb.chainSyn${QueryDb}.bb`;
    `rm -f $buildDir/bbi/$targetDb.chainSyn${QueryDb}Link.bb`;
    `ln -s ../trackData/lastz.$queryDb/axtChain/chainSyn${QueryDb}.bb  $buildDir/bbi/$targetDb.chainSyn${QueryDb}.bb`;
    `ln -s ../trackData/lastz.$queryDb/axtChain/chainSyn${QueryDb}Link.bb  $buildDir/bbi/$targetDb.chainSyn${QueryDb}Link.bb`;
  printf "
        track chainSyn%s
        subTrack %sChainNetViewchain off
        subGroups view=chain species=s%03d clade=c00
        shortLabel %s synChain
        longLabel %s (%s) Syntenic Chained Alignments
        type bigChain %s
        bigDataUrl bbi/%s.chainSyn%s.bb
        linkDataUrl bbi/%s.chainSyn%sLink.bb
        otherDb %s
", $QueryDb, $targetDb, $N, $queryDb, $queryDb, $queryDate, $queryDb, $targetDb,
     $QueryDb, $targetDb, $QueryDb, $queryDb;

  }

  if ( -s "$buildDir/trackData/lastz.$queryDb/axtChain/chainRBest${QueryDb}.bb" ) {
    `rm -f $buildDir/bbi/$targetDb.chainRBest${QueryDb}.bb`;
    `rm -f $buildDir/bbi/$targetDb.chainRBest${QueryDb}Link.bb`;
    `ln -s ../trackData/lastz.$queryDb/axtChain/chainRBest${QueryDb}.bb  $buildDir/bbi/$targetDb.chainRBest${QueryDb}.bb`;
    `ln -s ../trackData/lastz.$queryDb/axtChain/chainRBest${QueryDb}Link.bb  $buildDir/bbi/$targetDb.chainRBest${QueryDb}Link.bb`;
  printf "
        track chainRBest%s
        subTrack %sChainNetViewchain off
        subGroups view=chain species=s%03d clade=c00
        shortLabel %s rbChain
        longLabel %s (%s) Reciprocal Best Chained Alignments
        type bigChain %s
        bigDataUrl bbi/%s.chainRBest%s.bb
        linkDataUrl bbi/%s.chainRBest%sLink.bb
        otherDb %s
", $QueryDb, $targetDb, $N, $queryDb, $queryDb, $queryDate, $queryDb, $targetDb,
     $QueryDb, $targetDb, $QueryDb, $queryDb;

  }
  $N++;
}

printf "
    track %sMafNetViewnet
    shortLabel Nets
    view net
    visibility dense
    subTrack %sChainNet
", $targetDb, $targetDb;

$N = 0;
foreach my $queryDb (@queryList) {
  my @targetAccession = split('_', $targetDb);
  my $targetAcc = sprintf("%s_%s", $targetAccession[0], $targetAccession[1]);
  my $QueryDb = ucfirst($queryDb);
  `rm -f $buildDir/bbi/$targetDb.${queryDb}.net.bb`;
  `rm -f $buildDir/bbi/$targetDb.${queryDb}.net.summary.bb`;
  `ln -s ../trackData/lastz.$queryDb/bigMaf/${targetAcc}.${queryDb}.net.bb  $buildDir/bbi/$targetDb.${queryDb}.net.bb`;
  `ln -s ../trackData/lastz.$queryDb/bigMaf/${targetAcc}.${queryDb}.net.summary.bb  $buildDir/bbi/$targetDb.${queryDb}.net.summary.bb`;
  my $queryDate = "some date";
  if ( $queryDb !~ m/^GC/ ) {
    $queryDate = `hgsql -N -e 'select description from dbDb where name="$queryDb"' hgcentraltest | sed -e 's/ (.*//;'`;
    chomp $queryDate;
  }
    printf "
        track net%s
        parent %sMafNetViewnet
        subGroups view=net species=s%03d clade=c00
        shortLabel %s mafNet
        longLabel %s (%s) mafNet Alignment
        type bigMaf
        bigDataUrl bbi/%s.%s.net.bb
        summary bbi/%s.%s.net.summary.bb
        speciesOrder %s
        html html/%s.chainNet
", $QueryDb, $targetDb, $N, $queryDb, $queryDb, $queryDate, $targetDb, $queryDb, $targetDb, $queryDb, $queryDb, $targetDb;

  if ( -s "$buildDir/trackData/lastz.$queryDb/bigMaf/${targetAcc}.${queryDb}.synNet.bb" ) {
  `rm -f $buildDir/bbi/$targetDb.${queryDb}.synNet.bb`;
  `rm -f $buildDir/bbi/$targetDb.${queryDb}.synNet.summary.bb`;
  `ln -s ../trackData/lastz.$queryDb/bigMaf/${targetAcc}.${queryDb}.synNet.bb  $buildDir/bbi/$targetDb.${queryDb}.synNet.bb`;
  `ln -s ../trackData/lastz.$queryDb/bigMaf/${targetAcc}.${queryDb}.synNet.summary.bb  $buildDir/bbi/$targetDb.${queryDb}.synNet.summary.bb`;
    printf "
        track synNet%s
        parent %sMafNetViewnet
        subGroups view=net species=s%03d clade=c00
        shortLabel %s synNet
        longLabel %s (%s) Syntenic Net Alignment
        type bigMaf
        bigDataUrl bbi/%s.%s.synNet.bb
        summary bbi/%s.%s.synNet.summary.bb
        speciesOrder %s
        html html/%s.chainNet
", $QueryDb, $targetDb, $N, $queryDb, $queryDb, $queryDate, $targetDb, $queryDb, $targetDb, $queryDb, $queryDb, $targetDb;

  }

  if ( -s "$buildDir/trackData/lastz.$queryDb/bigMaf/${targetAcc}.${queryDb}.rbestNet.bb" ) {
  `rm -f $buildDir/bbi/$targetDb.${queryDb}.rbestNet.bb`;
  `rm -f $buildDir/bbi/$targetDb.${queryDb}.rbestNet.summary.bb`;
  `ln -s ../trackData/lastz.$queryDb/bigMaf/${targetAcc}.${queryDb}.rbestNet.bb  $buildDir/bbi/$targetDb.${queryDb}.rbestNet.bb`;
  `ln -s ../trackData/lastz.$queryDb/bigMaf/${targetAcc}.${queryDb}.rbestNet.summary.bb  $buildDir/bbi/$targetDb.${queryDb}.rbestNet.summary.bb`;
    printf "
        track rbestNet%s
        parent %sMafNetViewnet
        subGroups view=net species=s%03d clade=c00
        shortLabel %s rbestNet
        longLabel %s (%s) Reciprocal Best Net Alignment
        type bigMaf
        bigDataUrl bbi/%s.%s.rbestNet.bb
        summary bbi/%s.%s.rbestNet.summary.bb
        speciesOrder %s
        html html/%s.chainNet
", $QueryDb, $targetDb, $N, $queryDb, $queryDb, $queryDate, $targetDb, $queryDb, $targetDb, $queryDb, $queryDb, $targetDb;

  }
  $N++;
}
