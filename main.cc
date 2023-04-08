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
//#include "ns3/point-to-point-module.h"
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

// comment out for testing
#define MALL_CONFIG
// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");
/*
int numPacketDrop = 0;
static void
RxDrop(Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p)
{
    double timeNow = Simulator::Now().GetSeconds();
    numPacketDrop++;
    *stream->GetStream() << std::to_string(timeNow) << "\t"
                         << context << "\t"
                         << std::to_string(numPacketDrop)
                         << std::endl;
}*/

void
CourseChange (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const MobilityModel> model)
{
  double timeNow = Simulator::Now().GetSeconds();
  Vector position = model->GetPosition ();
  *stream->GetStream() << std::to_string(timeNow) << "\t" << position.x << "\t" << position.y << std::endl;

}
int choiceServerNum = 0;

static void PingRtt (Ptr<OutputStreamWrapper> stream,std::string context, Time rtt)
{
  double timeNow = Simulator::Now().GetSeconds();
  char const* digits = "0123456789";
  int clientNodeNum = -1;
  int serverNodeNum = -1;
  int pos = 0;
  for(int i = 0; i < 2; i++)
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
    //std::cout << context << "=" << rtt.GetNanoSeconds () << " ns" << std::endl;
    *stream->GetStream() << std::to_string(timeNow) << "\t" << clientNodeNum << "\t"
                         << choiceServerNum << "\t" << rtt.GetNanoSeconds () << std::endl;
}

int
main(int argc, char* argv[])
{
    //bool verbose = true;
    //uint32_t nCsma = 3;
    uint32_t nWifiClient = 1; // number of mall users
    std::string traceFile = "scenario1.ns_movements"; // mobility model traces
    //bool tracing = false;


    CommandLine cmd(__FILE__);
    cmd.AddValue("nWifiClient", "Number of wifi STA devices (# of people in the mall)", nWifiClient);
    cmd.AddValue("choiceServerNum", "select the server AP node to have the clients ping", choiceServerNum);
    //cmd.AddValue("traceFile","NS2 movement trace file from BonnMotion",traceFile);
    //cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    //cmd.AddValue("tracing", "Enable pcap tracing", tracing);

    cmd.Parse(argc, argv);

    if(traceFile.empty()) {
        std::cout << "Usage of " << argv[0] << ":\n" << "./waf --run \"script --traceFile=/path/to/tracefile\"\n";
    }

    std::string logFile = "router" + std::to_string(choiceServerNum) + ".txt";


    // The underlying restriction of 18 is due to the grid position
    // allocator's configuration; the grid layout will exceed the
    // bounding box if more than 18 nodes are provided.
    /*if (nWifiClient > 18)
    {
        std::cout << "nWifiClient should be 18 or less; otherwise grid layout exceeds the bounding box"
                  << std::endl;
        return 1;
    }

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
    */

    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(nWifiClient);
    NodeContainer wifiApNodeMinions;
    wifiApNodeMinions.Create(1); // 4 RTT values


    //Config::SetDefault("ns3::WifiPhy::ChannelNumber",UintegerValue(4))
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    //YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    //phy.Set("ChannelNumber",UintegerValue(4));

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");
    NetDeviceContainer staDevices;
    NetDeviceContainer apDevicesMinions;

    WifiHelper wifi; //Wifi 6, 5GHz, constant propagation delay, Friis loss model


    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, wifiStaNodes);

    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevicesMinions = wifi.Install(phy, mac, wifiApNodeMinions);

    MobilityHelper mobilityAP;
    Ptr<ListPositionAllocator> positionAllocAP = CreateObject <ListPositionAllocator>();

    double xposRout,yposRout;
#ifdef MALL_CONFIG
    if(choiceServerNum % 2 == 0)
    {
        xposRout = 2.5+25*choiceServerNum/2;
        yposRout = 0.375;
        positionAllocAP->Add(Vector(xposRout, yposRout, 0));
    }
    else
    {
        xposRout = 212.5+25*(choiceServerNum-1)/2;
        yposRout = 11.25;
        positionAllocAP->Add(Vector(xposRout, yposRout, 0));
    }
#else
    switch(choiceServerNum)
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

    mobilityAP.SetPositionAllocator(positionAllocAP);
    mobilityAP.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAP.Install(wifiApNodeMinions);

    MobilityHelper mobilitySTA;
    Ptr<ListPositionAllocator> positionAllocSTA = CreateObject <ListPositionAllocator>();

#ifdef MALL_CONFIG
    srand(100); //set a seed for consistency
    for(uint32_t i = 0; i < nWifiClient; i++)
    {
        float xstart =150*((float) rand()/RAND_MAX);
        float ystart = 15*((float) rand()/RAND_MAX);
        positionAllocSTA ->Add(Vector(xstart, ystart, 0));
    }
    mobilitySTA.SetPositionAllocator(positionAllocSTA);
    mobilitySTA.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                 "Speed", StringValue("ns3::UniformRandomVariable[Min=1.15|Max=1.65]"),
                                 //"Pause", StringValue("ns3::ConstantRandomVariable[Constant=2]"),
                                 "Bounds", StringValue("0|150|0|15"));
    mobilitySTA.Install(wifiStaNodes);
#else
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

    InternetStackHelper stack;
   // stack.Install(csmaNodes);
    stack.Install(wifiApNodeMinions);
    stack.Install(wifiStaNodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer apminionsInterfaces =  address.Assign(apDevicesMinions);
    Ipv4InterfaceContainer staInterfaces =  address.Assign(staDevices);

    ApplicationContainer clientApps[nWifiClient];


    //for(int i = 0;i < 4; i++)
    //{
    V4PingHelper pingServer(apminionsInterfaces.GetAddress (0));
    // install 4 ping applications to he phone client
    pingServer.SetAttribute("Interval",TimeValue(Seconds(1.)));
    //pingServer.SetAttribute("Verbose",BooleanValue(true));
    srand(100);
    for(int i = 0; i<nWifiClient;i++)
    {
        clientApps[i].Add(pingServer.Install(wifiStaNodes.Get(i)));
        //float timeJitter = 0.8*((float) rand()/RAND_MAX);
        clientApps[i].Start(Seconds(1.0+.05*i));
        //clientApps[i].Start(Seconds(1.0+timeJitter));
        clientApps[i].Stop(Seconds(60.0));
    }
    //}
    //"/NodeList/[i]/ApplicationList/[i]/$ns3::V4Ping"
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(60.0));

    /* old trace settings
    if (tracing)
    {
        phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        pointToPoint.EnablePcapAll("third");
        phy.EnablePcap("third", apDevices.Get(0));
        csma.EnablePcap("third", csmaDevices.Get(0), true);
    }

    std::ostringstream oss;
    oss <<
      "/NodeList/" << wifiStaNodes.Get (nWifiClient - 1)->GetId () <<
      "/$ns3::MobilityModel/CourseChange";

    Config::Connect (oss.str (), MakeCallback (&CourseChange));*/

    // open log file for output
    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream(logFile);
    //Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream("test.txt");
    Ptr<OutputStreamWrapper> streamMobility = asciiTraceHelper.CreateFileStream("client0-pos.txt");

    //std::string logFileDrops = "droppedPackets" + std::to_string(choiceServerNum);
    //Ptr<OutputStreamWrapper> streamPhyDrop = asciiTraceHelper.CreateFileStream(logFileDrops);

    // Configure callback for logging
    Config::Connect("/NodeList/0/$ns3::MobilityModel/CourseChange",
                    MakeBoundCallback(&CourseChange, streamMobility));
    Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::V4Ping/Rtt",
                     MakeBoundCallback (&PingRtt,stream));

    //std::string packetDropContext = "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop";
    //Config::Connect(packetDropContext,
    //                MakeBoundCallback(&RxDrop, streamPhyDrop));
    *stream->GetStream() << std::to_string(nWifiClient) << "\t" << std::to_string(xposRout)
                         << "\t" << std::to_string(yposRout)
                         << "\t0" <<std::endl;
    *streamMobility->GetStream() << std::to_string(nWifiClient) << "\t0" << "\t0" <<"\t0" << std::endl;
    //FlowMonitorHelper flowHelper;
    //Ptr<FlowMonitor> flowMonitor = flowHelper. InstallAll();
    //*streamPhyDrop->GetStream() << std::to_string(nWifiClient) << std::endl;
    Simulator::Run();
   // flowMonitor->SerializeToXmlFile("flowmonitor.xml",true,true);
    Simulator::Destroy();
    std::cout << "Done Simulation with nWifiClient="
              << std::to_string(nWifiClient)
              << ", router" << std::to_string(choiceServerNum)
              << " (" << std::to_string(xposRout) << "," << std::to_string(yposRout) << ")" << std::endl;
    return 0;
}
