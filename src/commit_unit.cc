#include "src/pim_core.hh"
#include "src/commit_unit.hh"

namespace gem5 {

CommitUnit::CommitUnit(const std::string& name,
                      PimCore *_core)
    : commitPort(name, _core,this),state(Idle){}

CommitUnit::CommitPort::CommitPort(const std::string& name, PimCore* _owner,CommitUnit* _commitUnit)
            : RequestPort(name, _owner), commitUnit(_commitUnit),owner(_owner){}

void
CommitUnit::commit(Addr addr){
      Request::Flags flags;
      unsigned int size = 64;
      req = std::make_shared<Request>(addr, size, flags, 0);
      PacketPtr pkt = Packet::createWrite(req);
      pkt->dataStatic(inputReg);
      commitPort.sendTimingReq(pkt);
}

void
CommitUnit::setInputReg(uint8_t *p){
  inputReg = p;
}

bool 
CommitUnit::CommitPort::recvTimingResp(PacketPtr pkt) 
{
    commitUnit->release();
    owner->notifyCommitDone();
    return true;
}

} 


