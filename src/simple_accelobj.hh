
#ifndef __PIM_DNN_SIMPLE_ACCELOBJ_HH__
#define __PIM_DNN_SIMPLE_ACCELOBJ_HH__

#include "src/pim_core.hh"
#include "src/spad_mem.hh"

#include <vector>

#include "mem/port.hh"
#include "params/SimpleAccelObj.hh"
#include "sim/clocked_object.hh"

namespace gem5 {

class SimpleAccelObj : public ClockedObject {
   public:
    void step();
    EventWrapper<SimpleAccelObj, &SimpleAccelObj::step> tickEvent;

   protected:
    enum State { Idle, WaitForDataTransfer, WaitForComputing };
    State mainState;

   public:
    typedef SimpleAccelObjParams Params;
    SimpleAccelObj(const Params &p);
    int instCount;

    void startup();
    void notifyDone();

   public:
     //void registerSpadMem(SpadMem *spadmem);
     void registerPimCore(PimCore *pimcore);

   private:
    //std::vector<SpadMem*> spadmem;
    std::vector<PimCore*> pimcore;
};
}  // namespace gem5
#endif
