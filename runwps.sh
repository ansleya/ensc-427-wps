#!/bin/bash


cd ..
for i in {0..11}
do
./ns3 run "main --nWifiClient=47 --choiceServerNum=$i"
done
