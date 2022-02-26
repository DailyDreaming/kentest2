package commonHtml;

# helper functions for common HTML output features

use warnings;
use strict;
use Carp;
use vars qw(@ISA @EXPORT_OK);
use Exporter;

@ISA = qw(Exporter);

# This is a listing of the public methods and variables (which should be
# treated as constants) exported by this module:
@EXPORT_OK = (
  qw( otherHubLinks htmlFooter )
);

# otherHubLinks: arg one: vgpIndex, arg two: asmHubName, arg three: orderList
# arguments allow decision on customization of the table for different
# types of assembly hubs

sub otherHubLinks($$) {

  my ($vgpIndex, $asmHubName) = @_;

my %asmCounts;	# key is hubName, value is number of assemblies
my @hubList = qw(primates mammals birds fish vertebrate invertebrate plants fungi);
foreach my $hubName (@hubList) {
  my $asmCount = `grep -v "^#" ../${hubName}AsmHub/${hubName}.orderList.tsv | wc -l`;
  chomp $asmCount;
  $asmCounts{$hubName} = $asmCount;
}
my $vgpCount = `grep -h -v "^#" ../vgpAsmHub/vgp.*.orderList.tsv | wc -l`;
chomp $vgpCount;
$asmCounts{'vgp'} = $vgpCount;
my $legacyCount = `grep -h -v "^#" ../legacyAsmHub/legacy.orderList.tsv | wc -l`;
chomp $legacyCount;
$asmCounts{'legacy'} = $legacyCount;

# different table output for VGP index

if ((0 == $vgpIndex) && ($asmHubName ne "viral")) {
  printf "<p>\n<table border='1' style='margin-left:auto; margin-right:auto;'><thead>\n";
  printf "<tr><th colspan=4 style='text-align:center;'>Additional hubs with collections of assemblies</th></tr>\n";
  printf "<tr><th>Collection</th>\n";
  printf "    <th>Hub index pages:</th>\n";
  printf "    <th>Assembly statistics:</th>\n";
  printf "    <th>Track statistics:</th>\n";
  printf "</tr>\n";
  foreach my $hubName (@hubList) {
    if ($hubName =~ m/^vertebrate/) {
      printf "<tr><th>other vertebrates</th>\n";
    } elsif ($hubName =~ m/fish/) {
      printf "<tr><th>Fishes</th>\n";
    } elsif ($hubName =~ m/invertebrate/) {
      printf "<tr><th>Invertebrates</th>\n";
    } else {
      printf "<tr><th>%s</th>\n", ucfirst($hubName);
    }
    printf "    <th style='text-align:right'><a href='../%s/index.html'>%d assemblies</a></th>\n", $hubName, $asmCounts{$hubName};
    printf "    <th><a href='../%s/asmStats.html'>assembly stats</a></th>\n", $hubName;
    printf "    <th><a href='../%s/trackData.html'>track stats</a></th>\n", $hubName;
    printf "</tr>\n";
  }
  printf "<tr><th>VGP - Vertebrate Genome Project</th>\n";
  printf "    <th style='text-align:right'><a href='../%s/index.html'>%d assemblies</a></th>\n", "VGP", $asmCounts{'vgp'};
  printf "    <th><a href='../VGP/asmStats.html'>assembly stats</a></th>\n";
  printf "    <th><a href='../VGP/trackData.html'>track stats</a></th>\n";
  printf "</tr>\n";
  printf "<tr><th>legacy/superseded</th>\n";
  printf "    <th style='text-align:right'><a href='../%s/index.html'>%d assemblies</a></th>\n", "legacy", $asmCounts{'legacy'};
  printf "    <th><a href='../legacy/asmStats.html'>assembly stats</a></th>\n";
  printf "    <th><a href='../legacy/trackData.html'>track stats</a></th>\n";
  printf "</tr></thead>\n</table>\n</p>\n";
} elsif (1 == $vgpIndex) {
  printf "<p>\n<table border='1'><thead>\n";
  printf "<tr><th colspan=5 style='text-align:center;'>Alternate sets of VGP assemblies</th><th style='text-align:center;'>NCBI Refseq</th></tr>\n";
  printf "<tr><th>Index pages:&nbsp;</th>\n";
  printf "<th><a href='index.html'>primary assembly</a></th>\n";
  printf "<th><a href='vgpAlt.html'>alternate/haplotype</a></th>\n";
  printf "<th><a href='vgpTrio.html'>trio mat/pat</a></th>\n";
  printf "<th><a href='vgpLegacy.html'>legacy/superseded</a></th>\n";
  printf "<th><a href='../primates/index.html'>other NCBI Refseq assemblies</a></th>\n";

  printf "</tr><tr>\n";
  printf "<th>Assembly statistics:&nbsp;</th>\n";
  printf "<th><a href='asmStats.html'>primary assembly</a></th>\n";
  printf "<th><a href='vgpAltStats.html'>alternate/haplotype</a></th>\n";
  printf "<th><a href='vgpTrioStats.html'>trio mat/pat</a></th>\n";
  printf "<th><a href='vgpLegacyStats.html'>legacy/superseded</a></th>\n";
  printf "<th><a href='../primates/asmStats.html'>other NCBI Refseq assemblies</a></th>\n";

  printf "</tr><tr>\n";
  printf "<th>Track statistics:&nbsp;</th>\n";
  printf "<th><a href='trackData.html'>primary assembly</a></th>\n";
  printf "<th><a href='vgpAltData.html'>alternate/haplotype</a></th>\n";
  printf "<th><a href='vgpTrioData.html'>trio mat/pat</a></th>\n";
  printf "<th><a href='vgpLegacyData.html'>legacy/superseded</a></th>\n";
  printf "<th><a href='../primates/trackData.html'>other NCBI Refseq assemblies</a></th>\n";

  printf "</tr><tr>\n";
  printf "</tr></thead>\n</table>\n</p>\n";
}

}	#	sub otherHubLinks($$)

############################################################################
# common output at the bottom of an html index page
sub htmlFooter() {

print <<"END"
</div><!-- closing gbsPage from gbPageStartHardcoded.html -->
</div><!-- closing container-fluid from gbPageStartHardcoded.html -->
<!--#include virtual="\$ROOT/inc/gbFooterHardcoded.html"-->
<script type="text/javascript" src="<!--#echo var="ROOT" -->/js/sorttable.js"></script>
</body></html>
END
}
############################################################################
