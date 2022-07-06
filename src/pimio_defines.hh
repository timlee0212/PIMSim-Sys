#ifndef __PIMIO_DEFINES_HH__
#define __PIMIO_DEFINES_HH__

namespace gem5 {
/* Shared fields for CTRL and STAT registers */
#define PIMIO_DEV_TYPE 0x2
#define PIMIO_CMD_COMP 0x1

/* Specific fields for CTRL */
#define PIMIO_DEV_IRQE 0x1

/* Specific fields for STAT */
#define PIMIO_DEV_BUSY 0x1
#define PIMIO_DEV_ERRR 0x4

// IOCTL Requests
// Naive encoding, DID NOT FOLLOW THE INSTRUCTION IN IOCTL DOC!
#define PIMIO_IOCTL_TRANSFER 0x00
#define PIMIO_IOCTL_COMPUTE 0x01
#define PIMIO_IOCTL_STATUS 0x02
// Structure for device registers
struct pimio_transfer_args {
    uint64_t memAddr;
    uint16_t wlAddr;
    int reqLen;
    bool writeOp;
};

struct pimio_compute_args {
    uint64_t memAddr;
    uint16_t wlAddr;
    uint8_t srcOp;
};

struct pimio_csr_args {
    bool error;
    bool busy;
    uint16_t wlSize;
    uint16_t blBytes;
};

// Register Map
enum {
    PIMIO_CONF,
    PIMIO_MADDR,
    PIMIO_PADDR,
    PIMIO_TLEN,
    PIMIO_SRC_OP,

    PIMIO_CTL,
    PIMIO_STAT
};

}  // namespace gem5

#endif