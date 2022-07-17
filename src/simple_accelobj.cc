#include "src/simple_accelobj.hh"

#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/SimpleAccelObj.hh"

namespace gem5 {

SimpleAccelObj::SimpleAccelObj(const SimpleAccelObjParams &params)
    : ClockedObject(params),
      tickEvent(this),
      mainState(Idle)
      //spadmem(params.spadMem),
      //pimcore(params.pimCore) 
    {
    DPRINTF(SimpleAccelObj, "Created the Accelerator object\n");
    instCount = 0;
}

//void SimpleAccelObj::registerSpadMem(SpadMem *spadmem){
//    this->spadmem.push_back(spadmem);/
//}

void SimpleAccelObj::registerPimCore(PimCore *pimcore){
    this->pimcore.push_back(pimcore);
}

void SimpleAccelObj::startup() {
    Tick latency = 67000;  // emulate the trigger behaviour

    schedule(tickEvent, curTick() + latency);
}

void SimpleAccelObj::step() {
    if (mainState == Idle) {
        DPRINTF(SimpleAccelObj, "Start to trigger the accelerator.\n");
        // clear DMA state;
        mainState = WaitForComputing;
        schedule(tickEvent, clockEdge(Cycles(1)));
    } else if (mainState == WaitForComputing) {
        if (instCount == 10) {
            instCount = 0;
            mainState = Idle;
            return;
        }
        if (pimcore[0]->isIdle()) {
            pimcore[0]->start();
            DPRINTF(SimpleAccelObj, "Triggerred instruction %d.\n", instCount);
        }
    }
}

void SimpleAccelObj::notifyDone() {
    if (mainState == WaitForComputing) {
        DPRINTF(SimpleAccelObj, "Receive Interupt for instruction %d.\n",
                instCount);
        instCount += 1;
        schedule(tickEvent, curTick() + 1);
    }
}

}  // namespace gem5