#!/bin/bash
args=("$@")
total=1 
for (( c=1; c<=$total; c++ ))
do
   ./kcolor data/DSJC500.5.txt 48 10 4000 7000 30 3 tmp
done
