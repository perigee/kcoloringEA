#!/bin/bash
args=("$@")
total=10 
for (( c=1; c<=$total; c++ ))
do
   ./eagraph data/le450_25d.txt 25 100000
done
