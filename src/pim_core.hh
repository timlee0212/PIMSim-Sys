#ifndef __PIM_CORE_HH__
#define __PIM_CORE_HH__

#include "mem/port.hh"
#include "params/PimCore.hh"
#include "sim/clocked_object.hh"

#include "src/fetch_unit.hh"
#include "src/commit_unit.hh"

#include <vector>

namespace gem5 {

class PimAccel;

class PimCore : public ClockedObject {
   public:
    Port& getPort(const std::string& if_name, PortID idx) override {
        if (if_name == "fetchPort")
            return fetchUnit->fetchPort;
        else if (if_name == "commitPort")
            return commitUnit->commitPort;            
        else
            fatal("cannot resolve the port name " + if_name);
    }

   private:
    FetchUnit *fetchUnit;
    CommitUnit *commitUnit;
    
   public:
    PimCore(const PimCoreParams& p);
    bool isIdle();
    void start();
    void compute();    
    void finish();
    void notifyFetchDone();  
    void notifyCommitDone();        

   public:
    void init() override;

   private:
    PimAccel *accel;

    std::vector<uint8_t> inputReg;
    std::vector<uint8_t> outputReg;  
};
}  // namespace gem5
#endif