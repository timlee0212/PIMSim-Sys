#include "src/pim_core.hh"
#include "src/fetch_unit.hh"

namespace gem5 {

FetchUnit::FetchUnit(const std::string& name,
                      PimCore *_core)
    : fetchPort(name, _core,this),state(Idle){}

FetchUnit::FetchPort::FetchPort(const std::string& name, PimCore* _owner,FetchUnit* _fetchUnit)
            : RequestPort(name, _owner), fetchUnit(_fetchUnit),owner(_owner){}

void
FetchUnit::fetch(Addr addr){
      Request::Flags flags;
      unsigned int size = 64;
      req = std::make_shared<Request>(addr, size, flags, 0);
      PacketPtr pkt = Packet::createRead(req);
      pkt->dataStatic(outputReg);
      fetchPort.sendTimingReq(pkt);
}

void
FetchUnit::setOutputReg(uint8_t *p){
  outputReg = p;
}

bool 
FetchUnit::FetchPort::recvTimingResp(PacketPtr pkt) 
{
    fetchUnit->release();
    owner->notifyFetchDone();
    return true;
}

} 


