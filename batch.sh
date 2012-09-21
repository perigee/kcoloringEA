#!/bin/bash
args=("$@")
total=10 
for (( c=1; c<=$total; c++ ))
do
   ./eagraph test25.col 25 100000 >> tmpLe450_25d.data
done
