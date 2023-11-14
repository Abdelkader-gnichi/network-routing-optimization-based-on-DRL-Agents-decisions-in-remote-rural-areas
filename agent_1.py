#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import pfrl
import torch
import gym
import argparse
from pfrl.q_function import StateQFunction
import torch.nn.functional as F
from pfrl.initializers import init_chainer_default
from pfrl.nn.mlp import MLP
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from ns3gym import ns3env

parser = argparse.ArgumentParser(description="start simulation script on/off")
parser.add_argument('--start', type= int,default=1,help="start the ns-3 simulation script 0/1, default=1")
parser.add_argument('--total_episodes', type=int ,default=100, help="the total episodes number  default= 100 episodes")
parser.add_argument('--total_steps', type=int, default=300, help="total steps number in each episode, default= 300 steps")
parser.add_argument('--agent_name',type="str",default="Dueling DDQN"
                    ,help="choose the DRL agent that will give decisions for the simulation, "
                     "DRL agents: Dueling DDQN | Dueling DDQN-MLPS | DDQN, default= Dueling DDQN")

args = parser.parse_args()
startSim = bool(args.start)
total_episodes = args.total_episodes
total_steps = args.total_steps
agent_name= args.agent_name

port = 5555

# env = gym.make('ns3-v0',port=port)
env = ns3env.Ns3Env(port=port,startSim=startSim)
env.reset() 

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.n)

# obs_size must be 4 since we inject the delay value in observation vector,
# but we'll not use it for the observation
obs_size = 4
n_actions = env.action_space.n

#Dueling DDQN QFunction
class DuelingDDQNQFunction(torch.nn.Module):
    def __init__(self, obs_size, n_actions):
        super().__init__()
        self.l1 = torch.nn.Linear(obs_size, 50)
        self.l2 = torch.nn.Linear(50, 50)
        self.l3_v = torch.nn.Linear(50, 1)  # State value function stream
        self.l3_a = torch.nn.Linear(50, n_actions)  # Advantage function stream

    def forward(self, x):
        h = x
        h = torch.nn.functional.relu(self.l1(h))
        h = torch.nn.functional.relu(self.l2(h))

        v = self.l3_v(h)
        a = self.l3_a(h)

        q = v + a - a.mean(dim=1, keepdim=True)
        return pfrl.action_value.DiscreteActionValue(q)

# bias initializer for the FC layers
def constant_bias_initializer(bias=0.0):
    @torch.no_grad()
    def init_bias(m):
        if isinstance(m, torch.nn.Linear):
            m.bias.fill_(bias)

    return init_bias

#Dueling DDQN with MLP
class DuelingDQNWithMLP(torch.nn.Module, StateQFunction):
    """Dueling Q-Network

    See: http://arxiv.org/abs/1511.06581
    """

    def __init__(self, n_actions, n_input_elements, activation=F.relu, bias=0.1):
        self.n_actions = n_actions
        self.n_input_elements = n_input_elements
        self.activation = activation
        self.bias = bias
        super().__init__()

        # Define the linear layers to replace the convolutional layers
        self.fc_layers = torch.nn.ModuleList(
            [
                torch.nn.Linear(n_input_elements, 50),
                torch.nn.Linear(50, 50),
                torch.nn.Linear(50, 50),
            ]
        )

        # MLPs for the advantage and value streams
        self.a_stream = MLP(50, n_actions, [25])  # Update the input size to 64
        self.v_stream = MLP(50, 1, [25])

        # Apply initialization and bias to the linear layers
        self.fc_layers.apply(init_chainer_default)  # MLP already applies
        self.fc_layers.apply(constant_bias_initializer(bias=self.bias))

    def forward(self, x):
        h = x

        # Pass through the linear layers with activation
        for layer in self.fc_layers:
            h = self.activation(layer(h))

        # Advantage
        batch_size = x.shape[0]
        h = h.view(batch_size, -1)  # Reshape instead of flatten
        ya = self.a_stream(h)
        mean = torch.reshape(torch.sum(ya, dim=1) / self.n_actions, (batch_size, 1))
        ya, mean = torch.broadcast_tensors(ya, mean)
        ya -= mean

        # State value
        ys = self.v_stream(h)

        ya, ys = torch.broadcast_tensors(ya, ys)
        q = ya + ys
        return pfrl.action_value.DiscreteActionValue(q)
    
# DDQN QFunction
class DDQNQFunction(torch.nn.Module):

     def __init__(self, obs_size, n_actions):
         super().__init__()
         self.l1 = torch.nn.Linear(obs_size, 50)
         self.l2 = torch.nn.Linear(50, 50)
         self.l3 = torch.nn.Linear(50, n_actions)

     def forward(self, x):
         h = x
         h = torch.nn.functional.relu(self.l1(h))
         h = torch.nn.functional.relu(self.l2(h))
         h = self.l3(h)
         return pfrl.action_value.DiscreteActionValue(h)

if agent_name == "Dueling DDQN":
    q_func = DuelingDDQNQFunction(obs_size, n_actions)
elif agent_name == "Dueling DDQN-MLPS":
    q_func = DuelingDQNWithMLP(n_actions=n_actions,n_input_elements=obs_size,activation=torch.nn.functional.relu, bias=0.1)
else:
    q_func= DDQNQFunction(obs_size, n_actions)

# Agent parameters and components setup
optimizer = torch.optim.Adam(q_func.parameters(), eps=1e-2)
# Set the discount factor that discounts future rewards.
gamma = 0.9

# Use epsilon-greedy for exploration
explorer = pfrl.explorers.LinearDecayEpsilonGreedy(
    start_epsilon=1.0, end_epsilon=0.0, decay_steps=2500, random_action_func=env.action_space.sample)
# Specify a replay buffer and its capacity.
replay_buffer = pfrl.replay_buffers.ReplayBuffer(capacity=10 ** 6)

# Since observations from ns3 simulation is numpy.float64 while
# As PyTorch only accepts numpy.float32 by default, specify
# a converter as a feature extractor function phi.
phi = lambda x: x.astype(np.float32, copy=False)

# Set the device id to use GPU. To use CPU only, set it to -1.
# gpu = -1

#Now create an agent that will interact with the environment 
#based on Double Learning and the selected QFunction.
agent = pfrl.agents.DoubleDQN(
    q_func,
    optimizer,
    replay_buffer,
    gamma,
    explorer,
    replay_start_size=128,
    update_interval=1,
    target_update_interval=10,
    minibatch_size=128,
    phi=phi,
    # gpu=gpu,
)

delay_history = []
rew_history = []

env._max_episode_steps = total_steps

for i in range(1, total_episodes + 1):
    obs = env.reset()
    
    delaySum = 0 # return (sum of delaySum)
    obs = obs[:4]
    print("reset State:",obs)
    rewardSum = 0  # return (sum of rewards)

    for step in range(1, total_steps + 1):
        
        action = agent.act(obs)
        obs, reward, done, _ = env.step(action)
        delaySum += obs[4]
        obs = obs[:4]
        rewardSum += reward
        reset = step == total_steps # testing if the steps reach the max value
        agent.observe(obs, reward, done, reset)

        if done:
            break

    avg_delay = delaySum / step 
    delay_history.append(avg_delay/int(_)) 
    rew_history.append(rewardSum)  
   
    if i % 10 == 0:
        print('episode:', i, 'R:', rewardSum)
        print("episode: {}/{}, time: {}, rew: {}, eps: {}"
                  .format(i, total_episodes, step, rewardSum, agent.explorer.__dict__.values()))
    if i % 50 == 0:
        print('statistics:', agent.get_statistics())
    
print('Finished.')

agent.save("/home/gadour/Blobs")

print("Plot Learning Performance")
mpl.rcdefaults()
mpl.rcParams.update({'font.size': 16})
# Create a figure with two subplots
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)
# Plot 1: Learning Performance (Reward)
ax1.plot(range(len(rew_history)), rew_history, label='Reward', marker="", color='red')
ax1.set_ylabel('Reward Sum')
ax1.legend(prop={'size': 12})

# Plot 2:  ( delaySum)
ax2.plot(range(len(delay_history)), delay_history, label='delay', marker="", color="blue")
ax2.set_ylabel('Average Delay')
ax2.legend(prop={'size': 12})

# Common X-axis label for both plots
ax2.set_xlabel('Episode')

# Adjust layout and save the plot to file
plt.tight_layout()
plt.savefig(f'{agent_name}-results.pdf', bbox_inches='tight')

# Show the plots
plt.show()

# ns3gym 0.1.0 requires protobuf==3.20.3, but you have protobuf 4.21.12 which is incompatible.   pip
