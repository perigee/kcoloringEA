#!/bin/bash
args=("$@")
total=1 
for (( c=1; c<=$total; c++ ))
do
   ./kcolor data/DSJC500.5.txt 47 20 5000 10000 30 3 tmp
done
