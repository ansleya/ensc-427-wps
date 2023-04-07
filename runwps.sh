#!/bin/bash


cd ..
for i in {0..3}
do
./ns3 run "main --nWifiClient=1 --choiceServerNum=$i"
done
