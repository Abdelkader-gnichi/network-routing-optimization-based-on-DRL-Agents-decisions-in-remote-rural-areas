
#include <ns3/core-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/internet-module.h>
#include <ns3/netanim-module.h>
#include <ns3/network-module.h>
#include <ns3/ofswitch13-module.h>
#include <ns3/on-off-helper.h>
#include <ns3/packet-sink-helper.h>
#include <ns3/mobility-module.h>
#include <ns3/flow-monitor.h>
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/config-store.h"
#include "ns3/ipv4.h"
#include "ns3/header.h" 
#include "ns3/ipv4-header.h" 
#include <regex>
#include <iostream>
#include <string>
#include<tuple>
#include <ns3/udp-header.h>

#include "qos-controller.h"
#include "mygym.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("Qos");

Ptr<FlowMonitor> flowMonitor;
FlowMonitorHelper flowHelper;

typedef std::map<Mac48Address, Ipv4Address> MacIpMap_t;
MacIpMap_t mac2ipTable;

Ipv4Address ip1("192.168.1.1");
Ipv4Address ip2("192.168.1.2");
Ipv4Address ip4("192.168.1.4");
Ipv4Address ip5("192.168.1.5");
Ipv4Address ip7("192.168.1.7");

Mac48Address mac1("00:00:00:00:00:01");
Mac48Address mac2("00:00:00:00:00:07");
Mac48Address mac4("00:00:00:00:00:04");
Mac48Address mac5("00:00:00:00:00:0a");
Mac48Address mac7("00:00:00:00:00:1b");

std::pair<Mac48Address, Ipv4Address> entry1(mac1, ip1);
std::pair<Mac48Address, Ipv4Address> entry2(mac2, ip2);
std::pair<Mac48Address, Ipv4Address> entry4(mac4, ip4);
std::pair<Mac48Address, Ipv4Address> entry5(mac5, ip5);
std::pair<Mac48Address, Ipv4Address> entry7(mac7, ip7);

int
main(int argc, char* argv[])
{
    mac2ipTable.insert(entry1);
    mac2ipTable.insert(entry2);
    mac2ipTable.insert(entry4);
    mac2ipTable.insert(entry5);
    mac2ipTable.insert(entry7);
    
    // Parameters of the environment
    uint32_t simSeed = 1;
    uint16_t simTime = 100;
    double envStepTime = 1/3; //seconds, ns3gym env step time interval
    uint32_t openGymPort = 5555;
    uint32_t testArg = 0;
  
    bool verbose = true;
    bool trace = true;

    // Configure command line parameters
    CommandLine cmd;
    cmd.AddValue("verbose", "Enable verbose output", verbose);
    cmd.AddValue("trace", "Enable datapath stats and pcap traces", trace);
    // required parameters for OpenGym interface
    cmd.AddValue ("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
    cmd.AddValue("simTime", "Simulation time (seconds)", simTime);
    cmd.AddValue ("simSeed", "Seed for random generator. Default: 1", simSeed);
    // optional parameters
    cmd.AddValue ("testArg", "Extra simulation argument. Default: 0", testArg);
    cmd.Parse(argc, argv);

    if (verbose)
    {
        OFSwitch13Helper::EnableDatapathLogs();
        LogComponentEnable ("Config", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Interface", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Device", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Port", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Queue", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13SocketHandler", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Controller", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13LearningController", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13Helper", LOG_LEVEL_ALL);
        LogComponentEnable("OFSwitch13InternalHelper", LOG_LEVEL_ALL);
        LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    }
    
    Config::SetDefault("ns3::OFSwitch13Helper::ChannelType",
                       EnumValue(OFSwitch13Helper::DEDICATEDP2P));

    // Enable checksum computations (required by OFSwitch13 module)
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

    NS_LOG_UNCOND("Ns3Env parameters:");
    NS_LOG_UNCOND("--simTime: " << simTime);
    NS_LOG_UNCOND("--openGymPort: " << openGymPort);
    NS_LOG_UNCOND("--envStepTime: " << envStepTime);
    NS_LOG_UNCOND("--seed: " << simSeed);
    NS_LOG_UNCOND("--testArg: " << testArg);

    RngSeedManager::SetSeed (1);
    RngSeedManager::SetRun (simSeed);
    
    // OpenGym Env for agent 1
    uint32_t agentId = 1;
    uint32_t obsSize = 4;
    Ptr<OpenGymInterface> openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
    Ptr<MyGymEnv> myGymEnv = CreateObject<MyGymEnv> (obsSize, agentId);
    myGymEnv->SetOpenGymInterface(openGymInterface);

    // Create 7 client nodes
    NodeContainer clientNodes;
    clientNodes.Create(7);

    NodeContainer clientNodes123;
    NodeContainer clientNodes456;

    // Create 7 switch nodes
    NodeContainer switchNodes;
    switchNodes.Create(7);

    // Create the controller node
    Ptr<Node> controllerNode = CreateObject<Node>();

    // Setting node positions for NetAnim support
    Ptr<ListPositionAllocator> listPosAllocator;
    
    listPosAllocator = CreateObject<ListPositionAllocator>();
    listPosAllocator->Add(Vector(5.0,  15.0, 0));  // client 0
    listPosAllocator->Add(Vector(5.0,  30.0, 0));  // client 1
    listPosAllocator->Add(Vector(5.0,  45.0, 0));  // client 2
    listPosAllocator->Add(Vector(90.0, 15.0, 0));  // client 3
    listPosAllocator->Add(Vector(90.0, 30.0, 0));  // client 4
    listPosAllocator->Add(Vector(90.0, 45.0, 0));  // client 5
    listPosAllocator->Add(Vector(50.0, 85.0, 0));  // client 6
    listPosAllocator->Add(Vector(30.0, 15.0, 0));  // switch 0
    listPosAllocator->Add(Vector(30.0, 30.0, 0));  // switch 1
    listPosAllocator->Add(Vector(30.0, 45.0, 0));  // switch 2
    listPosAllocator->Add(Vector(65.0, 15.0, 0));  // switch 3
    listPosAllocator->Add(Vector(65.0, 30.0, 0));  // switch 4
    listPosAllocator->Add(Vector(65.0, 45.0, 0));  // switch 5
    listPosAllocator->Add(Vector(50.0, 70.0, 0));  // switch 6
    listPosAllocator->Add(Vector(100.0, 85.0, 0)); // controller Node
   
    MobilityHelper mobilityHelper;
    mobilityHelper.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityHelper.SetPositionAllocator(listPosAllocator);
    mobilityHelper.Install(NodeContainer(clientNodes, switchNodes, controllerNode));

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

    // Use the CsmaHelper to connect client nodes to the switch node
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", DataRateValue(DataRate("50Mbps")));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(0)));

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
        NetDeviceContainer link1 = csmaHelper.Install(pair1);
        switchPorts456.Add(link1.Get(0));
        clientDevices456.Add(link1.Get(1));

        NodeContainer pair2(switchNodes123.Get(i), switchNodes456.Get(i));
        NetDeviceContainer link2 = csmaHelper.Install(pair2);
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
        NetDeviceContainer link1 = csmaHelper.Install(pair1);
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
    NetDeviceContainer link1 = csmaHelper.Install(pair1);
    switchPorts123.Add(link1.Get(0));
    switchPorts7.Add(link1.Get(1)); // All Ports for switch n°7

    NodeContainer pair2(switchNodes.Get(6), switchNodes456.Get(2));
    NetDeviceContainer link2 = csmaHelper.Install(pair2);
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


    //Setting up the delay for each path between the communuties   
    switchPorts7.Get(0)->GetChannel()->SetAttribute("Delay", StringValue("0ms"));

    switchPorts3.Get(1)->GetChannel()->SetAttribute("Delay", StringValue("0.5ms"));
    // switchPorts3.Get(1)->GetChannel()->SetAttribute("DataRate",
                                                            //  DataRateValue(DataRate("1000Mbps")));
    
    switchPorts3.Get(2)->GetChannel()->SetAttribute("Delay", StringValue("10.0ms"));
    // switchPorts3.Get(2)->GetChannel()->SetAttribute("DataRate",
                                                            //  DataRateValue(DataRate("1000Mbps")));

    switchPorts3.Get(3)->GetChannel()->SetAttribute("Delay", StringValue("1.0ms"));
    // switchPorts3.Get(3)->GetChannel()->SetAttribute("DataRate",
                                                            //  DataRateValue(DataRate("1000Mbps")));

    switchPorts6.Get(2)->GetChannel()->SetAttribute("Delay", StringValue("1.0ms"));
    // switchPorts3.Get(2)->GetChannel()->SetAttribute("DataRate",
                                                            //  DataRateValue(DataRate("1000Mbps")));
    
    switchPorts6.Get(3)->GetChannel()->SetAttribute("Delay", StringValue("0.5ms"));
    // switchPorts3.Get(3)->GetChannel()->SetAttribute("DataRate",
                                                            //  DataRateValue(DataRate("1000Mbps")));

    switchPorts1.Get(1)->GetChannel()->SetAttribute("Delay", StringValue("0.5ms"));
    // switchPorts3.Get(1)->GetChannel()->SetAttribute("DataRate",
                                                            //  DataRateValue(DataRate("1000Mbps")));
    
    switchPorts1.Get(2)->GetChannel()->SetAttribute("Delay", StringValue("1.0ms"));
    // switchPorts3.Get(2)->GetChannel()->SetAttribute("DataRate",
                                                            //  DataRateValue(DataRate("1000Mbps")));

    switchPorts5.Get(1)->GetChannel()->SetAttribute("Delay", StringValue("1.0ms"));
    // switchPorts3.Get(1)->GetChannel()->SetAttribute("DataRate",
                                                            //  DataRateValue(DataRate("1000Mbps")));
    switchPorts5.Get(2)->GetChannel()->SetAttribute("Delay", StringValue("10.0ms"));
    // switchPorts3.Get(2)->GetChannel()->SetAttribute("DataRate",
                                                            //  DataRateValue(DataRate("1000Mbps")));
    

    // Configure the OpenFlow network domain
    Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper>();
    Ptr<OFSwitch13Controller> controllerApp = CreateObject<OFSwitch13Controller>();
    Ptr<QosController> qosCtrl = CreateObject<QosController>();
    controllerApp = of13Helper->InstallController(controllerNode,qosCtrl);
    

    OFSwitch13DeviceContainer ofSwitchDevices;
    ofSwitchDevices.Add(of13Helper->InstallSwitch(switchNodes123.Get(0), switchPorts1));
    ofSwitchDevices.Add(of13Helper->InstallSwitch(switchNodes123.Get(1), switchPorts2));
    ofSwitchDevices.Add(of13Helper->InstallSwitch(switchNodes123.Get(2), switchPorts3));
    ofSwitchDevices.Add(of13Helper->InstallSwitch(switchNodes456.Get(0), switchPorts4));
    ofSwitchDevices.Add(of13Helper->InstallSwitch(switchNodes456.Get(1), switchPorts5));
    ofSwitchDevices.Add(of13Helper->InstallSwitch(switchNodes456.Get(2), switchPorts6));
    ofSwitchDevices.Add(of13Helper->InstallSwitch(switchNodes.Get(6), switchPorts7));
    of13Helper->CreateOpenFlowChannels();


    //Install the TCP/IP stack into hosts nodes
    InternetStackHelper internet;
    internet.Install(clientNodes);


    // Set IPv4 host addresses
    Ipv4AddressHelper ipv4helpr;
    Ipv4InterfaceContainer clientIpIfaces123,clientIpIfaces456,clientIpIfaces7;

    ipv4helpr.SetBase("192.168.1.0", "255.255.255.0");
    clientIpIfaces123 = ipv4helpr.Assign(clientDevices123);
    clientIpIfaces456 = ipv4helpr.Assign(clientDevices456);
    clientIpIfaces7 = ipv4helpr.Assign(clientDevices7);


    //receiver
    uint16_t port = 9;
    Address sinkLocalAddress(InetSocketAddress(clientIpIfaces7.GetAddress(0), port));
    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", sinkLocalAddress);
    ApplicationContainer sinkApp = sinkHelper.Install(clientNodes.Get(6));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(simTime));

    //sender
    OnOffHelper clientHelper("ns3::UdpSocketFactory", InetSocketAddress(clientIpIfaces7.GetAddress(0),port));
    
    ApplicationContainer clientApps;
    clientApps.Add(clientHelper.Install(clientNodes123.Get(0)));
    clientApps.Start(Seconds(0.0));

    flowMonitor = flowHelper.InstallAll();
    clientApps.Stop(Seconds(simTime));

    // // Enable datapath stats and pcap traces at hosts, switch(es), and controller(s)
    // if (trace)
    // {
    //     of13Helper->EnableOpenFlowPcap("openflow");
    //     of13Helper->EnableDatapathStats("switch-stats");
    //     p2pHelper.EnablePcap("switch", switchPorts, true);
    //     p2pHelper.EnablePcap("host", hostDevices);
    // }

   
    Config::Connect("NodeList/*/$ns3::Node/$ns3::OFSwitch13Device/PortList/*/$ns3::OFSwitch13Port/SwitchPortRx"
                , MakeBoundCallback (&MyGymEnv::TracePacketReceive, myGymEnv, controllerApp,flowMonitor,flowHelper.GetClassifier()));

   
    // Run the simulation
    Simulator::Stop(Seconds(simTime + 0.02));

    //csmaHelper.EnablePcapAll("myfirst");

    AnimationInterface anim("myofswitchAnimation.xml");
   
     // Set NetAnim icon images and size
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
    {   
        // printf("Current working directory: %s\n", cwd);
        std::string path = 
        /*std::string(cwd) + */
        "/home/gadour/source/ns-3.37/contrib/ofswitch13/examples/ofswitch13-qos-controller/images/";

        uint32_t clientImg = anim.AddResource(path + "client.png");
        uint32_t switchImg = anim.AddResource(path + "switch.png");
        uint32_t controllerImg = anim.AddResource(path + "controller.png");
        int k= 0;
        for (size_t i = 0; i < clientNodes.GetN() * 2 + 1; i++)
        {

            std::ostringstream clientdesc,switchdesc;
            if (i < 7 )
            {
                clientdesc << "Client " << i;
                anim.UpdateNodeDescription(i, clientdesc.str());
                anim.UpdateNodeImage(i, clientImg);
            }
            else if (i > 6 && i < 14)
            {
                switchdesc << "Switch " << k;
                anim.UpdateNodeDescription(i, switchdesc.str());
                anim.UpdateNodeImage(i, switchImg);
                k++;
            }
            else
            {
                anim.UpdateNodeDescription(i, "Controller");
                anim.UpdateNodeImage(i, controllerImg);
            }

            anim.UpdateNodeSize(i, 10, 10);
            
        }

    }
    
    config.ConfigureAttributes ();

    Simulator::Run();
    
    flowMonitor->SerializeToXmlFile("myFlowMon.xml", false, true);
    myGymEnv->NotifySimulationEnd();
    // openGymInterface2->NotifySimulationEnd();
    Simulator::Destroy();
}
