# ensc-427-wps
Wifi Positioning System for a Shopping Mall Simulation in ns-3

Authors:
Ansley Ang and Diego Flores

Each client in the network sends a ping to nearby beacons. The RTTs values retrieved are used to calculate the distance between client and beacon.
With 4 distances, we use multilateration to estimate the location of the client. 

To run our project, use the runwps.sh batch script to run the ns-3 simulation 12 times (once for each beacon). There should be 12 files output labelled "router#.txt". These files contain the RTT values for every client in the network and a timestamp. The true position of client 0 is also output to a seperate file. 

Then, run Multilateration.py to calculate the estimated positions based on the collected RTTs values for client 0. The script will generate a graph to visuallize this. 
