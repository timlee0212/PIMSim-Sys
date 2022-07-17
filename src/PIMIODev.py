
from m5.objects.Device import DmaDevice
from m5.objects.Process import EmulatedDriver

from m5.params import Param
from m5.proxy import Parent


class PIMIODriver(EmulatedDriver):
    type = 'PIMIODriver'
    cxx_class = 'gem5::PIMIODriver'
    cxx_header = 'src/pimio_driver.hh'
    device = Param.PIMIODev("PIMIO Device controlled by this driver.")
    # No parameters for now


class PIMIODev(DmaDevice):

    type = 'PIMIODev'
    cxx_class = 'gem5::PIMIODev'
    cxx_header = 'src/pimio_dev.hh'
    pio_size = Param.Addr(0x1000, "PIO Size")
    pio_addr = Param.Addr("Device Address")
    cb_nb_bl = Param.Int("Number of bytes of bitlines.")
    cb_n_wl = Param.Int("Number of wordlines.")
    latency = Param.Latency('0ns', "DMA Device Latency")
    # platform = Param.Platform(Parent.any,
    #                          "Platform this device is part of.")
    int_id = Param.Int("Interrupt ID for the PIC to use")
