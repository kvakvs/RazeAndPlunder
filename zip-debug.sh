#!/bin/bash

OUTDIR=cmake-vs-debug/Debug
BWDIR=/mnt/f/Games/Starcraft/bwapi-data
rm -f rnp-debug.zip
zip -9 -D -j rnp-debug.zip ${OUTDIR}/rnpbot-client-d.exe ${BWDIR}/BWAPId.dll
