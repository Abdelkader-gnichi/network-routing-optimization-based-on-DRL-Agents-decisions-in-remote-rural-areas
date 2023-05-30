/*
 * Copyright (c) 2014 University of Campinas (Unicamp)
 *
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
 *
 * Author: Luciano Jerez Chaves <ljerezchaves@gmail.com>
 *         Vitor M. Eichemberger <vitor.marge@gmail.com>
 */

/*
 * Two hosts connected to a single OpenFlow switch.
 * The switch is managed by the default learning controller application.
 *
 *                       Learning Controller
 *                                |
 *                       +-----------------+
 *            Host 0 === | OpenFlow switch | === Host 1
 *                       +-----------------+
 */

//#include "ns3/point-to-point-module.h"
#include <ns3/core-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/internet-module.h>
#include <ns3/netanim-module.h>
#include <ns3/network-module.h>
#include <ns3/ofswitch13-module.h>

using namespace ns3;
using namespace std;

int
main(int argc, char* argv[])
{
    uint16_t simTime = 10;
    bool verbose = true;
    bool trace = true;

    // Configure command line parameters
    CommandLine cmd;
    cmd.AddValue("simTime", "Simulation time (seconds)", simTime);
    cmd.AddValue("verbose", "Enable verbose output", verbose);
    cmd.AddValue("trace", "Enable datapath stats and pcap traces", trace);
    cmd.Parse(argc, argv);

    if (verbose)
    {
        OFSwitch13Helper::EnableDatapathLogs();
        LogComponentEnable("OFSwitch13Interface", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Device", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Port", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Queue", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13SocketHandler", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Controller", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13LearningController", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Helper", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13InternalHelper", LOG_LEVEL_ALL);
    }
    Config::SetDefault("ns3::OFSwitch13Helper::ChannelType",
                       EnumValue(OFSwitch13Helper::DEDICATEDP2P));
    // Enable checksum computations (required by OFSwitch13 module)
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

    // Create two host nodes
    NodeContainer clientNodes;
    clientNodes.Create(7);

    NodeContainer clientNodes123;
    NodeContainer clientNodes456;

    NodeContainer switchNodes;
    switchNodes.Create(7);

    NodeContainer switchNodes123;
    NodeContainer switchNodes456;

    for (size_t i = 0; i < clientNodes.GetN() - 1; i++) // except the last one,it's the BS
    {
        if (i < 3)
        {
            clientNodes123.Add(clientNodes.Get(i));
            switchNodes123.Add(switchNodes.Get(i));
        }
        else if (i > 2)
        {
            clientNodes456.Add(clientNodes.Get(i));
            switchNodes456.Add(switchNodes.Get(i));
        }
    }

    

    // Use the CsmaHelper to connect host nodes to the switch node

    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", DataRateValue(DataRate("100Mbps")));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer clientDevices;
    NetDeviceContainer switchPorts;

    NetDeviceContainer clientDevices123;
    NetDeviceContainer switchPorts123;

    NetDeviceContainer clientDevices456;
    NetDeviceContainer switchPorts456;

   

    for (size_t i = 0; i < clientNodes123.GetN(); i++)
    {
        NodeContainer pair(clientNodes123.Get(i), switchNodes123.Get(i));
        NetDeviceContainer link = csmaHelper.Install(pair);
        clientDevices123.Add(link.Get(0));
        switchPorts123.Add(link.Get(1));

        NodeContainer pair1(switchNodes456.Get(i), clientNodes456.Get(i));
        NetDeviceContainer link1 = csmaHelper.Install(pair);
        switchPorts456.Add(link1.Get(0));
        clientDevices456.Add(link1.Get(1));

        NodeContainer pair2(switchNodes123.Get(i), switchNodes456.Get(i));
        NetDeviceContainer link2 = csmaHelper.Install(pair);
        switchPorts123.Add(link2.Get(0));
        switchPorts456.Add(link2.Get(1));
    }

    for (size_t i = 0; i < clientNodes123.GetN() - 1; i++)
    {
        NodeContainer pair(switchNodes123.Get(i), switchNodes123.Get(i + 1));
        NetDeviceContainer link = csmaHelper.Install(pair);
        switchPorts123.Add(link.Get(0));
        switchPorts123.Add(link.Get(1));

        NodeContainer pair1(switchNodes456.Get(i), switchNodes456.Get(i + 1));
        NetDeviceContainer link1 = csmaHelper.Install(pair);
        switchPorts456.Add(link1.Get(0));
        switchPorts456.Add(link1.Get(1));
    }

    NetDeviceContainer clientDevices7;
    NetDeviceContainer switchPorts7;

    NodeContainer pair(clientNodes.Get(6), switchNodes.Get(6));
    NetDeviceContainer link = csmaHelper.Install(pair);
    clientDevices7.Add(link.Get(0));
    switchPorts7.Add(link.Get(1)); // All Ports for switch n°7

    NodeContainer pair1(switchNodes123.Get(2), switchNodes.Get(6));
    NetDeviceContainer link1 = csmaHelper.Install(pair);
    switchPorts123.Add(link1.Get(0));
    switchPorts7.Add(link1.Get(1)); // All Ports for switch n°7

    NodeContainer pair2(switchNodes.Get(6), switchNodes456.Get(2));
    NetDeviceContainer link2 = csmaHelper.Install(pair);
    switchPorts7.Add(link2.Get(0)); // All Ports for switch n°7
    switchPorts456.Add(link2.Get(1));

    NetDeviceContainer switchPorts1, switchPorts2, switchPorts3, switchPorts4, switchPorts5,
        switchPorts6;

    for (size_t i = 0; i < 2; i++)
    {
        switchPorts1.Add(switchPorts123.Get(i));
        switchPorts2.Add(switchPorts123.Get(i + 2));

        switchPorts4.Add(switchPorts456.Get(i));
        switchPorts5.Add(switchPorts456.Get(i + 2));
    }
    switchPorts1.Add(switchPorts123.Get(6));

    switchPorts2.Add(switchPorts123.Get(7));
    switchPorts2.Add(switchPorts123.Get(8));

    switchPorts4.Add(switchPorts456.Get(6));

    switchPorts5.Add(switchPorts456.Get(7));
    switchPorts5.Add(switchPorts456.Get(8));

    // All Ports for switch3
    switchPorts3.Add(switchPorts123.Get(4));
    switchPorts3.Add(switchPorts123.Get(5));
    switchPorts3.Add(switchPorts123.Get(9));
    switchPorts3.Add(switchPorts123.Get(10));

    // All Ports for switch6
    switchPorts6.Add(switchPorts456.Get(4));
    switchPorts6.Add(switchPorts456.Get(5));
    switchPorts6.Add(switchPorts456.Get(9));
    switchPorts6.Add(switchPorts456.Get(10));

    cout << "this in size of AllswitchPorts123= "
         << switchPorts1.GetN() + switchPorts2.GetN() + switchPorts3.GetN() << endl;
    cout << "this in size of AllswitchPorts456= "
         << switchPorts4.GetN() + switchPorts5.GetN() + switchPorts6.GetN() << endl;
    cout << "this in size of AllswitchPorts456= " << switchPorts7.GetN() << endl;

    // Setting up the delay for each path between the communuties
    switchPorts7.Get(0)->GetChannel()->SetAttribute("Delay", StringValue("0ms"));

    cout << "switchPort3 ports= " << switchPorts3.GetN() << endl;

    switchPorts3.Get(1)->GetChannel()->SetAttribute("Delay", StringValue("0.5ms"));
    switchPorts3.Get(2)->GetChannel()->SetAttribute("Delay", StringValue("10.0ms"));
    switchPorts3.Get(3)->GetChannel()->SetAttribute("Delay", StringValue("1.0ms"));

    switchPorts6.Get(2)->GetChannel()->SetAttribute("Delay", StringValue("1.0ms"));
    switchPorts6.Get(3)->GetChannel()->SetAttribute("Delay", StringValue("0.5ms"));

    switchPorts1.Get(1)->GetChannel()->SetAttribute("Delay", StringValue("0.5ms"));
    switchPorts1.Get(2)->GetChannel()->SetAttribute("Delay", StringValue("1.0ms"));

    switchPorts5.Get(1)->GetChannel()->SetAttribute("Delay", StringValue("1.0ms"));
    switchPorts5.Get(2)->GetChannel()->SetAttribute("Delay", StringValue("10.0ms"));

    TimeValue dataRateValue;
    Time dataRate;
    switchPorts4.Get(2)->GetChannel()->GetAttribute("Delay", dataRateValue);
    dataRate = dataRateValue.Get();
    cout << "this is the NEW switchPort4-5 Delay= " << dataRate << endl;

    
   

    // Create the controller node
    Ptr<Node> controllerNode = CreateObject<Node>();

    // Configure the OpenFlow network domain
    Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper>();
    of13Helper->InstallController(controllerNode);
    of13Helper->InstallSwitch(switchNodes123.Get(0), switchPorts1);
    of13Helper->InstallSwitch(switchNodes123.Get(1), switchPorts2);
    of13Helper->InstallSwitch(switchNodes123.Get(2), switchPorts3);
    of13Helper->InstallSwitch(switchNodes456.Get(0), switchPorts4);
    of13Helper->InstallSwitch(switchNodes456.Get(1), switchPorts5);
    of13Helper->InstallSwitch(switchNodes456.Get(2), switchPorts6);
    of13Helper->InstallSwitch(switchNodes.Get(6), switchPorts7);
    of13Helper->CreateOpenFlowChannels();
    //of13Helper->SetChannelType()
    // Configure the OpenFlow network domain

    cout << "*****************************************************" << endl;
    cout << "controllerNode Size= " << controllerNode->GetNDevices() << endl;
    cout << "clientNodes Size= " << clientNodes.GetN() << endl;
    cout << "*****************************************************" << endl;
    //Install the TCP/IP stack into hosts nodes
    InternetStackHelper internet;
    internet.Install(clientNodes);

    cout << "clientDevices123 Size= " << clientDevices123.GetN() << endl;
    cout << "clientDevices456 Size= " << clientDevices456.GetN() << endl;
    cout << "clientDevices7 Size= " << clientDevices7.GetN() << endl;

    // Set IPv4 host addresses
    Ipv4AddressHelper ipv4helpr;
    Ipv4InterfaceContainer clientIpIfaces123,clientIpIfaces456,clientIpIfaces7;
    ipv4helpr.SetBase("10.1.1.0", "255.255.255.0");
    clientIpIfaces123 = ipv4helpr.Assign(clientDevices123);
    clientIpIfaces456 = ipv4helpr.Assign(clientDevices456);
    clientIpIfaces7 = ipv4helpr.Assign(clientDevices7);
    // Configure ping application between hosts
    V4PingHelper pingHelper = V4PingHelper(clientIpIfaces7.GetAddress(0));
    pingHelper.SetAttribute("Verbose", BooleanValue(true));
    ApplicationContainer pingApps = pingHelper.Install(clientNodes123.Get(0));
    pingApps.Start(Seconds(0));

    // // Enable datapath stats and pcap traces at hosts, switch(es), and controller(s)
    // if (trace)
    // {
    //     of13Helper->EnableOpenFlowPcap("openflow");
    //     of13Helper->EnableDatapathStats("switch-stats");
    //     p2pHelper.EnablePcap("switch", switchPorts, true);
    //     p2pHelper.EnablePcap("host", hostDevices);
    // }
    
    // Run the simulation
    Simulator::Stop(Seconds(simTime));
    AnimationInterface anim("animation.xml");
    anim.SetConstantPosition(clientNodes.Get(0),25.0,15.0);

    anim.UpdateNodeDescription(clientNodes.Get(0), "client 0");
    
    anim.SetConstantPosition(clientNodes.Get(1),25.0,30.0);
    anim.SetConstantPosition(clientNodes.Get(2),25.0,45.0);
    anim.SetConstantPosition(clientNodes.Get(3),75.0,15.0);
    anim.SetConstantPosition(clientNodes.Get(4),75.0,30.0);
    anim.SetConstantPosition(clientNodes.Get(5),75.0,45.0);
    anim.SetConstantPosition(clientNodes.Get(6),50.0,50.0);
    

    anim.SetConstantPosition(switchNodes.Get(0),35.0,15.0);

    anim.UpdateNodeDescription(switchNodes.Get(0), "switch 0");

    anim.SetConstantPosition(switchNodes.Get(1),35.0,30.0);
    anim.SetConstantPosition(switchNodes.Get(2),35.0,45.0);
    anim.SetConstantPosition(switchNodes.Get(3),85.0,15.0);
    anim.SetConstantPosition(switchNodes.Get(4),85.0,30.0);
    anim.SetConstantPosition(switchNodes.Get(5),85.0,45.0);
    anim.SetConstantPosition(switchNodes.Get(6),50.0,30.0);

    anim.SetConstantPosition(controllerNode,100.0,80.0);
  

    // anim.SetConstantPosition(switchNode,30.0,50.0);
    // anim.SetConstantPosition(controllerNode,30.0,20);
    // anim.UpdateNodeDescription(hosts.Get(0), "Host 0");
    // anim.UpdateNodeDescription(hosts.Get(1), "Host 1");
    // anim.UpdateNodeDescription(switchNode, "switch");
    // anim.UpdateNodeSize(hosts.Get(0)->GetId(),4.0,4.0);
    // anim.UpdateNodeDescription(controllerNode, "Controller");
    //csmaHelper.EnablePcapAll("myfirst");
    
    
    Simulator::Run();
    Simulator::Destroy();
}

// for (size_t i = 0; i < hosts.GetN(); i++)
// {
//     NodeContainer pair(hosts.Get(i), switchNode);
//     NetDeviceContainer link = p2pHelper.Install(pair);
//     hostDevices.Add(link.Get(0));
//     switchPorts.Add(link.Get(1));
// }