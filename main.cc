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

#include <cstdlib>
#include <fstream>
#include <string>

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

/*(void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  NS_LOG_UNCOND (context <<
    " x = " << position.x << ", y = " << position.y);
}*/
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

    WifiHelper wifi;


    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, wifiStaNodes);

    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevicesMinions = wifi.Install(phy, mac, wifiApNodeMinions);

/*

    MeshHelper meshBeacons = MeshHelper::Default();
    meshBeacons.SetStackInstaller("ns3::Dot11sStack");

    meshBeacons.SetSpreadInterfaceChannels(MeshHelper::ZERO_CHANNEL);
    meshBeacons.SetMacType("RandomStart", TimeValue(Seconds(1.0)));
    // Set number of interfaces - default is single-interface mesh point
    meshBeacons.SetNumberOfInterfaces(4);

    apDevicesMinions = meshBeacons.Install(phy, wifiApNodeMinions);
*/
    //mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    //apDevicesMinions = wifi.Install(phy, mac, wifiApNodeMinions);

   // double deltaTime = 100;
    //Ptr<Node> n0 = wifiStaNodes.Get(0);
   // Ns2MobilityHelper ns2mobility = Ns2MobilityHelper(traceFile);
   // ns2mobility.Install(); //BonnMotion trace file installed to mall clients
    //Simulator::Schedule(Seconds(0.0), &showPosition, n0, deltaTime);

    MobilityHelper mobilityAP;
    Ptr<ListPositionAllocator> positionAllocAP = CreateObject <ListPositionAllocator>();
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
    mobilityAP.SetPositionAllocator(positionAllocAP);
    mobilityAP.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAP.Install(wifiApNodeMinions);


    MobilityHelper mobilitySTA;
    Ptr<ListPositionAllocator> positionAllocSTA = CreateObject <ListPositionAllocator>();
    if (nWifiClient > 1)
    {
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
    }
    else // if there's only 1 node, place in the middle and keep constant
    {
        positionAllocSTA ->Add(Vector(0, 0, 0));
        mobilitySTA.SetPositionAllocator(positionAllocSTA);
        mobilitySTA.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobilitySTA.Install(wifiStaNodes);
    }

    InternetStackHelper stack;
   // stack.Install(csmaNodes);
    stack.Install(wifiApNodeMinions);
    stack.Install(wifiStaNodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer apminionsInterfaces =  address.Assign(apDevicesMinions);
    Ipv4InterfaceContainer staInterfaces =  address.Assign(staDevices);

    ApplicationContainer clientApps;


    //for(int i = 0;i < 4; i++)
    //{
        V4PingHelper pingServer(apminionsInterfaces.GetAddress (0));
        // install 4 ping applications to he phone client
        pingServer.SetAttribute("Interval",TimeValue(Seconds(1.)));
//        pingServer.SetAttribute("Verbose",BooleanValue(true));
        clientApps = pingServer.Install(wifiStaNodes);
        clientApps.Start(Seconds(1.0));
        clientApps.Stop(Seconds(60.0));
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

    // Configure callback for logging
    //Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange",
    //                MakeBoundCallback(&CourseChange, &os));
    Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::V4Ping/Rtt",
                     MakeBoundCallback (&PingRtt,stream));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
