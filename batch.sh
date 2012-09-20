#!/bin/bash
args=("$@")
total=10 
for (( c=1; c<=$total; c++ ))
do
   ./eagraph data/dsjc1000.9.col.txt 222 1000000 >> tmpMatching.data
done
