#ifndef __FETCH_UNIT_HH__
#define __FETCH_UNIT_HH__

#include <queue>

#include "params/PimCore.hh"
#include "mem/port.hh"

namespace gem5 {

class PimCore;

class FetchUnit {
 public:
  FetchUnit(const std::string& name, PimCore *core);
 protected:
  class FetchPort : public RequestPort {
    public:
      FetchPort(const std::string& name, PimCore* owner,FetchUnit* _fetchUnit);
    private:
      FetchUnit *fetchUnit;    
      PimCore *owner;      

    protected:
      bool recvTimingResp(PacketPtr pkt);
      void recvReqRetry() override {}
      void recvRangeChange() override {}
    };

 public:
  FetchPort fetchPort;
  enum State {Idle, Busy};
  State state;

  void fetch(Addr addr);
  void setOutputReg(uint8_t *p);
  void release(){state = Idle;}
  bool isIdle(){return (state==Idle);}

 public:
  RequestPtr req;
  uint8_t *outputReg;
  
};

}  // namespace systolic

#endif
