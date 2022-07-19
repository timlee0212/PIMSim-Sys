from m5.params import *
from m5.objects.ClockedObject import ClockedObject
from m5.proxy import *

class PimCore(ClockedObject):
    type = 'PimCore'
    cxx_header = "src/pim_core.hh"
    cxx_class = 'gem5::PimCore' 
    accel = Param.PimAccel(Parent.any," Owner of this core")
    fetchPort = RequestPort("Mem port connected to SpadMem to send fetch request")
    commitPort = RequestPort("Mem port connected to SpadMem to send commit request")