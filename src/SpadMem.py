from m5.params import *
from m5.objects.ClockedObject import ClockedObject
from m5.proxy import *

class SpadMem(ClockedObject):
    type = 'SpadMem'
    cxx_header = "src/spad_mem.hh"
    cxx_class = 'gem5::SpadMem'
    dataPort = ResponsePort("Memory side port, sends response")
    addrRanges = VectorParam.AddrRange(
      [AllMemory], "Address range this controller responds to")

