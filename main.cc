/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/ssid.h"
#include "ns3/mesh-helper.h"
#include "ns3/mesh-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/internet-apps-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/flow-monitor-helper.h"
#include <cstdlib>
#include <fstream>
#include <string>

// comment out for testing with old 25m x 25m configuration
#define MALL_CONFIG // for the 15m x 150m mall environment
/* Ideal Network Topology
//
//   Wifi 10.1.3.0
//  AP              AP
//  *               * 
//  | \           / |  
// n1  \         /  n2  
        \  STA  /
            *
            |       
            x           x = {n0,n5,n7...}
          /     \
//  AP   /       \  AP
//  *               * 
//  |               |  
// n3               n4   
*/

/* Real Network Topology
//
//   Wifi 10.1.3.0
//  AP             
//  *               
//  | \           
// n1  \         
        \  STA  
            *
            |           Repeat simulation for AP -> n1,n2,n3,n4
            x          x = {n0,n5,n7...}    

*/
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WPS");

// Callback function that outputs the current position of a node when it reaches its next destination
void
CourseChange (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const MobilityModel> model)
{
  double timeNow = Simulator::Now().GetSeconds();
  Vector position = model->GetPosition ();
  *stream->GetStream() << std::to_string(timeNow) << "\t" << position.x << "\t" << position.y << std::endl;

}

int choiceServerNum = 0; // The beacon in which the clients will be transmitting to for this simulation

// Callback function that outputs the RTT for each client ping application
static void PingRtt (Ptr<OutputStreamWrapper> stream,std::string context, Time rtt)
{
  double timeNow = Simulator::Now().GetSeconds();
  char const* digits = "0123456789";
  int clientNodeNum = -1;
  int serverNodeNum = -1;
  int pos = 0;
  for(int i = 0; i < 2; i++) //this section is for extracting the client and beacon number
  {
      int number;
      std::size_t const n = context.find_first_of("0123456789",pos);
      if (n != std::string::npos) // position is not -1
      {
        std::size_t const m = context.find_first_not_of(digits, n);
        if(m != std::string::npos)
        {
            number = std::stoi(context.substr(n,n-m));
            pos = m;
        }
        else
        {
            number = std::stoi(context.substr(n,m));
            pos = m;
        }

        if(i == 0){clientNodeNum = number;}
        else{serverNodeNum = number;}

      }

  }
    *stream->GetStream() << std::to_string(timeNow) << "\t" << clientNodeNum << "\t"
                         << choiceServerNum << "\t" << rtt.GetNanoSeconds () << std::endl;
}

int
main(int argc, char* argv[])
{

    uint32_t nWifiClient = 1; // number of mall users

    // two command line arguements for number of clients in the network and which beacon to set position for
    CommandLine cmd(__FILE__);
    cmd.AddValue("nWifiClient", "Number of wifi STA devices (# of people in the mall)", nWifiClient);
    cmd.AddValue("choiceServerNum", "select the server AP node to have the clients ping", choiceServerNum);

    cmd.Parse(argc, argv);

    std::string logFile = "router" + std::to_string(choiceServerNum) + ".txt"; // for outputting RTT data

    LogComponentEnable("WPS", LOG_LEVEL_INFO);

    // Initiallize n amount of clients nodes and one beacon node. Store in their own containters
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(nWifiClient);
    NodeContainer wifiApNodeBeacons;
    wifiApNodeBeacons.Create(1); 
    
    // Configure a shared Wi-fi channel and install it onto the PHY
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
  
    // Configure the Wifi Mac
    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");

    // Create Device containers for STA and AP nodes
    NetDeviceContainer staDevices;
    NetDeviceContainer apDevicesBeacons;

    WifiHelper wifi; //Wifi 6, 5GHz, constant propagation delay, logdistance loss model - default settings

    // Install the PHY and MAC configurations for both the clients and beacon - Beacon is AP since it's the main server
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, wifiStaNodes);

    // Clients are not probing, Beacon is set to active probing
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevicesBeacons = wifi.Install(phy, mac, wifiApNodeBeacons);

    // Install mobility behaviour onto the nodes and starting positions
    MobilityHelper mobilityAP;
    Ptr<ListPositionAllocator> positionAllocAP = CreateObject <ListPositionAllocator>();

    double xposRout,yposRout;
#ifdef MALL_CONFIG
    // set the beacon position to it's corresponding spot, depending on which simulation you're running
    if(choiceServerNum % 2 == 0)
    {
        xposRout = 12.5+25*choiceServerNum/2;
        yposRout = 0.375;
        positionAllocAP->Add(Vector(xposRout, yposRout, 0));
    }
    else
    {
        xposRout = 12.5+25*(choiceServerNum-1)/2;
        yposRout = 11.25;
        positionAllocAP->Add(Vector(xposRout, yposRout, 0));
    }
#else
    switch(choiceServerNum) // old, set positions for 4 beacons in a 25 m x 25m room at the corners
    {
        case 0:
            positionAllocAP ->Add(Vector(12.5, 12.5, 0)); // node0
        case 1:
            positionAllocAP ->Add(Vector(-12.5, 12.5, 0)); // node1
        case 2:
            positionAllocAP ->Add(Vector(12.5, -12.5, 0)); // node2
        case 3:
            positionAllocAP ->Add(Vector(-12.5, -12.5, 0)); // node3
    }

#endif
    // set the clients to random positions in the mall
    mobilityAP.SetPositionAllocator(positionAllocAP);
    mobilityAP.SetMobilityModel("ns3::ConstantPositionMobilityModel"); // AP beacons are stationary
    mobilityAP.Install(wifiApNodeBeacons);

    MobilityHelper mobilitySTA;
    Ptr<ListPositionAllocator> positionAllocSTA = CreateObject <ListPositionAllocator>();

#ifdef MALL_CONFIG
    srand(125); //set a seed for consistency across trials

    for(uint32_t i = 0; i < nWifiClient; i++)
    {
        float xstart =150*((float) rand()/RAND_MAX);
        float ystart = 15*((float) rand()/RAND_MAX);
        positionAllocSTA ->Add(Vector(xstart, ystart, 0));
    }
    //set the type of mobility to RandomWalk2dMobility
    mobilitySTA.SetPositionAllocator(positionAllocSTA);
    for(int i = 0; i < nWifiClient;i++)
    {
        // speed values based on journal on shopping mall behaviour, direction is 360 degree range
        // stream values ensure that the same path is taken across different simulations
        std::string randSpeed = "ns3::UniformRandomVariable[Min=1.15|Max=1.65|Stream=" + std::to_string(i+1) + "]";
        std::string randDirection = "ns3::UniformRandomVariable[Min=0.0|Max=6.283184|Stream=" + std::to_string(i+1) + "]";

        mobilitySTA.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                     "Speed", StringValue(randSpeed),
                                     "Direction", StringValue(randDirection),
                                     "Bounds", StringValue("0|150|0|15")); // mall boundary
        mobilitySTA.Install(wifiStaNodes.Get(i));
    }
#else
    // old testing
    srand(100); //set a seed for consistency
    for(uint32_t i = 0; i < nWifiClient; i++)
    {
        float xstart = 25*((float) rand()/RAND_MAX - 0.5);
        float ystart = 25*((float) rand()/RAND_MAX - 0.5);
        positionAllocSTA ->Add(Vector(xstart, ystart, 0));
    }
    mobilitySTA.SetPositionAllocator(positionAllocSTA);
    mobilitySTA.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                 "Speed", StringValue("ns3::UniformRandomVariable[Min=1.15|Max=1.65]"),
                                 //"Pause", StringValue("ns3::ConstantRandomVariable[Constant=2]"),
                                 "Bounds", StringValue("-12.5|12.5|-12.5|12.5"));
    mobilitySTA.Install(wifiStaNodes);
#endif

    // Installation allows the nodes to use internet layer protocols (TCP, UDP, ARP, IPv4, etc.)
    InternetStackHelper stack;
    stack.Install(wifiApNodeBeacons);
    stack.Install(wifiStaNodes);

    // Set the IP address of all nodes to be in the same subnet
    Ipv4AddressHelper address;

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer apbeaconsInterfaces =  address.Assign(apDevicesBeacons);
    Ipv4InterfaceContainer staInterfaces =  address.Assign(staDevices);

    ApplicationContainer clientApps[nWifiClient];
    // install the V4Ping application to the client nodes - the beacon has no application but still handles the echo requests from the stack helper
    uint32_t simTime = 60;
    V4PingHelper pingServer(apbeaconsInterfaces.GetAddress (0)); // the Beacon IP address
    pingServer.SetAttribute("Interval",TimeValue(Seconds(1.))); // send a new packet every second
    //pingServer.SetAttribute("Verbose",BooleanValue(true));
    srand(100);
    for(int i = 0; i<nWifiClient;i++) // add jitter to the start times for the ping applications
    {
        clientApps[i].Add(pingServer.Install(wifiStaNodes.Get(i)));
        clientApps[i].Start(Seconds(1.0+.02*i));
        clientApps[i].Stop(Seconds(simTime));
    }

    // sets the routing path for each node - not very complex since it's only a single hop from client to beacon
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Set the simulation stop time
    Simulator::Stop(Seconds(simTime));

    // open log files for output
    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream(logFile);
    Ptr<OutputStreamWrapper> streamMobility = asciiTraceHelper.CreateFileStream("client0-pos.txt");

    // Configure callback for logging - trace sources
    Config::Connect("/NodeList/0/$ns3::MobilityModel/CourseChange",
                    MakeBoundCallback(&CourseChange, streamMobility));
    Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::V4Ping/Rtt",
                     MakeBoundCallback (&PingRtt,stream));

    // Write a header to the log files
    *stream->GetStream() << std::to_string(nWifiClient) << "\t" << std::to_string(xposRout)
                         << "\t" << std::to_string(yposRout)
                         << "\t" << std::to_string((simTime - 1)*nWifiClient) <<std::endl;

    *streamMobility->GetStream() << std::to_string(nWifiClient) 
                                 << "\t0" << "\t0" <<"\t" 
                                 << std::to_string((simTime - 1)*nWifiClient) << std::endl;
    
    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();
    std::cout << "Done Simulation with nWifiClient="
              << std::to_string(nWifiClient)
              << ", router" << std::to_string(choiceServerNum)
              << " (" << std::to_string(xposRout) << "," << std::to_string(yposRout) << ")" << std::endl;
    return 0;
}
