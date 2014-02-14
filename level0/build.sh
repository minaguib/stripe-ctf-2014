#!/bin/sh

set -e
CC="gcc -Wall -O3 -pipe -DBUCKETS=48000 -DBUCKET_SLOTS=15 -DBUFFLEN=4096 -DMURMUR_SEED=387"

echo Building dumper
$CC murmur3.c dumper.c -o dumper

echo Dumping
./dumper > dump.c

echo Building level0
$CC murmur3.c level0.c -o level0

echo Stripping level0
strip level0

echo Warming FS bufffer
cat level0 > /dev/null
