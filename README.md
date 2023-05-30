# myofswitch13-first

# for this script simulation i use ns-3.37 and the ofswitch13 library.

### To download ofswitch13 use this link https://github.com/ljerezchaves/ofswitch13/releases/tag/5.2.0
### also i install netanim for graphic simulation but it is optionnal since i only want you (Tom) to help me out to fix the error

### this is how to install ofswitch13 with ns-3.37:

2.1.1 Before starting 

Before starting, ensure you have the following minimal requirements installed on your system:
```
$ sudo apt install g++ python3 cmake ninja-build git
$ sudo apt install make pkg-config libtool libboost-dev
2.1.2 Compiling the code

Clone the ns-3 source code repository into your machine and checkout a stable version (we are using the ns-3.37):
$ git clone https://gitlab.com/nsnam/ns-3-dev.git
$ cd ns-3-dev
$ git checkout -b ns-3.37 ns-3.37
Download the OFSwitch13 code into the contrib/ folder.
$ cd contrib/
$ git clone https://github.com/ljerezchaves/ofswitch13.git 

Update the OFSwitch13 code to a stable version (we are using release 5.2.0, which is compatible with ns-3.37)2 :

$ cd ofswitch13
$ git checkout 5.2.0
Go back to the ns-3 root directory and patch the ns-3 code with the appropriated ofswitch13 patch available under the
ofswitch13/utils/ directory (check for the correct ns-3 version):
$ cd ../../
$ patch -p1 < contrib/ofswitch13/utils/ofswitch13-3_37.patch 

1 Other distributions or versions may require different steps, especially regarding library compilation.
2 Starting at OFSwitch13 release 5.2.0, the cmake build system will automatically download and compile the correct version of BOFUSS library
(Internet connection is required). For older OFSwitch13 releases, we suggest you check the documentation and follow the proper build steps.
7OFSwitch13 Module Documentation, Release 5.2.0
This patch creates the new OpenFlow receive callback at CsmaNetDevice and VirtualNetDevice, allowing OpenFlow
switch to get raw packets from these devices. The module also brings a csma-full-duplex patch for improving CSMA
connections with full-duplex support. This is an optional patch that can be applyed after the ofswitch13 patch.
Now, configure the ns-3. By default, the cmake build system will handle BOFUSS library download and compilation.
Anyway, if your want to use a custom BOFUSS library, use the -DNS3_OFSWITCH13_BOFUSS_PATH configuration option
to specify its location:

$ ./ns3 configure 

Check for the enabled ns-3 OFSwitch13 integration feature after configuration. Finally, compile the simulator:

$ ./ns3 build  
```
# That’s it! Enjoy your ns-3 fresh compilation with OpenFlow 1.3 capabilities.

### after building up the ofswitch13 module you can clone the myofswitch13-first.cc file into scratch file in the ns3 directoriy and run it 

### this is the error i that i want to fix :

``` OFSwitch13Controller:GetRemoteSwitch(0x138d790, 03-07-0a:64:06:02:01:c0:00)
RX from switch 10.100.6.2 [dp 7]: pkt_in{buffer="1", tlen="64", reas="no_match", table="0", dlen="64"}
OFSwitch13Controller:HandleSwitchMsg(0x138d790, 0x1552780, 0)
OFSwitch13LearningController:HandlePacketIn(0x138d790, 0x1552780, 0)
Packet in match: oxm{in_port="2", metadata="0x0", eth_dst="ff:ff:ff:ff:ff:ff", eth_src="00:00:00:00:00:01", eth_type="0x806", arp_op="0x1", arp_spa="10.1.1.1", arp_tpa="10.1.1.7", arp_sha="00:00:00:00:00:01", arp_tha="ff:ff:ff:ff:ff:ff", tunnel_id="0x0"}
assert failed. cond="itSrc->second == inPort", msg="Inconsistent L2 switching table", +1.006652644s 14 file=/home/gadour/source/ns-3.37/contrib/ofswitch13/model/ofswitch13-learning-controller.cc, line=148
terminate called without an active exception
Command 'build/scratch/ns3.37-myofswitch13-first-debug' died with <Signals.SIGABRT: 6>.
```
