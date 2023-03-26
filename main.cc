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
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/internet-apps-module.h"
#include "ns3/v4ping-helper.h"

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


static void PingRtt (std::string context, Time rtt)
{
  std::cout << context << "=" << rtt.GetMilliSeconds () << " ms" << std::endl;
}

int
main(int argc, char* argv[])
{
    //bool verbose = true;
    //uint32_t nCsma = 3;
    uint32_t nWifiClient = 1; // number of mall users
    std::string traceFile = "scenario1.ns_movements"; // mobility model traces
    //bool tracing = false;
    std::string logFile = "test.txt";

    CommandLine cmd(__FILE__);
    cmd.AddValue("nWifiClient", "Number of wifi STA devices (# of people in the mall)", nWifiClient);
    cmd.AddValue("traceFile","NS2 movement trace file from BonnMotion",traceFile);
    //cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    //cmd.AddValue("tracing", "Enable pcap tracing", tracing);

    cmd.Parse(argc, argv);

    if(traceFile.empty()) {
        std::cout << "Usage of " << argv[0] << ":\n" << "./waf --run \"script --traceFile=/path/to/tracefile\"\n";
    }

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
    wifiApNodeMinions.Create(4); // 4 RTT values


    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");

    WifiHelper wifi;

    NetDeviceContainer staDevices;
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, wifiStaNodes);

    NetDeviceContainer apDevicesMinions;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevicesMinions = wifi.Install(phy, mac, wifiApNodeMinions);

   // double deltaTime = 100;
    //Ptr<Node> n0 = wifiStaNodes.Get(0);
   // Ns2MobilityHelper ns2mobility = Ns2MobilityHelper(traceFile);
   // ns2mobility.Install(); //BonnMotion trace file installed to mall clients
    //Simulator::Schedule(Seconds(0.0), &showPosition, n0, deltaTime);

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    /*// old mobility
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(wifiStaNodes);*/

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodeMinions);
    mobility.Install(wifiStaNodes);

    InternetStackHelper stack;
   // stack.Install(csmaNodes);
    stack.Install(wifiApNodeMinions);
    stack.Install(wifiStaNodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer staInterfaces =  address.Assign(staDevices);
    Ipv4InterfaceContainer apminionsInterfaces =  address.Assign(apDevicesMinions);

    ApplicationContainer clientApps;


    for(int i = 0;i < 4; i++)
    {
        V4PingHelper pingServer(apminionsInterfaces.GetAddress (i));
        // install 4 ping applications to he phone client

        clientApps.Add(pingServer.Install(wifiStaNodes));
        clientApps.Start(Seconds(i));
        clientApps.Stop(Seconds(i+0.9));
    }
    //"/NodeList/[i]/ApplicationList/[i]/$ns3::V4Ping"
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(20.0));

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
    std::ofstream os;
    os.open(logFile);

    // Configure callback for logging
    //Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange",
    //                MakeBoundCallback(&CourseChange, &os));
    Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::V4Ping/Rtt",
                     MakeCallback (&PingRtt));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
