#ifndef __DEV_PIMIO_DEV_HH__
#define __DEV_PIMIO_DEV_HH__

#include "debug/PIMIODev.hh"
#include "debug/assertions.h"
#include "dev/dma_device.hh"
#include "dev/io_device.hh"
#include "dev/platform.hh"
#include "params/PIMIODev.hh"
#include "pimio_driver.hh"
#include "sim/full_system.hh"
#include "sim/system.hh"

namespace gem5 {

// DIRTY IMPLEMENTATION, MIXED SE AND FS IMPLEMENTATION
class PIMIODev : public DmaDevice {
   protected:
    const ByteOrder byteOrder = ByteOrder::little;
    // Platform *platform;
    EventFunctionWrapper dmaEvent;
    PIMIODriver *_driver;  // Only used for SE mode
    Addr pioAddr;
    Tick pioDelay;
    Addr pioSize;
    int PIMDevIntID;
    uint16_t n_wl;
    uint16_t nbyte_bl;

    void dmaEventDone();

    // Register map
   private:
    uint8_t *reqData;

    int reqLen = 0;
    Addr wl_addr = 0;
    Addr mem = 0;
    uint8_t src_op = 0;

    // storage of the crossbar content
    std::vector<uint8_t> cb_store;

    bool writeOp = false;
    bool err = false;
    bool busy = false;
    bool compOp = false;

    // Functional access
    uint64_t PIMRegRead(const uint8_t addr);
    void PIMRegWrite(const uint8_t addr, uint64_t val64);
    void PIMCmd(void);

   public:
    PARAMS(PIMIODev);
    PIMIODev(const Params &params);

    // Timing Access
    AddrRangeList getAddrRanges() const override;
    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override;

    // Only used for SE mode and emulated driver
    uint64_t SE_RegRead(const uint8_t addr);
    void SE_RegWrite(const uint8_t addr, uint64_t val64);
    void SE_PIMCmd(bool write, bool compute);
    void attachDriver(PIMIODriver *driver);
    PIMIODriver *driver() const;
};

}  // namespace gem5

#endif  // __DEV_PIMIO_DEV_HH_
