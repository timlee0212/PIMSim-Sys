#ifndef __COMMIT_UNIT_HH__
#define __COMMIT_UNIT_HH__

#include <queue>

#include "params/PimCore.hh"
#include "mem/port.hh"

namespace gem5 {

class PimCore;

class CommitUnit {
 public:
  CommitUnit(const std::string& name, PimCore *core);
 protected:
  class CommitPort : public RequestPort {
    public:
      CommitPort(const std::string& name, PimCore* owner, CommitUnit* _commitUnit);
    private:
      CommitUnit *commitUnit;    
      PimCore *owner;      

    protected:
      bool recvTimingResp(PacketPtr pkt);
      void recvReqRetry() override {}
      void recvRangeChange() override {}
    };

 public:
  CommitPort commitPort;
  enum State {Idle, Busy};
  State state;

  void commit(Addr addr);
  void setInputReg(uint8_t *p);
  void release(){state = Idle;}
  bool isIdle(){return (state==Idle);}

 public:
  RequestPtr req;
  uint8_t *inputReg;
  
};

}  // namespace systolic

#endif
