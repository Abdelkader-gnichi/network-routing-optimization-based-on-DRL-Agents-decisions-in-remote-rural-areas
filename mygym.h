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


#ifndef MY_GYM_ENTITY_H
#define MY_GYM_ENTITY_H

#include "ns3/stats-module.h"
#include "ns3/opengym-module.h"

#include "ns3/ipv4-flow-classifier.h"
#include "ns3/flow-monitor-helper.h"
#include <ns3/ofswitch13-module.h>
#include <ns3/ethernet-header.h>
#include <ns3/flow-monitor.h>
#include "ns3/ipv4-header.h"
#include <ns3/udp-header.h>
#include "ns3/header.h"  
#include "ns3/ipv4.h"
#include <string>
#include <regex>


namespace ns3 {

using namespace std;


class MyGymEnv : public OpenGymEnv
{
public:
  MyGymEnv ();
  MyGymEnv (uint32_t obsSize, uint32_t id);
  virtual ~MyGymEnv ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  Ptr<OpenGymSpace> GetActionSpace();
  Ptr<OpenGymSpace> GetObservationSpace();
  bool GetGameOver();
  Ptr<OpenGymDataContainer> GetObservation();
  float GetReward();
  std::string GetExtraInfo();
  bool ExecuteActions(Ptr<OpenGymDataContainer> action);

  // the function has to be static to work with MakeBoundCallback
  // that is why we pass pointer to MyGymEnv instance to be able to store the context (node, etc)
  static void TracePacketReceive(Ptr<MyGymEnv> entity, Ptr<OFSwitch13Controller> ControllerApp,
                     Ptr<ns3::FlowMonitor> flowMonitor, Ptr<ns3::FlowClassifier> flowClassifier,
                                                        string context, Ptr<const Packet> packet);
  

private:
void ScheduleNextStateRead(); 

  uint32_t rxCounter(std::vector<int64_t> rx, uint32_t a);
  Ptr<OFSwitch13Controller> m_ControllerApp;
  Time m_interval = Seconds(1/5);
  
  uint64_t m_rxPktNum  = 0;
  uint64_t m_TxPackets = 0;
  
  uint32_t m_currentSwitchId = 0;
  uint32_t m_currentFlowid = 0;
  uint32_t m_observation_size;
  uint32_t m_isRxedNow = 0;
  uint32_t m_agentId;
  
  int64_t m_currentdelaySum= 0;
  std::vector<int64_t> m_delaySum= {0,0,0,0,};

  float m_TxOffered  = 0.0;
  float m_Throughput = 0.0;
  
  std::vector<int64_t> m_RxPackets = {0,0,0,0};
  std::vector<int64_t> m_currentObs = {15,4,3,0,0,0,};
  std::vector<uint32_t> m_pOut = {3,2,4,3,4,4,1,};
  std::vector<uint32_t> m_pIn = {0,0,0,0,0,0,0,};
  // std::vector<int64_t> m_currentFlowid {15,16,17,18,0,0,0,};
  
  
};

}


#endif // MY_GYM_ENTITY_H
