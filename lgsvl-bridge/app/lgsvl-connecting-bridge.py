#!/usr/bin/env python3
#
# Copyright (c) 2019-2021 LG Electronics, Inc.
#
# This software contains code licensed as described in third-party-licenses/lgsvl_connecting_bridge_LICENSE

import time
from environs import Env
import lgsvl

print("LGSVL: Connecting to a bridge")
env = Env()

sim = lgsvl.Simulator(
    env.str("LGSVL__SIMULATOR_HOST", lgsvl.wise.SimulatorSettings.simulator_host),
    env.int("LGSVL__SIMULATOR_PORT", lgsvl.wise.SimulatorSettings.simulator_port))

# load AutonomousStuff scene
if sim.current_scene == '2aae5d39-a11c-4516-87c4-cdc9ca784551':
    sim.reset()
else:
    sim.load('2aae5d39-a11c-4516-87c4-cdc9ca784551')

spawns = sim.get_spawn()

state = lgsvl.AgentState()
state.transform = spawns[0]

# Lexus RX400 vehicle by default
ego = sim.add_agent(
    env.str("LGSVL__VEHICLE_0", '12d47257-a929-4077-827f-a94a42830cfd'),
    lgsvl.AgentType.EGO,
    state)

# An EGO will not connect to a bridge unless commanded to
print("Bridge connected:", ego.bridge_connected)

# The EGO is now looking for a bridge at the specified IP and port
ego.connect_bridge(
    env.str("LGSVL__AUTOPILOT_0_HOST", lgsvl.wise.SimulatorSettings.bridge_host),
    env.int("LGSVL__AUTOPILOT_0_PORT", lgsvl.wise.SimulatorSettings.bridge_port))

print("Waiting for connection...")

while not ego.bridge_connected:
    time.sleep(1)

print("Bridge connected:", ego.bridge_connected)
print("Running the simulation until exited with ctrl-c")

sim.run()
