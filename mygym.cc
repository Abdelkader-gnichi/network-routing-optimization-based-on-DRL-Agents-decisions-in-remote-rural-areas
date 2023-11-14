/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Technische Universität Berlin
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
 * Author: Piotr Gawlowicz <gawlowicz@tkn.tu-berlin.de>
 */

#include "mygym.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/node-list.h"
#include "ns3/log.h"
#include <sstream>
#include <iostream>

namespace ns3 {

using namespace std;
NS_LOG_COMPONENT_DEFINE ("MyGymEnv");

NS_OBJECT_ENSURE_REGISTERED (MyGymEnv);

MyGymEnv::MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
  m_observation_size = 4;
  // m_channelOccupation.clear();
  // Simulator::Schedule (Seconds(1.0153), &MyGymEnv::ScheduleNextStateRead, this);
}

MyGymEnv::MyGymEnv (uint32_t obsSize, uint32_t id)
{
  NS_LOG_FUNCTION (this);
  m_agentId = id;
  m_observation_size = obsSize;
  // m_channelOccupation.clear();
  // Simulator::Schedule (Seconds(1.0153), &MyGymEnv::ScheduleNextStateRead, this);
}


void
MyGymEnv::ScheduleNextStateRead ()
{
  NS_LOG_FUNCTION (this);
  Simulator::Schedule (m_interval, [&]() { Notify(); });
  
}


MyGymEnv::~MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
MyGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyGymEnv")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGym")
    .AddConstructor<MyGymEnv> ()
  ;
  return tid;
}

void
MyGymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<OpenGymSpace>
MyGymEnv::GetActionSpace()
{
  NS_LOG_FUNCTION (this);
 
  Ptr<OpenGymDiscreteSpace> space = CreateObject<OpenGymDiscreteSpace> (3);
  NS_LOG_UNCOND ("AgendId: "<< m_agentId << " GetActionSpace: " << space);
  return space;
}

Ptr<OpenGymSpace>
MyGymEnv::GetObservationSpace()
{
  NS_LOG_FUNCTION (this);
  float low = 0.0;
  float high = 100.0;
  std::vector<uint32_t> shape = {m_observation_size+1,};
  std::string dtype = TypeNameGet<uint32_t> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("AgendId: "<< m_agentId << " GetObservationSpace: " << space);
  return space;
}

bool
MyGymEnv::GetGameOver()
{
  NS_LOG_FUNCTION (this);
  bool isGameOver = false;
  NS_LOG_UNCOND ("AgendId: "<< m_agentId << " MyGetGameOver: " << isGameOver);
  return isGameOver;
}

Ptr<OpenGymDataContainer>
MyGymEnv::GetObservation()
{
  NS_LOG_FUNCTION (this);
  std::vector<uint32_t> shape = {m_observation_size+1,};
  Ptr<OpenGymBoxContainer<uint32_t> > box = CreateObject<OpenGymBoxContainer<uint32_t> >(shape);
  
  m_currentSwitchId = m_currentObs.at(1); //taking the current switch id
  m_pIn.at(m_currentSwitchId - 1) = m_currentObs.at(5); //taking the input port of current switch id
  m_currentFlowid = m_currentObs.at(0);
  // m_currentdelaySum = m_currentObs.at(4);

  cout << "Current SwitchId= " << m_currentSwitchId << endl;
  for (uint32_t i = 0; i < m_currentObs.size()-1; ++i) {
    uint32_t value = m_currentObs.at(i);
    box->AddValue(value);
  }
  
  NS_LOG_UNCOND ("AgendId: "<< m_agentId << " MyGetObservation: " << box);
  return box;
}

float
MyGymEnv::GetReward()
{
  NS_LOG_FUNCTION (this);
  static std::vector<float> lastValues = {0.0, 0.0, 0.0, 0.0};
  float t_Rx = 0;
  float reward= 1;
 
  switch (m_currentFlowid)
  {
  case 15:
    
    t_Rx = m_RxPackets.at(0) - lastValues.at(0);
    lastValues.at(0) = m_RxPackets.at(0);
    
    if ( (m_delaySum.at(0) > MilliSeconds(10).GetMilliSeconds()  && t_Rx == 0) )
    {
       reward = 0;
  
    } else if (t_Rx == 0) {

        reward = 0;
  
    }

    break;
  case 16:

    t_Rx = m_RxPackets.at(1) - lastValues.at(1);
    lastValues.at(1) = m_RxPackets.at(1);
    
    if ( (m_delaySum.at(1) > MilliSeconds(7).GetMilliSeconds()  && t_Rx == 0) )
    {
       reward = 0;
  
    } else if (t_Rx == 0) {

        reward = 0;
  
    }

    break;
  case 17:
    
    t_Rx = m_RxPackets.at(2) - lastValues.at(2);
    lastValues.at(2) = m_RxPackets.at(2);
    
    if ( (m_delaySum.at(2) > MilliSeconds(50).GetMilliSeconds()  && t_Rx == 0) )
    {
       reward = 0;
  
    } else if (t_Rx == 0) {

        reward = 0;
  
    }

    break;
  case 18:
       
    t_Rx = m_RxPackets.at(3) - lastValues.at(3);
    lastValues.at(3) = m_RxPackets.at(3);
    
    if ( (m_delaySum.at(3) > MilliSeconds(3).GetMilliSeconds()  && t_Rx == 0) )
    {
       reward = 0;
  
    } else if (t_Rx == 0) {

        reward = 0;
  
    }

    break;
  default:
    break;
  }
 
  NS_LOG_UNCOND ("AgendId: "<< m_agentId << " MyGetReward: " << reward);
  return reward;
}

string
MyGymEnv::GetExtraInfo()
{
  NS_LOG_FUNCTION (this);
  
  string myInfo = to_string(m_TxPackets);
  NS_LOG_UNCOND("AgendId: "<< m_agentId << " MyGetExtraInfo: " << myInfo);
  return myInfo;
}

bool
MyGymEnv::ExecuteActions(Ptr<OpenGymDataContainer> action)
{
  NS_LOG_FUNCTION (this);

  Ptr<OpenGymDiscreteContainer> discrete = DynamicCast<OpenGymDiscreteContainer>(action);
  cout <<"AgendId: "<< m_agentId << " Executed Action: "<< discrete->GetValue() + 2  << endl;
  m_pOut.at(m_currentSwitchId - 1) = discrete->GetValue() + 2 ; // Pout must be <2-3-4>
  
  string table;
  string srcIp;
  
  switch (m_currentFlowid)
  {
  case 15:
    table = "table=1";
    srcIp = "192.168.1.1";
    break;
  case 16:
    table = "table=2";
    srcIp = "192.168.1.2";
    break;
  
  case 17:
    table = "table=3";
    srcIp = "192.168.1.4";
    break;
  
  case 18:
    table = "table=4";
    srcIp = "192.168.1.5";
    break;
  default:
    cout << "there is no flow to redirect it !" << endl;
    break;
  }

  string portNum;

  switch (m_currentSwitchId) {
        case 1:

            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");

            if(m_pOut.at(m_currentSwitchId - 1) == 4)
            {
              m_pOut.at(m_currentSwitchId - 1) = 3;
            }
            
            if (m_pIn.at(m_currentSwitchId - 1) == m_pOut.at(m_currentSwitchId - 1) && m_pOut.at(m_currentSwitchId - 1) == 2 )
            {
              m_pOut.at(m_currentSwitchId - 1) = 3;
            
            }  
            
            
            portNum = to_string(m_pOut.at(m_currentSwitchId - 1));
            
            m_ControllerApp->DpctlExecute(m_currentSwitchId,
             "flow-mod cmd=add,"+table+",prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src="+srcIp+",ip_dst=192.168.1.7 write:output=" 
             + portNum);

            cout << "Switch 1 is Configured with Pout= " + portNum << endl;
            break;
        case 2:
            
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            
            

            if(m_pIn.at(m_currentSwitchId - 1) == m_pOut.at(m_currentSwitchId - 1) && m_pIn.at(m_currentSwitchId - 1) == 3) 
            {
              m_pOut.at(m_currentSwitchId - 1) = 2;
            }

            portNum = to_string(m_pOut.at(m_currentSwitchId - 1));
            
            m_ControllerApp->DpctlExecute(m_currentSwitchId,
             "flow-mod cmd=add,"+table+",prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src="+srcIp+",ip_dst=192.168.1.7 write:output=" 
             + portNum);
             
            cout << "Switch 2 is Configured with Pout= " + portNum << endl;
            break;
        case 3:
            
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");

             if (m_pIn.at(m_currentSwitchId - 1) == m_pOut.at(m_currentSwitchId - 1) && m_pOut.at(m_currentSwitchId - 1) == 2 )
            {
              m_pOut.at(m_currentSwitchId - 1) = 4;

            }  else if(m_pIn.at(m_currentSwitchId - 1) == m_pOut.at(m_currentSwitchId - 1) && m_pOut.at(m_currentSwitchId - 1) == 3) {
              
              m_pOut.at(m_currentSwitchId - 1) = 4;
            
            }
                                                
            portNum = to_string(m_pOut.at(m_currentSwitchId - 1));
            
            m_ControllerApp->DpctlExecute(m_currentSwitchId,
             "flow-mod cmd=add,"+table+",prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src="+srcIp+",ip_dst=192.168.1.7 write:output=" 
             + portNum);
             
            cout << "Switch 3 is Configured with Pout= " + portNum << endl;
            break;
        case 4:
            
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");

            if(m_pOut.at(m_currentSwitchId - 1) == 4)
            {
              m_pOut.at(m_currentSwitchId - 1) = 3;
            }
            
            if (m_pIn.at(m_currentSwitchId - 1) == m_pOut.at(m_currentSwitchId - 1) && m_pIn.at(m_currentSwitchId - 1) == 2 )
            {
              m_pOut.at(m_currentSwitchId - 1) = 2; //3

            }
            
            portNum = to_string(m_pOut.at(m_currentSwitchId - 1));
            
            m_ControllerApp->DpctlExecute(m_currentSwitchId,
             "flow-mod cmd=add,"+table+",prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src="+srcIp+",ip_dst=192.168.1.7 write:output=" 
             + portNum);
             
            cout << "Switch 4 is Configured with Pout= " + portNum << endl;
            break;
        case 5:

            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
                
             if(m_pIn.at(m_currentSwitchId - 1) == m_pOut.at(m_currentSwitchId - 1) && m_pOut.at(m_currentSwitchId - 1) == 3)
            {
              m_pOut.at(m_currentSwitchId - 1) = 4;
            
            }  else if (m_pIn.at(m_currentSwitchId - 1) == m_pOut.at(m_currentSwitchId - 1) && m_pIn.at(m_currentSwitchId - 1) == 2 ) {
              
              m_pOut.at(m_currentSwitchId - 1) = 4;
            
            }  
           
            portNum = to_string(m_pOut.at(m_currentSwitchId - 1));
            
            m_ControllerApp->DpctlExecute(m_currentSwitchId,
             "flow-mod cmd=add,"+table+",prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src="+srcIp+",ip_dst=192.168.1.7 write:output=" 
             + portNum);
             
            cout << "Switch 5 is Configured with Pout= " + portNum << endl;
            break;
        case 6:

            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");

             if (m_pIn.at(m_currentSwitchId - 1) == m_pOut.at(m_currentSwitchId - 1) && m_pIn.at(m_currentSwitchId - 1) == 2 )
            {
              m_pOut.at(m_currentSwitchId - 1) = 4;
            
            }  else if(m_pIn.at(m_currentSwitchId - 1) == m_pOut.at(m_currentSwitchId - 1) && m_pIn.at(m_currentSwitchId - 1) == 3) {
              
              m_pOut.at(m_currentSwitchId - 1) = 4;
            
            }  
           
            portNum = to_string(m_pOut.at(m_currentSwitchId - 1));
            
            m_ControllerApp->DpctlExecute(m_currentSwitchId,
             "flow-mod cmd=add,"+table+",prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src="+srcIp+",ip_dst=192.168.1.7 write:output=" 
             + portNum);
             
            cout << "Switch 6 is Configured with Pout= " + portNum << endl;
            break;
        case 7:
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            m_ControllerApp->DpctlExecute(m_currentSwitchId,"flow-mod cmd=del,"+table+",prio=200");
            // portNum = to_string(m_pOut.at(m_currentSwitchId - 1));

            m_pOut.at(m_currentSwitchId - 1) = 1;

            m_ControllerApp->DpctlExecute(m_currentSwitchId,
             "flow-mod cmd=add,"+table+",prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src="+srcIp+",ip_dst=192.168.1.7 write:output=1"); 
            //  + portNum);

            cout << "By defualt is Configured to take Pout= 1" << endl;
            break;
        default:
            cout << "Action Execution on Switches are Done" << endl;
            break;
    }
  
  return true;
}

uint32_t
MyGymEnv::rxCounter(std::vector<int64_t> rx, uint32_t currentFlowId)
{
  static std::vector<float> lastValues = {0.0, 0.0, 0.0, 0.0};
  float t_Rx = 0;
  switch (currentFlowId)
  {
  case 15:

    t_Rx = rx.at(0) - lastValues.at(0);
    lastValues.at(0) = rx.at(0);

    if(t_Rx != 0)
    {
      m_isRxedNow = 1;
    } else {
      m_isRxedNow = 0;
    }

    break;
  case 16:

    t_Rx = rx.at(1) - lastValues.at(1);
    lastValues.at(1) = rx.at(1);

    if(t_Rx != 0)
    {
      m_isRxedNow = 1;
    } else {
      m_isRxedNow = 0;
    }

    break;
  case 17:

    t_Rx = rx.at(2) - lastValues.at(2);
    lastValues.at(2) = rx.at(2);

    if(t_Rx != 0)
    {
      m_isRxedNow = 1;
    } else {
      m_isRxedNow = 0;
    }

    break;
  case 18:
       
    t_Rx = rx.at(3) - lastValues.at(3);
    lastValues.at(3) = rx.at(3);

    if(t_Rx != 0)
    {
      m_isRxedNow = 1;
    } else {
      m_isRxedNow = 0;
    }

    break;
  default:
    break;
  }
  return m_isRxedNow;
}


void
MyGymEnv::TracePacketReceive (Ptr<MyGymEnv> entity,
          Ptr<OFSwitch13Controller> ControllerApp,Ptr<ns3::FlowMonitor> flowMonitor,
                                  Ptr<ns3::FlowClassifier> flowClassifier, string context, Ptr<const Packet> packet)
{

    flowMonitor->SerializeToXmlFile("myFlowMon.xml", false, true);
    regex pattern("[0-9]+");
    cout << context << endl;
    smatch sm;
    regex_search(context, sm, pattern);

    regex pattern1("PortList\\/(1|2|3)\\/");   
    smatch sm1;
    regex_search(context, sm1, pattern1);
     
    
    cout << "SwitchId " << stoi(sm.str()) - 6 <<  endl;
    
        // controllerApp->DpctlExecute(stoi(sm.str()) - 6,
        //      "flow-mod cmd=add,table=0,prio=20 "
        //      "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 apply:output=3");
    cout << "txPacket: " << packet->GetSize() << " \t simTime: " << Simulator::Now().GetSeconds() << endl;
        
    
    Ptr<Packet> p = packet->Copy();
    EthernetHeader ipvHeader;
    p->RemoveHeader(ipvHeader);
    
    Ipv4Header ipHeader;
    p->PeekHeader(ipHeader);

    Ipv4Address sourceIp = ipHeader.GetSource();
    Ipv4Address destIp = ipHeader.GetDestination();
    
    std::cout << "Source IP: " <<  sourceIp  <<std::endl;
    std::cout << "Destination IP: " << destIp   <<std::endl;

     Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowClassifier);
     FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats();
   
     for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin();
         i != stats.end();
         ++i)
    {
        // first 14 FlowIds are for Connection between Switches and the Controller,
        // we don't want to display them
        // Duration for throughput measurement is 9.0 seconds, since
        // StartTime of the OnOffApplication is at about "second 1"
        // and Simulator::Stops at "second 10".
        if (i->first > 14 )
        {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

            if(sourceIp == t.sourceAddress && destIp == t.destinationAddress)
            {
                
                std::cout << "FlowId= " << i->first - 14 << " (" << t.sourceAddress << " -> "
                          << t.destinationAddress << ")\n";

                // entity->m_delaySum = i->second.delaySum.GetMilliSeconds();
                // std::cout << "  delaySum:  " << entity->m_delaySum << " ms\n";

                // gathering the delaySum for each
                if(i->first == 15)
                {
                  entity->m_delaySum.at(0) = i->second.delaySum.GetMilliSeconds();
                  cout << "  delaySum:  " << entity->m_delaySum.at(0) << " ms\n";
                
                } else if(i->first == 16){

                  entity->m_delaySum.at(1) = i->second.delaySum.GetMilliSeconds();
                  cout << "  delaySum:  " << entity->m_delaySum.at(1) << " ms\n";
                }else if(i->first == 17){
                  
                  entity->m_delaySum.at(2) = i->second.delaySum.GetMilliSeconds();
                  cout << "  delaySum:  " << entity->m_delaySum.at(2) << " ms\n";
                }else if(i->first == 18){
                  
                  entity->m_delaySum.at(3) = i->second.delaySum.GetMilliSeconds();
                  cout << "  delaySum:  " << entity->m_delaySum.at(3) << " ms\n";
                }

                entity->m_TxPackets = i->second.txPackets;
                cout << "  Tx Packets: " << entity->m_TxPackets << "\n";

                //Storing the RxPackets for each flow 
                if(i->first == 15)
                {
                  entity->m_RxPackets.at(0) = i->second.rxPackets;
                  cout << "  Rx Packets: " << entity->m_RxPackets.at(0) << "\n";
                
                } else if(i->first == 16){

                  entity->m_RxPackets.at(1) = i->second.rxPackets;
                  cout << "  Rx Packets: " << entity->m_RxPackets.at(1) << "\n";
                }else if(i->first == 17){
                  
                  entity->m_RxPackets.at(2) = i->second.rxPackets;
                  cout << "  Rx Packets: " << entity->m_RxPackets.at(2) << "\n";
                }else if(i->first == 18){
                  
                  entity->m_RxPackets.at(3) = i->second.rxPackets;
                  cout << "  Rx Packets: " << entity->m_RxPackets.at(3) << "\n";
                }
                
                entity->m_TxOffered = i->second.txBytes * 8.0 /(i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) 
                            / 1024;
 
                cout << "  TxOffered:  " << entity->m_TxOffered  << " Kbps\n"; 

               
                entity->m_Throughput = i->second.rxBytes * 8.0 / 
                    (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) 
                            / 1024 ;

                cout << "  Throughput: " << entity->m_Throughput  << " Kbps\n";

                entity->m_ControllerApp = ControllerApp;

                entity->m_currentObs.at(0) = i->first;
                entity->m_currentObs.at(1) = stoi(sm.str()) - 6; //s1,s2,..s7
                entity->m_currentObs.at(2) = entity->m_pOut.at(stoi(sm.str()) - 7);
                entity->m_currentObs.at(3) = entity->rxCounter(entity->m_RxPackets,i->first);
                entity->m_currentObs.at(4) = entity->m_delaySum.at(i->first -15);

                //Input port
                if(sm1.size() > 0)
                { 
                  cout << "Input Port " << stoi(sm1.str().substr(9,1)) + 1  <<  endl;
                  entity->m_currentObs.at(5) = stoi(sm1.str().substr(9,1)) + 1;
                  entity->m_pIn.at(stoi(sm.str()) - 7) = stoi(sm1.str().substr(9,1)) + 1;
                }                                           
                entity->ScheduleNextStateRead();
            }
        }
    }
 

  
}

} // ns3 namespace




