#include "src/pim_accel.hh"
#include "src/pim_core.hh"

#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/PimCore.hh"
#include "mem/port.hh"

namespace gem5 {

PimCore::PimCore(const PimCoreParams &params)
    : ClockedObject(params),
      accel(params.accel)
{
    DPRINTF(PimCore, "Created the PimCore object\n");

    inputReg.assign(1024,0);
    fetchUnit = new FetchUnit(name(),this);
    fetchUnit->setOutputReg(inputReg.data());

    outputReg.assign(1024,0);    
    commitUnit = new CommitUnit(name(),this);
    commitUnit->setInputReg(outputReg.data());    
}

void 
PimCore::init()
{
    accel->registerPimCore(this);
}

bool
PimCore::isIdle() 
{
    return (fetchUnit->isIdle() && commitUnit->isIdle());
}

void
PimCore::start() 
{
    Addr addr = 0;
    DPRINTF(PimCore, "Finish data fetch.\n");       
    fetchUnit->fetch(addr);
}

void
PimCore::notifyFetchDone(){
    DPRINTF(PimCore, "Finish data fetch.\n");   
    compute();
}

void
PimCore::compute(){
    DPRINTF(PimCore, "Finish computation commit.\n");   
    for(int i=0;i<128;i++){
        outputReg[i] = inputReg[i] + 4;
    }
    Addr addr = 0;    
    commitUnit->commit(addr);
}

void
PimCore::notifyCommitDone(){
    DPRINTF(PimCore, "Finish data commit.\n");   
    finish(); 
}

void 
PimCore::finish() {
    DPRINTF(PimCore, "Finish Execution.\n");
    accel->notifyDone();
}


}  // namespace gem5