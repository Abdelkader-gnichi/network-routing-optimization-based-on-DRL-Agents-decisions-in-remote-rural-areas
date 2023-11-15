# self-management network based on SDN & DRL Agnet decicsions for routing optimization in remote/rural areas

This project aims to address connectivity challenges in remote/rural areas through the application of AI capabilities, specifically Deep Reinforcement Learning (DRL). The approach involves deploying a Dueling Double Deep Q-Network (Dueling DDQN) agent to optimize packet routing within a backhaul network utilizing SDN technologies.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Introduction

Connectivity remains a persistent challenge in remote and rural areas, limiting the potential for community development and access to global networks. This project introduces a cutting-edge solution leveraging the power of Artificial Intelligence, specifically Deep Reinforcement Learning (DRL), in combination with Software-Defined Networking (SDN). The primary goal is to optimize packet routing within backhaul networks, thus overcoming connectivity obstacles. Additionally, the project aims to eliminate the human factor that not only consumes financial resources but also elevates the risk of errors during network management.

## Features

- Dueling DDQN Agent: The project deploys a Dueling Double Deep Q-Network (DDQN) agent, harnessing the capabilities of Deep Reinforcement Learning. This intelligent agent makes decisions to optimize packet routing within SDN-based backhaul networks.

- Community Connectivity Assurance: Ensuring reliable connectivity for communities in remote regions, bridging the gap between isolated areas and the global network.

- Backhaul SDN-Based Network Optimization: Intelligent packet routing optimizations within the backhaul network, enhancing the efficiency and performance of data transfer.

- Quality of Service (QoS) Compliance: A crucial aspect of the project is to respect Quality of Service limits during packet routing, prioritizing factors such as minimal delay. This ensures a seamless and reliable network experience for users in remote/rural areas.

This project represents a forward-looking approach to addressing connectivity issues, utilizing AI technologies to bring about positive impacts on community development and network performance in challenging geographic locations.

## Prerequisites

Before you begin, make sure you have the following prerequisites:

- **ns-3:** This is a discrete-event network simulator for Internet systems. We will use ns-3 to simulate the backhaul network, representing the environment in which our Dueling DDQN agent interacts to make decisions ensuring packet routing to their destinations. ns-3 is free and open-source software. You can obtain it from the official website [here](https://www.nsnam.org/) or directly from its GitHub [repository](https://github.com/nsnam/ns-3-dev-git).
- **NetAnim:** NetAnim is an offline animator based on the Qt toolkit. It animates a previously executed ns-3 simulation using an XML trace file generated during a simulation, we use NetAnim to generate a graphical representation for our simulation to show the packets routing based on the DRL agent decicions, Also you can git it from the official ns-3 website [link](https://www.nsnam.org/wiki/NetAnim).

- **ofswitch13:** This is the OFSwitch13 module, which enhances the ns-3 Network Simulator with [OpenFlow 1.3](https://opennetworking.org/wp-content/uploads/2014/10/openflow-spec-v1.3.0.pdf) capabilities, allowing ns-3 users to simulate Software-Defined Networks (SDN), Also you can get it from its GitHub [repository](https://github.com/ljerezchaves/ofswitch13).
- **NS3-Gym:** is a middleware play the role of a bridge between ns-3 and OpenAI Gym framework. It notably takes care of transferring data between the our Dueling DDQN agent and the simulation environment, you can get it from it offcial GitHub [repo](https://github.com/tkn-tub/ns3-gym).
- **OpenAI Gym:** is a Pythonic API that provides simulated training environments(ns-3 in this case) to train and test deep reinforcement learning agents, you can get it from this [link](https://github.com/openai/gym).
- **PFRL:** is a PyTorch-based library within the OpenAI Gym setup that implements several Deep Reinforcement Learning algorithms such as (DQN, DDQN, IQN, Rainbow, etc), we will use this library to develop our Dueling DDQN agent and its components, Also you can get it from its offcial GitHub [repo](https://github.com/pfnet/pfrl).

## Installation

After successfully installing ns-3 and configuring it, including the installation of all other prerequisites, follow these steps:

1. Clone this repository into the `ns-3.X/contrib/opengym/examples` directory of your ns-3 installation folder. If you're using ns-3.37, the path will be `ns-3.37/contrib/opengym/examples`.

```shell
git clone https://github.com/Abdelkader-gnichi/network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas.git
```

2. Make sure to configure the build and compile process of the project in the `CMakeLists.txt` file located in the `ns-3.X/contrib/opengym/examples` directory to enable ns-3 to build and compile our project properly with all the other ns-3 external libraries.

```cmake
build_lib_example(
  NAME network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas
  SOURCE_FILES qos/sim.cc
               qos/qos-controller.cc
               qos/mygym.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libapplications}
    ${libinternet-apps}
    ${libinternet}
    ${libopengym}
    ${libnetanim}
    ${libflow-monitor}
    ${libofswitch13}
    ${libnetwork}
    
)
```
## Usage

**1. Run the Simulation:**

* Change your current working directory from `ns-3.X` to `/contrib/opengym/examples/opengym/` using this command:
  ```shell
  cd ./contrib/opengym/examples/opengym/
  ```

* Execute the simulation script with the desired parameters. Below is an example command:

```shell
./agent_1.py --start 1 --total_episodes 100 --total_steps 300 --agent_name "Dueling DDQN"
```
- Avalable parameters:
   - `--start`: Set to 1 to start the ns-3 simulation script (default is 1).
   - `--total_episodes`: Specify the total number of episodes (default is 100).
   - `--total_steps`: Set the total number of steps in each episode (default is 300).
   - `--agent_name`: Choose the DRL agent for simulation (options: "Dueling DDQN", "Dueling DDQN-MLPS", "DDQN"), 
     (default is Dueling DDQN).
  
   - Adjust the parameters as needed for your specific simulation requirements.
  
* (Optional) Start ns-3 simulation script and Gym agent separately in two terminals (useful for debugging):
```shell

# Terminal 1
./ns3 run "network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas"

# Terminal 2
cd ./contrib/opengym/examples/opengym/ 
./agent_1.py --start 0 --total_episodes 100 --total_steps 300 --agent_name "Dueling DDQN"

```
* (Optional) You can print the output results in a `output.txt` file using this command:

  ```shell
  
  ./agent_1.py --start 1 --total_episodes 100 --total_steps 300 --agent_name "Dueling DDQN" > output.txt           
  
  ```
* Or simply you can run the script without specifying anything using this command:
  
```shell
./agent_1.py
```
* Finaly if the script wont' to be executed you may should grant it the execute permition using this command:
    
```shell 
sudo chmod a+x ./agent_1.py
```

  
  











