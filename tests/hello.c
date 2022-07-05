#include <stdio.h>

#define PIM_IO_ADDR 0x200000000
#define ReadReg(reg_id) (*((__uint32_t *)PIM_IO_ADDR+reg_id))
#define WriteReg(reg_id, val) ((*((__uint32_t *)PIM_IO_ADDR+reg_id))=(__uint32_t)val)
#define PIM_READ 0x0
#define PIM_WRITE 0x2
#define PIM_COMP 0x1

#define PIM_BUSY 0x1
#define PIM_ERRR 0x4

enum {
    PIMIO_CONF,
    PIMIO_MADDR,
    PIMIO_PADDR,
    PIMIO_TLEN,
    PIMIO_SRC_OP,

    PIMIO_CTL,
    PIMIO_STAT
};

int main(void) {
    __uint8_t *srcdata = (__uint8_t *)0x1000000;
    for(int i =0; i<512; i++)
    {
        *(srcdata+i) = i;
    }

    printf("Buffer Addr: %x\n", srcdata);
    WriteReg(PIMIO_MADDR, 0x1000000);
    WriteReg(PIMIO_PADDR, 16);
    WriteReg(PIMIO_TLEN, 256);
    WriteReg(PIMIO_CTL, PIM_WRITE);
    while(ReadReg(PIMIO_STAT)&PIM_BUSY){}
    WriteReg(PIMIO_MADDR, 0x1000000 + 256);
    WriteReg(PIMIO_CTL, PIM_READ);
    while(ReadReg(PIMIO_STAT)&PIM_BUSY){}
    for(int i =0; i<256; i++)
    {
       printf("ReadBack Data at %d: %d", i, *(srcdata+256+i));
    }
    printf("Test Read PIM_PADDR: %d\n", ReadReg(PIMIO_PADDR));
    return 0;
}