#!/bin/bash


cd ..
for i in {0..11}
do
./ns3 run "main --nWifiClient=5 --choiceServerNum=$i"
done
