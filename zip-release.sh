#!/bin/bash

OUTDIR=cmake-vs-release/Release
BWDIR=/mnt/f/Games/Starcraft/bwapi-data
rm -f rnp-release.zip
zip -9 -D -j rnp-release.zip ${OUTDIR}/rnpbot-client.exe ${BWDIR}/BWAPI.dll
