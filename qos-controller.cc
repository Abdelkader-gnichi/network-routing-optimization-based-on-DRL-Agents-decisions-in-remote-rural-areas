/*
 * Author: Luciano Jerez Chaves <ljerezchaves@gmail.com>
 */

#include "qos-controller.h"

#include <ns3/internet-module.h>
#include <ns3/network-module.h>

NS_LOG_COMPONENT_DEFINE("QosController");
NS_OBJECT_ENSURE_REGISTERED(QosController);

QosController::QosController()
{
    NS_LOG_FUNCTION(this);

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

    std::pair<Ipv4Address, Mac48Address> entry1(ip1, mac1);
    std::pair<Ipv4Address, Mac48Address> entry2(ip2, mac2);
    std::pair<Ipv4Address, Mac48Address> entry4(ip4, mac4);
    std::pair<Ipv4Address, Mac48Address> entry5(ip5, mac5);
    std::pair<Ipv4Address, Mac48Address> entry7(ip7, mac7);

    m_arpTable.insert(entry1);
    m_arpTable.insert(entry2);
    m_arpTable.insert(entry4);
    m_arpTable.insert(entry5);
    m_arpTable.insert(entry7);
}

QosController::~QosController()
{
    NS_LOG_FUNCTION(this);
}

void
QosController::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_arpTable.clear();
    OFSwitch13Controller::DoDispose();
}

TypeId
QosController::GetTypeId()
{
    static TypeId tid = TypeId("ns3::QosController")
                            .SetParent<OFSwitch13Controller>()
                            .SetGroupName("OFSwitch13")
                            .AddConstructor<QosController>()
                            .AddAttribute("EnableMeter",
                                          "Enable per-flow mettering.",
                                          BooleanValue(false),
                                          MakeBooleanAccessor(&QosController::m_meterEnable),
                                          MakeBooleanChecker())
                            .AddAttribute("MeterRate",
                                          "Per-flow meter rate.",
                                          DataRateValue(DataRate("256Kbps")),
                                          MakeDataRateAccessor(&QosController::m_meterRate),
                                          MakeDataRateChecker());
                          // here
    return tid;
}

ofl_err
QosController::HandlePacketIn(struct ofl_msg_packet_in* msg,
                              Ptr<const RemoteSwitch> swtch,
                              uint32_t xid)
{
    NS_LOG_FUNCTION(this << swtch << xid);

    char* msgStr = ofl_structs_match_to_string((struct ofl_match_header*)msg->match, nullptr);
    NS_LOG_DEBUG("Packet in match: " << msgStr);
    free(msgStr);

    if (msg->reason == OFPR_ACTION)
    {
        // Get Ethernet frame type
        uint16_t ethType;
        struct ofl_match_tlv* tlv;
        tlv = oxm_match_lookup(OXM_OF_ETH_TYPE, (struct ofl_match*)msg->match);
        memcpy(&ethType, tlv->value, OXM_LENGTH(OXM_OF_ETH_TYPE));

        if (ethType == ArpL3Protocol::PROT_NUMBER)
        {
            // ARP packet
            return HandleArpPacketIn(msg, swtch, xid);
        }
    }

    // All handlers must free the message when everything is ok
    ofl_msg_free((struct ofl_msg_header*)msg, nullptr);
    return 0;
}

void
QosController::HandshakeSuccessful(Ptr<const RemoteSwitch> swtch)
{
    NS_LOG_FUNCTION(this << swtch);

    // This function is called after a successfully handshake between controller
    // and each switch. Let's check the switch for proper configuration.
    if (swtch->GetDpId() == 1)
    {
        ConfigureSwitch1(swtch);
    }
    else if (swtch->GetDpId() == 2)
    {
        ConfigureSwitch2(swtch);
    }
    else if (swtch->GetDpId() == 3)
    {
        ConfigureSwitch3(swtch);
    }
    else if (swtch->GetDpId() == 4)
    {
        ConfigureSwitch4(swtch);
    }
    else if (swtch->GetDpId() == 5)
    {
        ConfigureSwitch5(swtch);
    }
    else if (swtch->GetDpId() == 6)
    {
        ConfigureSwitch6(swtch);
    }
    else if (swtch->GetDpId() == 7)
    {
        ConfigureSwitch7(swtch);
    }

}

void
QosController::ConfigureSwitch1(Ptr<const RemoteSwitch> swtch)
{
    NS_LOG_FUNCTION(this << swtch);

    // Get the switch datapath ID
    uint64_t swDpId = swtch->GetDpId();

    // For packet-in messages, send only the first 128 bytes to the controller
    DpctlExecute(swDpId, "set-config miss=128");

    // Redirect ARP requests to the controller
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=0,prio=20 "
                 "eth_type=0x0806,arp_op=1 apply:output=ctrl");
    
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 goto:1");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 goto:2");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 goto:3");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 goto:4");
    
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=1,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 write:output=3");

    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=2,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 write:output=3");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=3,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 write:output=3");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=4,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 write:output=3");


}

void
QosController::ConfigureSwitch2(Ptr<const RemoteSwitch> swtch)
{
    NS_LOG_FUNCTION(this << swtch);

    // Get the switch datapath ID
    uint64_t swDpId = swtch->GetDpId();

    // For packet-in messages, send only the first 128 bytes to the controller
    DpctlExecute(swDpId, "set-config miss=128");

    // Redirect ARP requests to the controller
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=0,prio=20 "
                 "eth_type=0x0806,arp_op=1 apply:output=ctrl");

    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 goto:1");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 goto:2");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 goto:3");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 goto:4");


    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=1,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 write:output=2");

    
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=2,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 write:output=2");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=3,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 write:output=2");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=4,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 write:output=2");


}

void
QosController::ConfigureSwitch3(Ptr<const RemoteSwitch> swtch)
{
    NS_LOG_FUNCTION(this << swtch);

    // Get the switch datapath ID
    uint64_t swDpId = swtch->GetDpId();

    // For packet-in messages, send only the first 128 bytes to the controller
    DpctlExecute(swDpId, "set-config miss=128");

    // Redirect ARP requests to the controller
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=0,prio=20 "
                 "eth_type=0x0806,arp_op=1 apply:output=ctrl");

    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 goto:1");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 goto:2");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 goto:3");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 goto:4");
    
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=1,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 write:output=4");
    

    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=2,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 write:output=4");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=3,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 write:output=4");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=4,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 write:output=4");

}

void
QosController::ConfigureSwitch4(Ptr<const RemoteSwitch> swtch)
{
    NS_LOG_FUNCTION(this << swtch);

    // Get the switch datapath ID
    uint64_t swDpId = swtch->GetDpId();

    // For packet-in messages, send only the first 128 bytes to the controller
    DpctlExecute(swDpId, "set-config miss=128");

    // Redirect ARP requests to the controller
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=0,prio=20 "
                 "eth_type=0x0806,arp_op=1 apply:output=ctrl");

    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 goto:1");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 goto:2");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 goto:3");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 goto:4");

    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=1,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 write:output=2");
    
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=2,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 write:output=2");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=3,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 write:output=3");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=4,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 write:output=2");

}

void
QosController::ConfigureSwitch5(Ptr<const RemoteSwitch> swtch)
{
    NS_LOG_FUNCTION(this << swtch);

    // Get the switch datapath ID
    uint64_t swDpId = swtch->GetDpId();

    // For packet-in messages, send only the first 128 bytes to the controller
    DpctlExecute(swDpId, "set-config miss=128");

    // Redirect ARP requests to the controller
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=0,prio=20 "
                 "eth_type=0x0806,arp_op=1 apply:output=ctrl");

    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 goto:1");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 goto:2");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 goto:3");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 goto:4");

    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=1,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 write:output=4");

    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=2,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 write:output=4");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=3,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 write:output=4");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=4,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 write:output=4");

}

void
QosController::ConfigureSwitch6(Ptr<const RemoteSwitch> swtch)
{
    NS_LOG_FUNCTION(this << swtch);

    // Get the switch datapath ID
    uint64_t swDpId = swtch->GetDpId();

    // For packet-in messages, send only the first 128 bytes to the controller
    DpctlExecute(swDpId, "set-config miss=128");

    // Redirect ARP requests to the controller
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=0,prio=20 "
                 "eth_type=0x0806,arp_op=1 apply:output=ctrl");

    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 goto:1");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 goto:2");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 goto:3");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 goto:4");

    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=1,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 write:output=4");
    
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=2,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 write:output=4");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=3,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 write:output=4");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=4,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 write:output=4");

}

void
QosController::ConfigureSwitch7(Ptr<const RemoteSwitch> swtch)
{
    NS_LOG_FUNCTION(this << swtch);

    // Get the switch datapath ID
    uint64_t swDpId = swtch->GetDpId();

    // For packet-in messages, send only the first 128 bytes to the controller
    DpctlExecute(swDpId, "set-config miss=128");

    // Redirect ARP requests to the controller
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=0,prio=20 "
                 "eth_type=0x0806,arp_op=1 apply:output=ctrl");

    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 goto:1");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 goto:2");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 goto:3");
    
    DpctlExecute(swDpId,
             "flow-mod cmd=add,table=0,prio=200 "
             "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 goto:4");

    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=1,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.1,ip_dst=192.168.1.7 write:output=1");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=2,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.2,ip_dst=192.168.1.7 write:output=1");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=3,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.4,ip_dst=192.168.1.7 write:output=1");
    DpctlExecute(swDpId,
                 "flow-mod cmd=add,table=4,prio=200 "
                 "eth_type=0x0800,ip_proto=17,udp_dst=9,ip_src=192.168.1.5,ip_dst=192.168.1.7 write:output=1");

}

ofl_err
QosController::HandleArpPacketIn(struct ofl_msg_packet_in* msg,
                                 Ptr<const RemoteSwitch> swtch,
                                 uint32_t xid)
{
    NS_LOG_FUNCTION(this << swtch << xid);

    struct ofl_match_tlv* tlv;
    Ipv4Address serverIp = Ipv4Address::ConvertFrom(Address(Ipv4Address("192.168.1.7")));
    Mac48Address serverMac = Mac48Address::ConvertFrom(Address(Mac48Address("00:00:00:00:00:1b")));

    // Get ARP operation
    uint16_t arpOp;
    tlv = oxm_match_lookup(OXM_OF_ARP_OP, (struct ofl_match*)msg->match);
    memcpy(&arpOp, tlv->value, OXM_LENGTH(OXM_OF_ARP_OP));

    // Get input port
    uint32_t inPort;
    tlv = oxm_match_lookup(OXM_OF_IN_PORT, (struct ofl_match*)msg->match);
    memcpy(&inPort, tlv->value, OXM_LENGTH(OXM_OF_IN_PORT));

    // Get source and target IP address
    Ipv4Address srcIp;
    Ipv4Address dstIp;
    srcIp = ExtractIpv4Address(OXM_OF_ARP_SPA, (struct ofl_match*)msg->match);
    dstIp = ExtractIpv4Address(OXM_OF_ARP_TPA, (struct ofl_match*)msg->match);

    // Get Source MAC address
    Mac48Address srcMac;
    Mac48Address dstMac;
    tlv = oxm_match_lookup(OXM_OF_ARP_SHA, (struct ofl_match*)msg->match);
    srcMac.CopyFrom(tlv->value);
    tlv = oxm_match_lookup(OXM_OF_ARP_THA, (struct ofl_match*)msg->match);
    dstMac.CopyFrom(tlv->value);

    // Check for ARP request
    if (arpOp == ArpHeader::ARP_TYPE_REQUEST)
    {
        uint8_t replyData[64];

        // Check for destination IP
        if (dstIp == serverIp)
        {
            // Reply with virtual service IP/MAC addresses
            Ptr<Packet> pkt = CreateArpReply(serverMac, dstIp, srcMac, srcIp);
            NS_ASSERT_MSG(pkt->GetSize() == 64, "Invalid packet size.");
            pkt->CopyData(replyData, 64);
        }
        else
        {
            // Check for existing information
            Mac48Address replyMac = GetArpEntry(dstIp);
            Ptr<Packet> pkt = CreateArpReply(replyMac, dstIp, srcMac, srcIp);
            NS_ASSERT_MSG(pkt->GetSize() == 64, "Invalid packet size.");
            pkt->CopyData(replyData, 64);
        }

        // Send the ARP replay back to the input port
        struct ofl_action_output* action =
            (struct ofl_action_output*)xmalloc(sizeof(struct ofl_action_output));
        action->header.type = OFPAT_OUTPUT;
        action->port = OFPP_IN_PORT;
        action->max_len = 0;

        // Send the ARP reply within an OpenFlow PacketOut message
        struct ofl_msg_packet_out reply;
        reply.header.type = OFPT_PACKET_OUT;
        reply.buffer_id = OFP_NO_BUFFER;
        reply.in_port = inPort;
        reply.data_length = 64;
        reply.data = &replyData[0];
        reply.actions_num = 1;
        reply.actions = (struct ofl_action_header**)&action;

        SendToSwitch(swtch, (struct ofl_msg_header*)&reply, xid);
        free(action);
    }

    // All handlers must free the message when everything is ok
    ofl_msg_free((struct ofl_msg_header*)msg, nullptr);
    return 0;
}


Ipv4Address
QosController::ExtractIpv4Address(uint32_t oxm_of, struct ofl_match* match)
{
    switch (oxm_of)
    {
    case static_cast<uint32_t>(OXM_OF_ARP_SPA):
    case static_cast<uint32_t>(OXM_OF_ARP_TPA):
    case static_cast<uint32_t>(OXM_OF_IPV4_DST):
    case static_cast<uint32_t>(OXM_OF_IPV4_SRC): {
        uint32_t ip;
        int size = OXM_LENGTH(oxm_of);
        struct ofl_match_tlv* tlv = oxm_match_lookup(oxm_of, match);
        memcpy(&ip, tlv->value, size);
        return Ipv4Address(ntohl(ip));
    }
    default:
        NS_ABORT_MSG("Invalid IP field.");
    }
}

Ptr<Packet>
QosController::CreateArpRequest(Mac48Address srcMac, Ipv4Address srcIp, Ipv4Address dstIp)
{
    NS_LOG_FUNCTION(this << srcMac << srcIp << dstIp);

    Ptr<Packet> packet = Create<Packet>();

    // ARP header
    ArpHeader arp;
    arp.SetRequest(srcMac, srcIp, Mac48Address::GetBroadcast(), dstIp);
    packet->AddHeader(arp);

    // Ethernet header
    EthernetHeader eth(false);
    eth.SetSource(srcMac);
    eth.SetDestination(Mac48Address::GetBroadcast());
    if (packet->GetSize() < 46)
    {
        uint8_t buffer[46];
        memset(buffer, 0, 46);
        Ptr<Packet> padd = Create<Packet>(buffer, 46 - packet->GetSize());
        packet->AddAtEnd(padd);
    }
    eth.SetLengthType(ArpL3Protocol::PROT_NUMBER);
    packet->AddHeader(eth);

    // Ethernet trailer
    EthernetTrailer trailer;
    if (Node::ChecksumEnabled())
    {
        trailer.EnableFcs(true);
    }
    trailer.CalcFcs(packet);
    packet->AddTrailer(trailer);

    return packet;
}

Ptr<Packet>
QosController::CreateArpReply(Mac48Address srcMac,
                              Ipv4Address srcIp,
                              Mac48Address dstMac,
                              Ipv4Address dstIp)
{
    NS_LOG_FUNCTION(this << srcMac << srcIp << dstMac << dstIp);

    Ptr<Packet> packet = Create<Packet>();

    // ARP header
    ArpHeader arp;
    arp.SetReply(srcMac, srcIp, dstMac, dstIp);
    packet->AddHeader(arp);

    // Ethernet header
    EthernetHeader eth(false);
    eth.SetSource(srcMac);
    eth.SetDestination(dstMac);
    if (packet->GetSize() < 46)
    {
        uint8_t buffer[46];
        memset(buffer, 0, 46);
        Ptr<Packet> padd = Create<Packet>(buffer, 46 - packet->GetSize());
        packet->AddAtEnd(padd);
    }
    eth.SetLengthType(ArpL3Protocol::PROT_NUMBER);
    packet->AddHeader(eth);

    // Ethernet trailer
    EthernetTrailer trailer;
    if (Node::ChecksumEnabled())
    {
        trailer.EnableFcs(true);
    }
    trailer.CalcFcs(packet);
    packet->AddTrailer(trailer);

    return packet;
}

void
QosController::SaveArpEntry(Ipv4Address ipAddr, Mac48Address macAddr)
{
    std::pair<Ipv4Address, Mac48Address> entry(ipAddr, macAddr);
    std::pair<IpMacMap_t::iterator, bool> ret;
    ret = m_arpTable.insert(entry);
    if (ret.second == true)
    {
        NS_LOG_INFO("New ARP entry: " << ipAddr << " - " << macAddr);
        return;
    }
}

Mac48Address
QosController::GetArpEntry(Ipv4Address ip)
{
    IpMacMap_t::iterator ret;
    ret = m_arpTable.find(ip);
    if (ret != m_arpTable.end())
    {
        NS_LOG_INFO("Found ARP entry: " << ip << " - " << ret->second);
        return ret->second;
    }
    NS_ABORT_MSG("No ARP information for this IP.");
}


