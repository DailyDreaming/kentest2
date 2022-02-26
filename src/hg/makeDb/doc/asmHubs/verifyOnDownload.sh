#!/bin/bash

# set -beEu -o pipefail

if [ $# -ne 2 ]; then
  printf "usage: ./verifyOnDownload.sh <host> <subset.orderList.tsv>\n" 1>&2
  printf "where <host> is something like:\n" 1>&2
  printf "api-test.gi.ucsc.edu - to use the hgwdev server\n" 1>&2
  printf "apibeta.soe.ucsc.edu - to use the hgwbeta server\n" 1>&2
  exit 255
fi

#  printf "usage: ./verifyOnDownload.sh <GCF/012/345/678/GCF_012345678.nn>\n" 1>&2
#	${toolsDir}/mkSendList.pl ${orderList} | while read F; do \
#	  ${toolsDir}/verifyOnDownload.sh $$F < /dev/null; done

export host=$1
export orderList=$2
export successCount=0
export doneCount=0

export dbHost="localhost"
export hubSource="hgdownload-test.gi.ucsc.edu"
if [ "${host}" = "apibeta.soe.ucsc.edu" ]; then
  hubSource="hgdownload.soe.ucsc.edu"
fi

for dirPath in `~/kent/src/hg/makeDb/doc/asmHubs/mkSendList.pl "${orderList}"`
do
  ((doneCount=doneCount+1))

  export genome=`basename $dirPath`

  case $genome in
     GC*)
  trackCount=`curl -L "https://$host/list/tracks?genome=$genome;trackLeavesOnly=1;hubUrl=https://$hubSource/hubs/${dirPath}/hub.txt" \
      2> /dev/null | python -mjson.tool | egrep ": {$" \
       | tr -d '"' | sed -e 's/^ \+//; s/ {//;' | xargs echo | wc -w`
  if [ "${trackCount}" -gt 14 ]; then
    ((successCount=successCount+1))
    printf "%03d\t%s\t%d tracks:\t" "${doneCount}" "${genome}" "${trackCount}"
  else
    printf "%03d\t%s\t%d (error < 15) tracks:\t" "${doneCount}" "${genome}" "${trackCount}"
  fi
  curl -L "https://$host/list/hubGenomes?hubUrl=https://$hubSource/hubs/${dirPath}/hub.txt" 2> /dev/null \
     | python -mjson.tool | egrep "organism\":|description\":" | sed -e "s/'/_/g;" \
       | tr -d '"'  | xargs echo \
          | sed -e 's/genomes: //; s/description: //; s/organism: //; s/{ //g;'
       ;;
     *)
       db=`echo $genome | tr -d '_'`
 trackCount=`curl -L "https://$host/list/tracks?genome=$db;trackLeavesOnly=1" \
           2> /dev/null | python -mjson.tool | egrep ": {$" \
               | egrep -v '"'$db'":' | tr -d '"' \
                 | sed -e 's/^ \+//; s/ {//;' | xargs echo | wc -w`
  if [ "${trackCount}" -gt 14 ]; then
    ((successCount=successCount+1))
    printf "%03d\t%s\t%d tracks:\t" "${doneCount}" "${db}" "${trackCount}"
  else
    printf "%03d\t%s\t%d (error < 15) tracks:\t" "${doneCount}" "${db}" "${trackCount}"
  fi
hgsql -N -e "select organism,description,\",\",scientificName from dbDb where name=\"$db\";" hgcentraltest | tr "'" '_' | xargs echo | sed -e 's/ ,/,/;'
       ;;
  esac

done
export failCount=`echo $doneCount $successCount | awk '{printf "%d", $1-$2}'`
printf "# checked %3d hubs, %3d success, %3d fail\n" "${doneCount}" "${successCount}" "${failCount}"
