#!/bin/bash

################################################
#
#  Ben Rotter - September 2016
#
#  This code should run on the four ANITA clusters
#  at UH to run whatever code faster
#
#################################################

startTime=`date +%m.%d.%y_%Hh`
sharedDir="/home/brotter/nfsShared/results/clusterClusterer/"${startTime}
mkdir ${sharedDir}
mkdir ${sharedDir}"/log"

inFile="/home/brotter/nfsShared/results/clusterClusterer/fisherCut.root"

if [ `hostname | cut -d"." -f1` == "anitaI" ]; then
    host=0
elif [ `hostname | cut -d"." -f1` == "anitaII" ]; then
    host=1
elif [ `hostname | cut -d"." -f1` == "anitaIII" ]; then
    host=2
elif [ `hostname | cut -d"." -f1` == "anitaIV" ]; then
    host=3
else
    echo "The server isn't an anita cluster server, so you shouldn't use this script"
    exit
fi


startSeq=$((host*64))
stopSeq=$(((host+1)*64-1))

for core in `seq ${startSeq} ${stopSeq}`; do
    nice ./clusterClusterer ${sharedDir}/${core}".root" 256 $core 1> ${sharedDir}/log/${core}.log 2>&1 &
done
    