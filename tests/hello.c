#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "pimio.h"

#define TEST_SIZE 128
#define CACHELINE_SIZE 64

#define BOOLSTR(val) (val ? "true" : "false")

int main(void) {
    __uint8_t *srcdata = (__uint8_t *)aligned_alloc(CACHELINE_SIZE, TEST_SIZE * 2 * sizeof(__uint8_t));
    for (int i = 0; i < TEST_SIZE; i++) {
        srcdata[i] = i;
        srcdata[TEST_SIZE+i] = 0;
    }

    int pim_fd = open("/dev/pimio", O_RDWR);
    if (pim_fd < 0) {
        printf("Cannot open device for use!\n");
        exit(-1);
    }
    struct pimio_csr_args csr;
    ioctl(pim_fd, PIMIO_IOCTL_STATUS, &csr);
    printf(
        "Read Back:\n\tPIM Config: %dWords x %d Bytes\n\tStatus: "
        "Busy:%s\n",
        csr.wlSize, csr.blBytes, BOOLSTR(csr.busy));

    printf("=========== Test 1: Loop Back =============\n");
    struct pimio_transfer_args transfer = {
        .hostMemAddr = srcdata, .devMemAddr = 0x10, .dmaReqLen = 256, .hostToDev = true};
    ioctl(pim_fd, PIMIO_IOCTL_TRANSFER, &transfer);
    printf("Copy transfer done!\n");

    transfer.hostMemAddr += 256;
    transfer.hostToDev = false;
    ioctl(pim_fd, PIMIO_IOCTL_TRANSFER, &transfer);

    for (int i = 0; i < 256; i++) {
        printf("Loop back at %d: %d\n", i, srcdata[256 + i]);
    }

    // printf("=========== Test 2: Compute =============\n");

    // //__uint8_t *result = (__uint8_t *)malloc(csr.blBytes * sizeof(__uint8_t));
    // struct pimio_compute_args comp = {
    //     .hostMemAddr = srcdata, .devMemAddr = 0x10, .instruction = 2};
    // ioctl(pim_fd, PIMIO_IOCTL_COMPUTE, &comp);
    // //for (int i = 0; i < csr.blBytes; i++) {
    // //    printf("Compute result at %d: %d\n", i, result[i]);
    // //}
    return 0;
}