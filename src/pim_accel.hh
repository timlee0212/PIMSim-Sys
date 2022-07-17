#ifndef __PIM_ACCEL_HH__
#define __PIM_ACCEL_HH__

#include "debug/assertions.h"
#include "dev/dma_device.hh"
#include "dev/io_device.hh"
#include "dev/platform.hh"
#include "params/PimAccel.hh"
#include "pimio_driver.hh"
#include "sim/full_system.hh"
#include "sim/system.hh"

#include "src/pim_core.hh"
#include "src/spad_mem.hh"

#include <vector>


namespace gem5 {

// DIRTY IMPLEMENTATION, MIXED SE AND FS IMPLEMENTATION
class PimAccel : public DmaDevice {
   protected:
    const ByteOrder byteOrder = ByteOrder::little;
    // Platform *platform;
    EventFunctionWrapper dmaEvent;
    PIMIODriver *_driver;  // Only used for SE mode
    Addr pioAddr;
    Tick pioDelay;
    Addr pioSize;
    int PimAccelIntID;

    void dmaEventDone();

   private:
    uint8_t *dmaReqData;
    int dmaReqLen = 0;
    //Addr wl_addr = 0;
    Addr hostMemAddr = 0;
    //uint8_t src_op = 0;

    // storage of the crossbar content
    // std::vector<uint8_t> memData;

    // Functional access
    uint64_t devConfigRegRead(const uint8_t addr);
    void devConfigRegWrite(const uint8_t addr, uint64_t val64);
    void executeDevCmd(uint32_t dev_instruction);


   public:
    PARAMS(PimAccel);
    PimAccel(const Params &params);

    // Timing Access
    AddrRangeList getAddrRanges() const override;
    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override;

    // Only used for SE mode and emulated driver
    uint64_t SE_DevConfigRegRead(const uint8_t addr);
    void SE_DevConfigRegWrite(const uint8_t addr, uint64_t val64);
    void SE_ExecDevCmd(uint64_t val64);
    void attachDriver(PIMIODriver *driver);
    PIMIODriver *driver() const;

   protected:
    enum State { Idle, WaitForDMALoad, WaitForDMAStore, WaitForComputing};
    State mainState;

   public:


    void executePimFunction();
    void notifyDone();

   public:
     //void registerSpadMem(SpadMem *spadmem);
     void registerPimCore(PimCore *pimcore);

   private:
    //std::vector<SpadMem*> spadmem;
    std::vector<PimCore*> pimcore;
    SpadMem* spadmem;

    std::vector<uint8_t> memData;



};

}  // namespace gem5

#endif  // __DEV_PIMIO_DEV_HH_
