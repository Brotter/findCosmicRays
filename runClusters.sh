#!/bin/bash

################################################
#
#  Ben Rotter - September 2016
#
#  This code should run on the four ANITA clusters
#  at UH to run whatever code faster
#
#################################################



if [ `hostname | cut -d"." -f1` == "anitaI" ]; then
    startSeq=130
    stopSeq=213
elif [ `hostname | cut -d"." -f1` == "anitaII" ]; then
    startSeq=214
    stopSeq=298
elif [ `hostname | cut -d"." -f1` == "anitaIII" ]; then
    startSeq=298
    stopSeq=364
elif [ `hostname | cut -d"." -f1` == "anitaIV" ]; then
    startSeq=365
    stopSeq=439
else
    echo "The server isn't an anita cluster server, so you shouldn't use this script"
    exit
fi

for run in `seq ${startSeq} ${stopSeq}`; do
    nice ./findCosmicRays ${run} ${run} 1> log/${i}.log 2>&1 &
done
    