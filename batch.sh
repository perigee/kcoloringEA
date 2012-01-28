#!/bin/bash
args=("$@")
total=30 
for (( c=1; c<=$total; c++ ))
do
   ./kcolor data/DSJC500.5.txt 48 10 5000 7000 30 3 >> dsjc500.5.30runs.data
done
