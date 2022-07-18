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

#include "pimio_dev.hh"
#include "pimio_defines.hh"

#include "debug/PIMIODev.hh"
#include "dev/dma_device.hh"
#include "mem/packet_access.hh"
#include "params/PIMIODev.hh"

namespace gem5
{

PIMIODev::PIMIODev(const Params &params)
    : DmaDevice(params),
      // platform(params.platform),
      dmaEvent([this] { dmaEventDone(); }, name()),
      _driver(nullptr),
      pioAddr(params.pio_addr),
      pioSize(params.pio_size),
      PIMDevIntID(params.int_id),
      n_wl(params.cb_n_wl),
      nbyte_bl(params.cb_nb_bl)
{
    gem5_assert(n_wl * nbyte_bl <= cb_store.max_size(),
                "Selected PIM size exceed possible space for data structure!");
    cb_store.assign(nbyte_bl * n_wl, 0);
    DPRINTF(
        PIMIODev,
        "PIM IO Device initalized!\nPIM Size: %d Bytes x %d words, at %llx\n",
        nbyte_bl, n_wl, pioAddr);
}

void
PIMIODev::dmaEventDone()
{
    int64_t _offset = wl_addr * nbyte_bl;
    if (writeOp)
    {
        memcpy((void *)(cb_store.data() + _offset), (void *)reqData, reqLen);
    }
    delete[] reqData;
    busy = false;
    DPRINTF(PIMIODev, "Done with DMA event\n");
    if (_driver)
    {
        _driver->signalWakeup();
    }
    // platform->postPciInt(PIMDevIntID);
}

uint64_t
PIMIODev::PIMRegRead(const uint8_t addr)
{
    uint64_t r = 0;

    // Each register is 32-bit
    switch (addr >> 2)
    {
        case PIMIO_CONF:
            r = n_wl << 16 | nbyte_bl;
            DPRINTF(PIMIODev, "Read PIMIO_CONF: %d\n", r);
            break;
        case PIMIO_MADDR:
            r = mem;
            DPRINTF(PIMIODev, "Read PIMIO_MADDR: %#llx\n", r);
            break;
        case PIMIO_PADDR:
            r = wl_addr;
            DPRINTF(PIMIODev, "Read PIMIO_PADDR: %#llx\n", r);
            break;
        case PIMIO_TLEN:
            r = reqLen;
            DPRINTF(PIMIODev, "Read PIMIO_TLEN: %d\n", r);
            break;
        case PIMIO_SRC_OP:
            r = src_op;
            DPRINTF(PIMIODev, "Read PIMIO_SRC_OP: %d\n", r);
            break;

        case PIMIO_STAT:
            r = busy;
            if (writeOp)
            {
                r |= PIMIO_DEV_TYPE;  // Write command
            }
            if (err)
            {
                r |= PIMIO_DEV_ERRR;  // Error
            }
            DPRINTF(PIMIODev, "Read PIMIO_DEV_STAT: %d\n", r);

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
    return r;
}

void
PIMIODev::PIMRegWrite(const uint8_t addr, uint64_t val64)
{
    uint32_t val = val64;

    switch (addr >> 2)
    {
        case PIMIO_MADDR:
            mem = val;
            DPRINTF(PIMIODev, "Write PIMIO_MADDR: %#llx\n", val);
            break;
        case PIMIO_PADDR:
            wl_addr = val;
            DPRINTF(PIMIODev, "Write PIMIO_PADDR: %#llx\n", val);
            break;
        case PIMIO_TLEN:
            reqLen = val;
            DPRINTF(PIMIODev, "WritePIMIO_TLEN: %d\n", val);
            break;
        case PIMIO_SRC_OP:
            src_op = val;
            DPRINTF(PIMIODev, "Write PIMIO_SRC_OP: %d\n", val);
            break;

        case PIMIO_CTL:
            // Perform command
            if (!busy)
            {
                err = false;
                compOp = val & PIMIO_CMD_COMP;
                writeOp = (val & PIMIO_DEV_TYPE) && !compOp;
                PIMCmd();
            }
            else
            {
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
PIMIODev::PIMCmd(void)
{
    // Check parameters
    if ((uint16_t)wl_addr > n_wl)
    {
        panic("Bad Wordline Address!\n");
    }
    else if (!compOp && reqLen % nbyte_bl != 0)
    {
        panic("Read/Write length must align with the crossbar width!\n");
    }
    DPRINTF(PIMIODev, "Execute Command, Write:%s, Compute:%s\n",
            writeOp ? "true" : "false", compOp ? "true" : "false");
    // Device is busy when a transfer occurs
    busy = true;

    // Perform transfer
    reqData = new uint8_t[reqLen];
    int64_t _offset = nbyte_bl * wl_addr;

    // TODO: Modify to correct PIM behavior. Currently we assume 8bit uint
    // integer multiply each 8-bit uint stored in crossbar
    if (compOp)
    {
        for (int i = 0; i < nbyte_bl; i++)
        {
            reqData[i] = static_cast<uint8_t>(cb_store[_offset + i] * src_op);
        }
    }
    else if (!writeOp)
    {
        memcpy((void *)reqData, (void *)(cb_store.data() + _offset), reqLen);
    }
    if (!writeOp)
    {
        // Read command (block -> mem)
        dmaWrite(mem, reqLen, &dmaEvent, reqData, cyclesToTicks(Cycles(5)));
    }
    else
    {
        // Write command (mem -> block)
        dmaRead(mem, reqLen, &dmaEvent, reqData, cyclesToTicks(Cycles(5)));
    }
}

AddrRangeList
PIMIODev::getAddrRanges() const
{
    AddrRangeList ranges = {RangeSize(pioAddr, pioSize)};
    return ranges;
}

Tick
PIMIODev::read(PacketPtr pkt)
{
    Addr addr = pkt->getAddr() - pioAddr;

    DPRINTF(PIMIODev, "Read request - addr: %#x, size: %#x\n", addr,
            pkt->getSize());

    uint64_t read_request = PIMRegRead(addr);
    DPRINTF(PIMIODev, "Packet Read: %d\n", read_request);
    pkt->setUintX(read_request, byteOrder);
    pkt->makeResponse();

    return pioDelay;
}

Tick
PIMIODev::write(PacketPtr pkt)
{
    Addr daddr = pkt->getAddr() - pioAddr;

    DPRINTF(PIMIODev, "Write register %#x value %#x\n", daddr,
            pkt->getUintX(byteOrder));

    PIMRegWrite(daddr, pkt->getUintX(byteOrder));
    DPRINTF(PIMIODev, "Packet Write Value: %d\n", pkt->getUintX(byteOrder));

    pkt->makeResponse();

    return pioDelay;
}

// SE Functionalities
uint64_t
PIMIODev::SE_RegRead(const uint8_t addr)
{
    gem5_assert(!FullSystem,
                "This function should only be accessed in SE mode.");
    return PIMRegRead(addr);
}
void
PIMIODev::SE_RegWrite(const uint8_t addr, uint64_t val64)
{
    gem5_assert(!FullSystem,
                "This function should only be accessed in SE mode.");
    PIMRegWrite(addr, val64);
}
void
PIMIODev::SE_PIMCmd(bool write, bool compute)
{
    gem5_assert(!FullSystem,
                "This function should only be accessed in SE mode.");
    if (!busy)
    {
        err = false;
        compOp = compute;
        writeOp = write;
        PIMCmd();
    }
    else
    {
        panic(
            "Attempting to write to PIMIO device while transfer"
            " is ongoing!\n");
    }
}

void
PIMIODev::attachDriver(PIMIODriver *driver)
{
    fatal_if(_driver, "Should not overwrite driver.");
    _driver = driver;
    assert(_driver);
}

PIMIODriver *
PIMIODev::driver() const
{
    return _driver;
}

}  // namespace gem5
