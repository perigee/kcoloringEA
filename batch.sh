#!/bin/bash
args=("$@")
total=30 
for (( c=1; c<=$total; c++ ))
do
   ./kcolor data/DSJC500.5.txt 48 20 5000 10000 30 3 tmp
done
