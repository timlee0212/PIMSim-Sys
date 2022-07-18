#include "pimio_driver.hh"
#include "pimio_defines.hh"

#include "arch/x86/page_size.hh"
#include "cpu/thread_context.hh"
#include "debug/PIMIODev.hh"
#include "mem/page_table.hh"
#include "mem/se_translating_port_proxy.hh"
#include "mem/translating_port_proxy.hh"
#include "params/PIMIODriver.hh"
#include "sim/full_system.hh"
#include "sim/process.hh"
#include "sim/proxy_ptr.hh"
#include "sim/se_workload.hh"
#include "sim/syscall_emul_buf.hh"

namespace gem5
{
PIMIODriver::PIMIODriver(const Params &params)
    : EmulatedDriver(params), device(params.device), sleep_tc(nullptr)
{
    device->attachDriver(this);
}

int
PIMIODriver::open(ThreadContext *tc, int mode, int flags)
{
    DPRINTF(PIMIODev, "Driver called to initialized for: %s\n", filename);
    auto process = tc->getProcessPtr();
    auto device_fd_entry = std::make_shared<DeviceFDEntry>(this, filename);
    int tgt_fd = process->fds->allocFD(device_fd_entry);
    return tgt_fd;
}
int
PIMIODriver::ioctl(ThreadContext *tc, unsigned req, Addr ioc_buf)
{
    TranslatingPortProxy fs_proxy(tc);
    SETranslatingPortProxy se_proxy(tc);
    PortProxy &virt_proxy = FullSystem ? fs_proxy : se_proxy;
    auto process = tc->getProcessPtr();
    auto mem_state = process->memState;

    switch (req)
    {
        case PIMIO_IOCTL_TRANSFER:
        {
            TypedBufferArg<pimio_transfer_args> args(ioc_buf);
            args.copyIn(virt_proxy);
            // Obtain the physical address (no delay here)
            Addr paddr = 0;
            process->pTable->translate(args->memAddr, paddr);
            DPRINTF(
                PIMIODev,
                "Data Transfer Command: mem:%#x (Phys: %#x), wl:%#x, size: "
                "%dB, Write:%d",
                args->memAddr, paddr, args->wlAddr, args->reqLen,
                args->writeOp);
            device->SE_RegWrite(PIMIO_MADDR << 2, paddr);
            device->SE_RegWrite(PIMIO_PADDR << 2, args->wlAddr);
            device->SE_RegWrite(PIMIO_TLEN << 2, args->reqLen);
            device->SE_PIMCmd(args->writeOp, false);
            assert(!sleep_tc);
            tc->suspend();
            sleep_tc = tc;
            break;
        }
        case PIMIO_IOCTL_COMPUTE:
        {
            TypedBufferArg<pimio_compute_args> args(ioc_buf);
            args.copyIn(virt_proxy);
            Addr paddr = 0;
            process->pTable->translate(args->memAddr, paddr);
            device->SE_RegWrite(PIMIO_MADDR << 2, paddr);
            device->SE_RegWrite(PIMIO_PADDR << 2, args->wlAddr);
            device->SE_RegWrite(PIMIO_SRC_OP << 2, args->srcOp);
            // Fix for one wordline size
            // FIXME, Dirty implementation
            auto pim_conf = device->SE_RegRead(PIMIO_CONF << 2);
            device->SE_RegWrite(PIMIO_TLEN << 2, pim_conf & 0xFFFF);

            device->SE_PIMCmd(false, true);
            assert(!sleep_tc);
            tc->suspend();
            sleep_tc = tc;
            break;
        }
        case PIMIO_IOCTL_STATUS:
        {
            TypedBufferArg<pimio_csr_args> args(ioc_buf);
            args.copyIn(virt_proxy);
            auto status = device->SE_RegRead(PIMIO_STAT << 2);
            args->busy = status & PIMIO_DEV_BUSY;
            args->error = status & PIMIO_DEV_ERRR;
            auto pim_conf = device->SE_RegRead(PIMIO_CONF << 2);
            args->blBytes = pim_conf & 0xFFFF;
            args->wlSize = (pim_conf >> 16) & 0xFFFF;
            args.copyOut(virt_proxy);
            break;
        }
        default:
            panic("Unrecognized IOCTL Request.");
            break;
    }
    // Not following IOCTL
    return 0;
}
Addr
PIMIODriver::mmap(ThreadContext *tc, Addr start, uint64_t length, int prot,
                  int tgt_flags, int tgt_fd, off_t offset)
{
    auto process = tc->getProcessPtr();
    auto mem_state = process->memState;
    start = mem_state->extendMmap(length);
    // TODO: Currently
    int npages = divCeil(length, (int64_t)TheISA::PageBytes);
    auto pa_addr = process->seWorkload->allocPhysPages(npages);
    process->pTable->map(start, pa_addr, length,
                         EmulationPageTable::MappingFlags::Uncacheable);
    return start;
}

void
PIMIODriver::signalWakeup()
{
    assert(sleep_tc);
    sleep_tc->activate();
    sleep_tc = nullptr;
}

}  // namespace gem5