#ifndef __PIMIO_DRIVER_HH__
#define __PIMIO_DRIVER_HH__
#include "debug/PIMIODev.hh"
#include "mem/request.hh"
#include "params/PIMIODriver.hh"
#include "pimio_dev.hh"
#include "sim/emul_driver.hh"

namespace gem5 {

class PIMIODriver final : public EmulatedDriver {
   private:
    PIMIODev *device;
    ThreadContext *sleep_tc;    //Store the thread that slept. We should only have one suspended thread

   public:
    PARAMS(PIMIODriver);
    PIMIODriver(const Params &params);
    int open(ThreadContext *tc, int mode, int flags) override;
    int ioctl(ThreadContext *tc, unsigned req, Addr ioc_buf) override;
    Addr mmap(ThreadContext *tc, Addr start, uint64_t length, int prot,
              int tgt_flags, int tgt_fd, off_t offset) override;

    void signalWakeup();
};

}  // namespace gem5

#endif