# -*- coding: utf-8 -*-
# Copyright (c) 2017 Jason Lowe-Power
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

""" This file creates a barebones system and executes 'hello', a simple Hello
World application. Adds a simple cache between the CPU and the membus.

This config file assumes that the x86 ISA was built.
"""

# import the m5 (gem5) library created when gem5 is built
import argparse
import m5
import os
from m5.defines import buildEnv
# import all of the SimObjects
from m5.objects import (System,
                        Addr,
                        SrcClockDomain,
                        VoltageDomain,
                        AddrRange,
                        TimingSimpleCPU,
                        SystemXBar,
                        SimpleCache,
                        MemCtrl,
                        PimAccel,
                        DDR3_1600_8x8,
                        SEWorkload,
                        Process,
                        Root,
                        AtomicSimpleCPU,
                        IOXBar,
                        Bridge,
                        PIMIODriver,
                        SpadMem,
                        PimCore)
from ruby import Ruby
from common import Options

parser = argparse.ArgumentParser()
Options.addCommonOptions(parser)
Options.addSEOptions(parser)
Ruby.define_options(parser)
args = parser.parse_args()

# Helper Function

def x86IOAddress(port):
    IO_address_space_base = 0x400000000
    return Addr(IO_address_space_base + port)


# create the system we are going to simulate
system = System()

# #IO Subsystem
# system.pc = Pc()


# Set the clock frequency of the system (and all of its children)
system.voltage_domain = VoltageDomain(voltage = args.sys_voltage)
system.clk_domain = SrcClockDomain(clock = args.sys_clock,
                                   voltage_domain = system.voltage_domain)

# Set up the system
system.mem_mode = 'timing'               # Use timing accesses
# Create an address range
system.mem_ranges = [AddrRange('8GB')]

# Create a simple CPU
system.cpu = TimingSimpleCPU()

# create the interrupt controller for the CPU and connect to the membus
system.cpu.createInterruptController()

# Setup our customized IO Device
accel = PimAccel(pio_size=4096, pio_addr=x86IOAddress(
    0x3C0), int_id=0, latency="1ns")
accel.spadMem = SpadMem(wordNum=1024,wordSize=16,bankNum=16)
accel.pimCore = PimCore()
accel.spadMem.dataPort = accel.pimCore.dataPort

pimio_driver = PIMIODriver(filename="pimio", device=accel)

system.piobus = IOXBar()

accel.pio = system.piobus.mem_side_ports
Ruby.create_system(args, None, system, None, [accel.dma], None)
system.ruby.clk_domain = SrcClockDomain(clock = args.ruby_clock,
                                    voltage_domain = system.voltage_domain)
ruby_port = system.ruby._cpu_ports[0]

# Create interrupt controller
system.cpu.createInterruptController()

# Connect cache port's to ruby
system.cpu.icache_port = ruby_port.in_ports
system.cpu.dcache_port = ruby_port.in_ports

ruby_port.mem_request_port = system.piobus.cpu_side_ports
if buildEnv['TARGET_ISA'] == "x86":
    system.cpu.interrupts[0].pio = system.piobus.mem_side_ports
    system.cpu.interrupts[0].int_requestor = \
        system.piobus.cpu_side_ports
    system.cpu.interrupts[0].int_responder = \
        system.piobus.mem_side_ports

# for device in system.devs:
#     device.pio = system.iobus.mem_side_ports
#     if hasattr(device, "dma"):
#         device.dma = system.membus.cpu_side_ports

# Create a process for a simple "Hello World" application
process = Process(drivers=[pimio_driver])
# Set the command
# grab the specific path to the binary
thispath = os.path.dirname(os.path.realpath(__file__))
binpath = os.path.join(thispath, '../',
                       'tests/hello')
# cmd is a list which begins with the executable (like argv)
process.cmd = [binpath]
# Set the cpu to use the process as its workload and create thread contexts
system.cpu.workload = process

# Map the device address to process to bypass driver
system.cpu.createThreads()

system.workload = SEWorkload.init_compatible(binpath)

# set up the root SimObject and start the simulation
root = Root(full_system=False, system=system)
# instantiate all of the objects we've created above
m5.instantiate()

# system.cpu.workload[0].map( Addr(0x200000000), x86IOAddress(0x3C0), int(32), False)
# system.cpu.workload[0].map( Addr(0x1000000), Addr(0x1000000), int(4096), False)

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))
