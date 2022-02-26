#!/bin/sh

if [ $# -ne 1 ]; then
    echo "usage: ./commonNames.sh <Nway.nh>"
    echo "Renames the names from UCSC database names to common names."
    echo "This depends upon the exact UCSC database names present in the"
    echo ".nh file.  Last updated to work with the 96way.nh file."
    exit 255
fi


export F=$1

/cluster/bin/phast/tree_doctor -r \
"acaChl1 -> Rifleman ;
aciJub1 -> Cheetah ;
ailMel1 -> Panda ;
allMis1 -> American_alligator ;
allSin1 -> Chinese_alligator ;
amaVit1 -> Puerto_Rican_parrot ;
anaPla1 -> Mallard_duck ;
ancCey1 -> A__ceylanicum;
angJap1 -> Japanese_eel ;
anoCar2 -> Lizard ;
aotNan1 -> Nancy_Ma_s_night_monkey ;
apaSpi1 -> Spiny_softshell_turtle ;
apaVit1 -> Bar_tailed_trogon ;
aptFor1 -> Emperor_penguin ;
aquChr1 -> Golden_eagle ;
aquChr2 -> Golden_eagle ;
araMac1 -> Scarlet_macaw ;
ascSuu1 -> Pig_roundworm;
astMex1 -> Mexican_tetra ;
balAcu1 -> Minke_whale ;
balPav1 -> Grey_crowned_crain ;
bisBis1 -> Bison ;
bosInd1 -> Zebu_cattle ;
bosMut1 -> Wild_yak ;
bosTau6 -> Cow ;
bosTau7 -> Cow ;
bosTau8 -> Cow ;
bruMal2 -> Filarial_worm;
bubBub1 -> Water_buffalo ;
bucRhi1 -> Rhinoceros_hornbill ;
burXyl1 -> Pine_wood_nematode;
caeAng2 -> C__angaria;
caeJap4 -> C__japonica;
caePb3 -> C__brenneri;
caeRem4 -> C__remanei;
caeSp111 -> C__tropicalis;
caeSp51 -> C__sp__5_ju800;
calAnn1 -> Anna_s_hummingbird ;
calJac3 -> Marmoset ;
calMil1 -> Elephant_shark ;
camBac1 -> Bactrian_camel ;
camDro1 -> Arabian_camel ;
camFer1 -> Wild_bactrian_camel ;
canFam2 -> Dog ;
canFam3 -> Dog ;
canLup1 -> Gray_wolf ;
capCar1 -> Chuck_will_s_widow ;
capHir1 -> Domestic_goat ;
carCri1 -> Red_legged_seriema ;
catAur1 -> Turkey_vulture ;
cavApe1 -> Brazilian_guinea_pig ;
cavPor3 -> Guinea_pig ;
cb4 -> C__briggsae;
ce11 -> C__elegans;
cebCap1 -> White_faced_sapajou ;
cerAty1 -> Sooty_mangabey ;
cerSim1 -> White_rhinoceros ;
chaPel1 -> Chimney_swift ;
chaVoc1 -> Killdeer ;
chaVoc2 -> Killdeer ;
cheMyd1 -> Green_seaturtle ;
chiLan1 -> Chinchilla ;
chlSab1 -> Green_monkey ;
chlSab2 -> Green_monkey ;
chlUnd1 -> Macqueen_s_bustard ;
choHof1 -> Sloth ;
chrAsi1 -> Cape_golden_mole ;
chrPic1 -> Painted_turtle ;
chrPic2 -> Painted_turtle ;
colAng1 -> Angolan_colobus ;
colLiv1 -> Rock_pigeon ;
colStr1 -> Speckled_mousebird ;
conCri1 -> Star_nosed_mole ;
corBra1 -> American_crow ;
corCor1 -> Hooded_crow ;
cotJap1 -> Japanese_quail ;
cotJap2 -> Japanese_quail ;
criGri1 -> Chinese_hamster ;
croPor1 -> Crocodile ;
croPor2 -> Crocodile ;
cucCan1 -> Common_cuckoo ;
cynSem1 -> Tongue sole ;
cypVar1 -> Sheepshead_minnow ;
danRer10 -> Zebrafish ;
danRer11 -> Zebrafish ;
danRer7 -> Zebrafish ;
dasNov2 -> Armadillo ;
dasNov3 -> Armadillo ;
dicLab1 -> European_sea_bass ;
dipOrd1 -> Kangaroo_rat ;
dipOrd2 -> Kangaroo_rat ;
dirImm1 -> Dog_heartworm;
echTel1 -> Tenrec ;
echTel2 -> Tenrec ;
egrGar1 -> Little_egret ;
eidHel1 -> Straw_colored_fruit_bat ;
eleEdw1 -> Cape_elephant_shrew ;
eptFus1 -> Big_brown_bat ;
equAsi1 -> Donkey ;
equCab2 -> Horse ;
equPrz1 -> Przewalski_horse ;
eriEur1 -> Hedgehog ;
eriEur2 -> Hedgehog ;
esoLuc1 -> Northern_pike ;
eulFla1 -> Sclater_s_lemur ;
eulMac1 -> Black_lemur ;
eurHel1 -> Sunbittern ;
falChe1 -> Saker_falcon ;
falPer1 -> Peregrine_falcon ;
felCat4 -> Cat ;
felCat5 -> Cat ;
felCat8 -> Cat ;
ficAlb1 -> Collared_flycatcher ;
ficAlb2 -> Collared_flycatcher ;
fr2 -> Fugu ;
fr3 -> Fugu ;
fukDam1 -> Damara_mole_rat ;
fulGla1 -> Northern_fulmar ;
gadMor1 -> Atlantic_cod ;
galGal3 -> Chicken ;
galGal4 -> Chicken ;
galGal5 -> Chicken ;
galGal6 -> Chicken ;
galVar1 -> Malayan_flying_lemur ;
gasAcu1 -> Stickleback ;
gavGan0 -> Gharial ;
gavGan1 -> Gharial ;
gavSte1 -> Red_throated_loon ;
gekJap1 -> Schlegel_s_Japanese_gecko ;
geoFor1 -> Medium_ground_finch ;
gorGor3 -> Gorilla ;
gorGor5 -> Gorilla ;
haeCon2 -> Barber_pole_worm;
halAlb1 -> White_tailed_eagle ;
halLeu1 -> Bald_eagle ;
hapBur1 -> Burton_s_mouthbreeder ;
hetBac1 -> H__bacteriophora/m31e;
hetGla1 -> Naked_mole_rat ;
hetGla2 -> Naked_mole_rat ;
hg19 -> Human ;
hg38 -> Human ;
hipArm1 -> Great_roundleaf_bat ;
jacJac1 -> Lesser_Egyptian_jerboa ;
latCha1 -> Coelacanth ;
lepDis1 -> Cuckoo_roller ;
lepOcu1 -> Spotted_gar ;
lepWed1 -> Weddell_seal ;
letCam1 -> Arctic_lamprey ;
lipVex1 -> Yangtze_river_dolphin ;
loaLoa1 -> Eye_worm;
loxAfr3 -> Elephant ;
macEug2 -> Wallaby ;
macFas5 -> Crab_eating_macaque ;
macNem1 -> Pig_tailed_macaque ;
manJav1 -> Malayan_pangolin ;
manLeu1 -> Drill ;
manPen1 -> Chinese_pangolin ;
manVit1 -> Golden_collared_manakin ;
manVit2 -> Golden_collared_manakin ;
marMar1 -> European_marmot ;
mayZeb1 -> Zebra_mbuna ;
megLyr1 -> Indian_false_vampire ;
melGal1 -> Turkey ;
melGal5 -> Turkey ;
melHap1 -> M__hapla;
melInc2 -> M__incognita;
melUnd1 -> Budgerigar ;
merNub1 -> Northern_carmine_bee_eater ;
mesAur1 -> Golden_hamster ;
mesUni1 -> Brown_roatelo ;
micMur1 -> Mouse_lemur ;
micMur3 -> Mouse_lemur ;
micOch1 -> Prairie_vole ;
minNat1 -> Natal_long_fingered_bat ;
mm10 -> Mouse ;
mm9 -> Mouse ;
monDom5 -> Opossum ;
musFur1 -> Ferret ;
musPut1 -> Ferret ;
myoBra1 -> Brand_s_bat ;
myoDav1 -> David_s_myotis ;
myoLuc1 -> Little_brown_bat ;
myoLuc2 -> Little_brown_bat ;
nanGal1 -> Upper_Galilee_mountains_blind_mole_rat ;
nanPar1 -> Tibetan_frog ;
nasLar1 -> Proboscis_monkey ;
necAme1 -> N__americanus;
neoBri1 -> Princess_of_Burundi ;
neoSch1 -> Hawaiian_monk_seal ;
nesNot1 -> Kea ;
nipNip1 -> Crested_ibis;
nomLeu1 -> Gibbon ;
nomLeu3 -> Gibbon ;
notCor1 -> Black_rockcod ;
ochPri2 -> Pika ;
ochPri3 -> Pika ;
octDeg1 -> Brush_tailed_rat ;
odoTex1 -> White_tailed_deer ;
odoRosDiv1 -> Pacific_walrus ;
oncVol1 -> O__volvulus;
opiHoa1 -> Hoatzin ;
orcOrc1 -> Killer_whale ;
oreNil1 -> Nile_tilapia ;
oreNil2 -> Nile_tilapia ;
oreNil3 -> Nile_tilapia ;
ornAna1 -> Platypus ;
ornAna2 -> Platypus ;
ornAna5 -> Platypus ;
oryAfe1 -> Aardvark ;
oryCun2 -> Rabbit ;
oryLat2 -> Medaka ;
otoGar1 -> Bushbaby ;
otoGar3 -> Bushbaby ;
oviAri1 -> Sheep ;
oviAri3 -> Sheep ;
oviMus1 -> Muflon ;
panHod1 -> Tibetan_antelope ;
panPan1 -> Bonobo ;
panPan2 -> Bonobo ;
panPar1 -> Leopard ;
panRed1 -> Microworm;
panTig1 -> Amur_tiger ;
panTro3 -> Chimp ;
panTro4 -> Chimp ;
panTro5 -> Chimp ;
papAnu2 -> Baboon ;
papAnu3 -> Baboon ;
papHam1 -> Baboon ;
pelCri1 -> Dalmatian_pelican ;
pelSin1 -> Chinese_softshell_turtle ;
perManBai1 -> Prairie_deer_mouse ;
petMar1 -> Lamprey ;
petMar2 -> Lamprey ;
petMar3 -> Lamprey ;
phaCar1 -> Great_cormorant ;
phaCin1 -> Koala ;
phaLep1 -> White_tailed_tropicbird ;
phoRub1 -> American_flamingo ;
phyCat1 -> Sperm_whale ;
picPub1 -> Downy_woodpecker ;
podCri1 -> Great_crested_grebe ;
poeFor1 -> Amazon_molly ;
poeRet1 -> Guppy ;
ponAbe2 -> Orangutan ;
ponAbe3 -> Orangutan ;
priExs1 -> P__exspectatus;
priPac3 -> P__pacificus;
proCap1 -> Rock_hyrax ;
proCap2 -> Rock_hyrax ;
proCoq1 -> Coquerel_s_sifaka ;
proMuc1 -> Brown_spotted_pit_viper ;
pseHum1 -> Tibetan_ground_jay ;
pteAle1 -> Black_flying_fox ;
pteGut1 -> Yellow_throated_sandgrouse ;
ptePar1 -> Parnell_s_mustached_bat ;
pteVam1 -> Megabat ;
pteVam2 -> Megabat ;
punNye1 -> Pundamilia_nyererei ;
pygAde1 -> Adelie_penguin ;
pytBiv1 -> Burmese_python ;
rheMac2 -> Rhesus ;
rheMac3 -> Rhesus ;
rheMac8 -> Rhesus ;
rhiBie1 -> Black_snub_nosed_monkey ;
rhiFer1 -> Greater_horseshoe_bat ;
rhiRox1 -> Golden_snub_nosed_monkey ;
rhiSin1 -> Chinese_horseshoe_bat ;
rn4 -> Rat ;
rn5 -> Rat ;
rn6 -> Rat ;
rouAeg1 -> Egyptian_fruit_bat ;
saiBol1 -> Squirrel_monkey ;
sarHar1 -> Tasmanian_devil ;
sebNig1 -> Tiger_fockfish ;
sebRub1 -> Flag_fockfish ;
serCan1 -> Canary ;
sorAra1 -> Shrew ;
sorAra2 -> Shrew ;
speTri1 -> Squirrel ;
speTri2 -> Squirrel ;
stePar1 -> Bicolor_damselfish ;
strCam1 -> Ostrich ;
strRat2 -> Threadworm;
susScr2 -> Pig ;
susScr3 -> Pig ;
taeGut1 -> Zebra_finch ;
taeGut2 -> Zebra_finch ;
takFla1 -> Yellowbelly_pufferfish ;
tarSyr1 -> Tarsier ;
tarSyr2 -> Tarsier ;
tauEry1 -> Red_crested_turaco ;
tetNig1 -> Tetraodon ;
tetNig2 -> Tetraodon ;
thaSir1 -> Garter_snake ;
tinGut1 -> White_throated_tinamou ;
tinGut2 -> White_throated_tinamou ;
triMan1 -> Manatee ;
triSpi1 -> Trichinella;
triSui1 -> Whipworm;
tupBel1 -> Tree_shrew ;
tupChi1 -> Chinese_tree_shrew ;
turTru1 -> Dolphin ;
turTru2 -> Dolphin ;
tytAlb1 -> Barn_owl ;
ursMar1 -> Polar_bear ;
vicPac1 -> Alpaca ;
vicPac2 -> Alpaca ;
vipBer1 -> Adder ;
xenLae2 -> African_clawed_frog ;
xenTro3 -> Frog_X_tropicalis ;
xenTro4 -> Frog_X_tropicalis ;
xenTro7 -> Frog_X_tropicalis ;
xenTro9 -> Frog_X_tropicalis ;
xipMac1 -> Southern_platyfish ;
zonAlb1 -> White_throated_sparrow ;" \
	${F} | sed -e "s/long_finger/long-finger/; s/rel_s/rel's/; s/Ma_s/Ma's/; s/White_faced/White-faced/; s/White_tailed_deer/White-tailed_deer/; s/Pig_tailed/Pig-tailed/; s/X_trop/X._trop/; s/Burton_s/Burton's/; s/Brand_s/Brand's/; s/David_s/David's/; s/Parnell_s/Parnell's/; s/queen_s/queen's/; s/will_s/will's/; s/Anna_s/Anna's/; s/_nosed/-nosed/; s/00*)/)/g; s/00*,/,/g; s/Schlegel_s/Schlegel's/; s/Sclater_s/Sclater's/"
