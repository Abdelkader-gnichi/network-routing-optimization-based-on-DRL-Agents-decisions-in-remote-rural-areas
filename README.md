<!-- @format -->

# self-management network based on SDN & DRL Agent decisions for routing optimization in remote/rural areas

This project aims to address connectivity challenges in remote/rural areas through the application of AI capabilities, specifically Deep Reinforcement Learning (DRL). The approach involves deploying a Dueling Double Deep Q-Network (Dueling DDQN) agent to optimize packet routing within a backhaul network utilizing SDN technologies.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Training Results](#training-results)
- [Dueling DDQN Agent architecture](#dueling-ddqn-agent-architecture)
- [Case Study Network topology](#case-study-network-topology)
- [Dueling DDQN Agent interactions with the Network Topology (environment)](#dueling-ddqn-agent-interactions-with-the-network-topology-(environment))
- [Performance Summary](#performance-summary) 
- [](#)

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
- **NetAnim:** NetAnim is an offline animator based on the Qt toolkit. It animates a previously executed ns-3 simulation using an XML trace file generated during a simulation, we use NetAnim to generate a graphical representation for our simulation to show the packets routing based on the DRL agent decisions, Also you can git it from the official ns-3 website [link](https://www.nsnam.org/wiki/NetAnim).

- **ofswitch13:** This is the OFSwitch13 module, which enhances the ns-3 Network Simulator with [OpenFlow 1.3](https://opennetworking.org/wp-content/uploads/2014/10/openflow-spec-v1.3.0.pdf) capabilities, allowing ns-3 users to simulate Software-Defined Networks (SDN), Also you can get it from its GitHub [repository](https://github.com/ljerezchaves/ofswitch13).
- **NS3-Gym:** is a middleware play the role of a bridge between ns-3 and OpenAI Gym framework. It notably takes care of transferring data between the our Dueling DDQN agent and the simulation environment, you can get it from it official GitHub [repo](https://github.com/tkn-tub/ns3-gym).
- **OpenAI Gym:** is a Pythonic API that provides simulated training environments(ns-3 in this case) to train and test deep reinforcement learning agents, you can get it from this [link](https://github.com/openai/gym).
- **PFRL:** is a PyTorch-based library within the OpenAI Gym setup that implements several Deep Reinforcement Learning algorithms such as (DQN, DDQN, IQN, Rainbow, etc), we will use this library to develop our Dueling DDQN agent and its components, Also you can get it from its official GitHub [repo](https://github.com/pfnet/pfrl).

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

- Change your current working directory from `ns-3.X` to `/contrib/opengym/examples/opengym/` using this command:

  ```shell
  cd ./contrib/opengym/examples/opengym/
  ```

- Execute the simulation script with the desired parameters. Below is an example command:

```shell
./agent_1.py --start 1 --total_episodes 100 --total_steps 300 --agent_name "Dueling DDQN"
```

- Available parameters:

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

- (Optional) You can print the output results in a `output.txt` file using this command:

  ```shell

  ./agent_1.py --start 1 --total_episodes 100 --total_steps 300 --agent_name "Dueling DDQN" > output.txt

  ```

- Or simply you can run the script without specifying anything using this command:

```shell
./agent_1.py
```

- Finally, if the script won't execute, you may need to grant it execute permission using this command:

```shell
sudo chmod a+x ./agent_1.py
```

## Dueling DDQN Agent architecture

- This Figure below shows our Dueling DDQN Agent architecture including the agent input and output:

![dueling-struct drawio](https://github.com/Abdelkader-gnichi/network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas/assets/64486451/806f6d9f-087b-47cd-8b07-968b35cb3b0a)

## Case Study Network topology

- As shown in the figure below, there are six communities, each with its own Base Station. Each Base Station is fully connected to all its neighboring Base Stations, forming a partial mesh that interconnects all the communities. Returning to the communities, each community attempts to send packets or reach the Central Base Station, representing the gateway to the global Internet network.


![Screenshot from 2023-05-30 19-44-13](https://github.com/Abdelkader-gnichi/network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas/assets/64486451/56266147-7adb-43a4-9f41-dbac1e6d50f2)

- To test our self-management architecture, we have considered the following scenario.
- The chosen scenario as shown in the Fig above draws inspiration from prior work on connectivity in rural Amazon regions.
- The scenario is characterized by a multi-hop topology for the backhaul network.
- This topology has been successful in reaching remote communities in a cost-effective manner with limited infrastructure investment.
  
## Dueling DDQN Agent interactions with the Network Topology (environment)

![Dueling-DDQN-sequence-diag-colorful](https://github.com/Abdelkader-gnichi/network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas/assets/64486451/2cc830da-713a-4737-a1c2-f1b3045f10b5)

- As shown in the sequence diagram, the agent bases its decisions on information (observations) gathered by the SDN-C (Controller) from the switches. Once the Dueling DDQN determines the appropriate output port for the flow and switch, it communicates this decision to the SDN-C. The SDN-C then implements the decision into the correct switch, directing the packet to the appropriate port.
- The Dueling DDQN agent receives new observations along with the associated reward. This information is utilized by the agent to update its neural networks, aiding in the training process to reach the optimal policy.

## Training Results

- For the simulation we prepare two scenarios:
  - In the first scenario, we attempt to send traffic, referred to as Flow 1, from the farthest and most challenging community, Community C, to the Central Base Station.
  - In the second scenario, we aim to send traffic, labeled as Flow 2, from a community that is neither too close nor too far, Community E. This scenario poses a significant challenge for the agent, as it may make suboptimal decisions and get stuck in loops before finding the optimal route to the Central Base Station. Consequently, the average delay value may increase compared to the optimal value.

### 1. DDQN Agent Results: 
For comparison purposes, we initially deploy a DDQN agent to assess its performance in solving the routing optimization problem in the network topology. This evaluation will serve as a benchmark for comparing its performance with our `Dueling DDQN agent`, which is expected to exhibit significant improvements. Stay tuned for the results.

#### ⬦ DDQN Agent Results for the Flow 1 from community C (1st scenario):


![Research Proposal Business Presentation in Dark Green Orange Geometric Style (1)](https://github.com/Abdelkader-gnichi/network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas/assets/64486451/9ba5055c-1572-438b-9c28-ec134d10df4a)

- As illustrated in the Reward Sum Figure, the DDQN agent exhibits a struggle to converge to the optimal policy, requiring `30 episodes` to achieve convergence. Notably, a significant decrease in the agent's performance is observed in `episode 18`.

- In the Average Delay Figure, it is evident that the DDQN agent initially struggles to keep the average delay values below the optimal levels in the early episodes. However, it eventually converges after `28 episodes`, leading to improved average delay results.

#### ⬦ DDQN Agent Results for the Flow 2 from community E (2nd scenario):

![Research Proposal Business Presentation in Dark Green Orange Geometric Style (2)](https://github.com/Abdelkader-gnichi/network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas/assets/64486451/f2506713-915b-4342-a089-4412a148b9f4)

- As depicted in the Reward Sum Figure, the DDQN agent converges with some hesitation in its decisions, requiring `14 episodes` to reach the optimal policy.
- This poor performance has an impact on the average delay values, causing the DDQN agent to struggle in gradually controlling the delay values. It eventually converges to the optimal values after `13 episodes`.

### 2. Dueling DDQN Agent Results: 

In this section, we present the results for the Dueling DDQN agent, which we have chosen as the decision-making unit for this project. Our selection is based on a comparative study that assesses six widely recognized Deep Reinforcement Learning (DRL) agent variants, including Q-learning, DQN without Target Network, DQN with Target Network, Dueling DQN, DDQN, and finally, Dueling DDQN.

Our preference for the Dueling DDQN variant is rooted in its outstanding performance, surpassing all other DRL variants in benchmark environments such as CartPole-v1 and LunarLander-v2. Additionally, we find the Dueling architecture particularly suitable for navigation problems and tasks, such as autonomous driving, maze solving, and drone control. Given that packet routing or routing optimization problems are inherently navigation problems, the Dueling DDQN emerges as the ideal candidate for this project. 

#### ⬦ Dueling DDQN Agent Results for the Flow 1 from community C (1st scenario):

![Research Proposal Business Presentation in Dark Green Orange Geometric Style](https://github.com/Abdelkader-gnichi/network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas/assets/64486451/57601c54-95d9-411d-a326-61d595f4a111)

- It is more evident in the Reward Sum Figure that the Dueling DDQN agent converges much faster than the DDQN agent, achieving convergence in just 17 episodes without any performance decrease. This efficiency instills confidence in its decision-making capabilities.

- This strong performance is reflected in the Average Delay Figure, where the Dueling DDQN adeptly controls the average delay values from the early episodes, maintaining them at low levels. It then smoothly converges to the optimal values in just 11 episodes.

#### ⬦ Dueling DDQN Agent Results for the Flow 2 from community E (2nd scenario): 

![Research Proposal Business Presentation in Dark Green Orange Geometric Style (3)](https://github.com/Abdelkader-gnichi/network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas/assets/64486451/b01bb797-f979-4866-bfa4-33e5d2345461)

- As shown in the Reward Sum Figure, it is evident that the Dueling DDQN converges much faster and without hesitation compared to the DDQN agent, reaching the optimal policy in just 7 episodes.
- This strong performance enables the Dueling DDQN agent to effortlessly control the average delay values below the optimal levels. As depicted in the Average Delay Figure, the agent converges in just 5 episodes without any spikes.

  ## Performance Summary

![Screenshot from 2023-11-20 14-43-40](https://github.com/Abdelkader-gnichi/network-routing-optimization-based-on-DRL-Agents-decisions-in-remote-rural-areas/assets/64486451/1da7cddf-1611-4570-8d68-988c9e93c7b6)

- It is evident that our Dueling DDQN agent outperforms the DDQN agent with a significant difference in terms of convergence time and maintaining the average delay values below the optimal levels within a short timeframe. This indicates that the Dueling DDQN agent consistently identifies the best routes for packet forwarding efficiently. These results affirm that the Dueling DDQN agent is the most effective Deep Reinforcement Learning agent for addressing problems of this nature in our project.
  












