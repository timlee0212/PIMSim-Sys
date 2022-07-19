/*
 * Copyright (c) 2021 The Regents of the University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "pim_accel.hh"

#include "debug/PimAccel.hh"
#include "dev/dma_device.hh"
#include "mem/packet_access.hh"
#include "params/PimAccel.hh"
#include "pimio_defines.hh"

namespace gem5 {

PimAccel::PimAccel(const Params &params)
    : DmaDevice(params),
      // platform(params.platform),
      dmaEvent([this] { dmaEventDone(); }, name()),
      _driver(nullptr),
      pioAddr(params.pio_addr),
      pioSize(params.pio_size),
      PimAccelIntID(params.int_id),
      mainState(Idle){
    DPRINTF(PimAccel,"PIM accelerator initalized!");
        dmaBuffer.assign(sys->cacheLineSize(),0);
    DPRINTF(PimAccel,"DMA Buffer Size %d,\n",dmaBuffer.size());        
}

void 
PimAccel::dmaEventDone() 
{
    DPRINTF(PimAccel, "DMA Event Done is notified.\n");
    if (mainState==WaitForDMALoad) {
        //memcpy((void *)(dmaBuffer.data()), (void *)dmaReqData, dmaReqLen);
        Addr addr = 0;
        DPRINTF(PimAccel, "Finish data transfer from host to device.\n");     
        spadmem->accessFunctional(addr, dmaReqLen, dmaBuffer.data(), false);       
    }
    mainState = Idle;
    DPRINTF(PimAccel, "Done with DMA event\n");
    if (!FullSystem) {
        _driver->signalWakeup();
    }
}

uint64_t
PimAccel::devConfigRegRead(const uint8_t addr) 
{
    uint64_t dataRead = 0;
    // Each register is 32-bit
    switch (addr >> 2) {
        case PIMIO_CONF:
            dataRead = 0; ///
            DPRINTF(PimAccel, "Read PIMIO_CONF: %d\n", dataRead);
            break;
        case PIMIO_MADDR:
            dataRead = hostMemAddr;
            DPRINTF(PimAccel, "Read PIMIO_MADDR: %#llx\n", dataRead);
            break;
        case PIMIO_PADDR:
            dataRead = 1;
            DPRINTF(PimAccel, "Read PIMIO_PADDR: %#llx\n", dataRead);
            break;
        case PIMIO_TLEN:
            dataRead = dmaReqLen;
            DPRINTF(PimAccel, "Read PIMIO_TLEN: %d\n", dataRead);
            break;
        case PIMIO_SRC_OP:
            dataRead = 20;
            DPRINTF(PimAccel, "Read PIMIO_SRC_OP: %d\n", dataRead);
            break;
        case PIMIO_STAT:
            dataRead = (mainState!=Idle);
            DPRINTF(PimAccel, "Read PIMIO_DEV_STAT: %d\n", dataRead);
            // Acknowledge IRQ
            // platform->clearPciInt(PIMDevIntID);
            break;
        default:
            panic(
                "Unexpected read to the PIMIO device at address"
                " %#llx!\n",
                addr);
            break;
    }
    return dataRead;
}

void
PimAccel::devConfigRegWrite(const uint8_t addr, uint64_t val64) 
{
    uint32_t incoming_value = val64;
    switch (addr >> 2) {
        case PIMIO_MADDR:
            hostMemAddr = incoming_value;
            DPRINTF(PimAccel, "Write PIMIO_MADDR: %#llx\n", incoming_value);
            break;
        case PIMIO_PADDR:
            DPRINTF(PimAccel, "Write PIMIO_PADDR: %#llx\n", incoming_value);
            break;
        case PIMIO_TLEN:
            dmaReqLen = incoming_value;
            DPRINTF(PimAccel, "WritePIMIO_TLEN: %d\n", incoming_value);
            break;
        case PIMIO_SRC_OP:
            DPRINTF(PimAccel, "Write PIMIO_SRC_OP: %d\n", incoming_value);
            break;
        case PIMIO_CTL:
            // Perform command
            if (mainState==Idle) {
                DPRINTF(PimAccel, "Execute a PIM instruction: %d\n", incoming_value);
                executeDevCmd(incoming_value);
            } else {
                panic(
                    "Attempting to write to PIMIO device while transfer"
                    " is ongoing!\n");
            }
            break;
        default:
            panic(
                "Unexpected write to the PIMIO device at address"
                " %#llx!\n",
                addr);
            break;
    }
}

void
PimAccel::executeDevCmd(uint32_t dev_instruction)
{   
    switch(dev_instruction){
        case PIMIO_CMD_COMP:
            mainState = WaitForComputing;
            DPRINTF(PimAccel, "Start to trigger PIM function.\n");              
            executePimFunction();
            break;
        case PIMIO_CMD_STORE:
            mainState = WaitForDMAStore;
            DPRINTF(PimAccel, "Execute a DMA Store Command to hostMemAddr:%d with length of %d.\n",hostMemAddr,dmaReqLen);            
            dmaWrite(hostMemAddr, dmaReqLen, &dmaEvent, dmaBuffer.data(), 0);
            break;
        case PIMIO_CMD_LOAD:
            mainState = WaitForDMALoad;
            DPRINTF(PimAccel, "Execute a DMA Load Command from hostMemAddr:%d with length of %d.\n",hostMemAddr,dmaReqLen);            
            dmaRead(hostMemAddr, dmaReqLen, &dmaEvent, dmaBuffer.data(), 0);
            break;
        default:
            panic(
                "Unsupported DMA Instruction.\n"
            );
            break;                        
    }
}

void
PimAccel::executePimFunction(){
    if (pimcore[0]->isIdle()) {
        DPRINTF(PimAccel, "Triggerred pim function.\n");        
        pimcore[0]->start();
    }    

}

void 
PimAccel::notifyDone() {
    if (mainState == WaitForComputing) {
        DPRINTF(PimAccel, "Received interupt for pim function execution.\n");
        mainState = Idle;
        if (!FullSystem) {
            _driver->signalWakeup();
        }
    }
}

void 
PimAccel::registerPimCore(PimCore *pimcore){
    this->pimcore.push_back(pimcore);
}

AddrRangeList 
PimAccel::getAddrRanges() const 
{
    AddrRangeList ranges = {RangeSize(pioAddr, pioSize)};
    return ranges;
}

Tick 
PimAccel::read(PacketPtr pkt) 
{
    Addr addr = pkt->getAddr() - pioAddr;

    DPRINTF(PimAccel, "Read request - addr: %#x, size: %#x\n", addr,
            pkt->getSize());

    uint64_t read_request = devConfigRegRead(addr);
    DPRINTF(PimAccel, "Packet Read: %d\n", read_request);
    pkt->setUintX(read_request, byteOrder);
    pkt->makeResponse();

    return pioDelay;
}

Tick 
PimAccel::write(PacketPtr pkt) 
{
    Addr daddr = pkt->getAddr() - pioAddr;
    DPRINTF(PimAccel, "Write register %#x value %#x\n", daddr,
            pkt->getUintX(byteOrder));

    devConfigRegWrite(daddr, pkt->getUintX(byteOrder));
    DPRINTF(PimAccel, "Packet Write Value: %d\n", pkt->getUintX(byteOrder));

    pkt->makeResponse();

    return pioDelay;
}

// SE Functionalities
uint64_t 
PimAccel::SE_DevConfigRegRead(const uint8_t addr) 
{
    gem5_assert(!FullSystem,
                "This function should only be accessed in SE mode.");
    return devConfigRegRead(addr);
}
void 
PimAccel::SE_DevConfigRegWrite(const uint8_t addr, uint64_t val64) 
{
    gem5_assert(!FullSystem,
                "This function should only be accessed in SE mode.");
    devConfigRegWrite(addr, val64);
}

void 
PimAccel::SE_ExecDevCmd(uint64_t val64) 
{
    gem5_assert(!FullSystem,
        "This function should only be accessed in SE mode.");
    uint32_t devInstruction = val64;
    executeDevCmd(devInstruction);
}

void 
PimAccel::attachDriver(PIMIODriver *driver) 
{
    fatal_if(_driver, "Should not overwrite driver.");
    _driver = driver;
    assert(_driver);
}

PIMIODriver *PimAccel::driver() const { return _driver; }

}  // namespace gem5
