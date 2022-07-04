from m5.params import *
from m5.objects.ClockedObject import ClockedObject
from m5.proxy import *

class SimpleAccelObj(ClockedObject):
    type = 'SimpleAccelObj'
    cxx_header = "src/simple_accelobj.hh"
    cxx_class = 'gem5::SimpleAccelObj'
    spadMem = Param.SpadMem("Local input scratchpad.")
    pimCore = Param.PimCore("PIM Core.")
    interuptPort = RequestPort("Controller side port, sends requests")  

