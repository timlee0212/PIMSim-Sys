
from m5.objects.Device import DmaDevice
from m5.objects.Process import EmulatedDriver

from m5.params import Param
from m5.proxy import Parent


class PIMIODriver(EmulatedDriver):
    type = 'PIMIODriver'
    cxx_class = 'gem5::PIMIODriver'
    cxx_header = 'src/pimio_driver.hh'
    device = Param.PimAccel("PIMAccel Device controlled by this driver.")
    # No parameters for now


class PimAccel(DmaDevice):

    type = 'PimAccel'
    cxx_class = 'gem5::PimAccel'
    cxx_header = 'src/pim_accel.hh'
    pio_size = Param.Addr(0x1000, "PIO Size")
    pio_addr = Param.Addr("Device Address")
    latency = Param.Latency('0ns', "DMA Device Latency")
    int_id = Param.Int("Interrupt ID for the PIC to use")
