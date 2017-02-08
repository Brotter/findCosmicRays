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

for core in `seq 0 63`; do
    nice ./clusterClusterer ${sharedDir}/${core}.root 64 $core 1> ${sharedDir}/log/${core}.log 2>&1 &
done
    