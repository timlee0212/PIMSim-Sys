#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "pimio.h"

#define BOOLSTR(val) (val ? "true" : "false")

int main(void) {
    __uint8_t *srcdata = (__uint8_t *)malloc(512 * sizeof(__uint8_t));
    for (int i = 0; i < 256; i++) {
        srcdata[i] = i;
        srcdata[256 + i] = 0;
    }

    int pim_fd = open("/dev/pimio", O_RDWR);
    if (pim_fd < 0) {
        printf("Cannot open device for use!\n");
        exit(-1);
    }
    struct pimio_csr_args csr;
    ioctl(pim_fd, PIMIO_IOCTL_STATUS, &csr);
    printf(
        "Read Back:\n\tPIM Config: %dWords x %d Bytes\n\tStatus: Error:%s, "
        "Busy:%s\n",
        csr.wlSize, csr.blBytes, BOOLSTR(csr.error), BOOLSTR(csr.busy));

    printf("=========== Test 1: Loop Back =============\n");
    struct pimio_transfer_args transfer = {
        .memAddr = srcdata, .wlAddr = 0x10, .reqLen = 256, .writeOp = true};
    ioctl(pim_fd, PIMIO_IOCTL_TRANSFER, &transfer);
    printf("Copy transfer done!\n");

    transfer.memAddr += 256;
    transfer.writeOp = false;
    ioctl(pim_fd, PIMIO_IOCTL_TRANSFER, &transfer);

    for (int i = 0; i < 256; i++) {
        printf("Loop back at %d: %d\n", i, srcdata[256 + i]);
    }

    printf("=========== Test 2: Compute =============\n");
    __uint8_t *result = (__uint8_t *)malloc(csr.blBytes * sizeof(__uint8_t));
    struct pimio_compute_args comp = {
        .memAddr = result, .wlAddr = 0x10, .srcOp = 2};
    ioctl(pim_fd, PIMIO_IOCTL_COMPUTE, &comp);
    for (int i = 0; i < csr.blBytes; i++) {
        printf("Compute result at %d: %d\n", i, result[i]);
    }
    return 0;
}