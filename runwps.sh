#!/bin/bash


cd ..
for i in {0..11}
do
./ns3 run "main --nWifiClient=40 --choiceServerNum=$i"
done
