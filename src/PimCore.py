from m5.params import *
from m5.objects.ClockedObject import ClockedObject
from m5.proxy import *

class PimCore(ClockedObject):
    type = 'PimCore'
    cxx_header = "src/pim_core.hh"
    cxx_class = 'gem5::PimCore'
    dataPort = RequestPort("Memory side port, sends requests")
    interuptPort = ResponsePort("Controller side port, sends requests")    