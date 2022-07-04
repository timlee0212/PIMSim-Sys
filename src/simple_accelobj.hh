
#ifndef __PIM_DNN_SIMPLE_ACCELOBJ_HH__
#define __PIM_DNN_SIMPLE_ACCELOBJ_HH__

#include "mem/port.hh"
#include "params/SimpleAccelObj.hh"
#include "sim/clocked_object.hh"
#include "src/pim_core.hh"
#include "src/spad_mem.hh"

namespace gem5 {

class SimpleAccelObj : public ClockedObject {
   public:
    void step();
    EventWrapper<SimpleAccelObj, &SimpleAccelObj::step> tickEvent;

    Port &getPort(const std::string &if_name, PortID idx) override {
        if (if_name == "interuptPort")
            return interuptPort;
        else
            fatal("cannot resolve the port name " + if_name);
    }

   protected:
    enum State { Idle, WaitForDataTransfer, WaitForComputing };
    State mainState;

   public:
    class InteruptPort : public RequestPort {
       private:
        SimpleAccelObj *accel;

       public:
        InteruptPort(const std::string &name, SimpleAccelObj *owner)
            : RequestPort(name, owner), accel(owner) {}

       protected:
        bool recvTimingResp(PacketPtr pkt);
        void recvReqRetry() override {}
        void recvRangeChange() override {}
    };

    InteruptPort interuptPort;

   public:
    typedef SimpleAccelObjParams Params;
    SimpleAccelObj(const Params &p);
    int instCount;

    void startup();
    void notifyDone();

   private:
    SpadMem *spadmem;
    PimCore *pimcore;
};
}  // namespace gem5
#endif
