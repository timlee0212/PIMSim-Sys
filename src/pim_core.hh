#ifndef __PIM_DNN_PIM_CORE_HH__
#define __PIM_DNN_PIM_CORE_HH__

#include "mem/port.hh"
#include "params/PimCore.hh"
#include "sim/clocked_object.hh"

namespace gem5 {

class SimpleAccelObj;

class PimCore : public ClockedObject {
   public:
    void dataRead();
    EventWrapper<PimCore, &PimCore::dataRead> dataReadEvent;
    void compute();
    EventWrapper<PimCore, &PimCore::compute> computeEvent;
    void dataWrite();
    EventWrapper<PimCore, &PimCore::dataWrite> dataWriteEvent;

    Port& getPort(const std::string& if_name, PortID idx) override {
        if (if_name == "dataPort")
            return dataPort;
        else if (if_name == "interuptPort")
            return interuptPort;
        else
            fatal("cannot resolve the port name " + if_name);
    }

   public:
    class InteruptPort : public ResponsePort {
       public:
        InteruptPort(const std::string& name, PimCore* owner)
            : ResponsePort(name, owner), pimcore(owner) {}
        AddrRangeList getAddrRanges() const override {
            return AddrRangeList(1, AddrRange());
        }

        bool recvTimingReq(PacketPtr pkt);
        void recvRespRetry() override{};

        Tick recvAtomic(PacketPtr pkt) override { return 0; }
        void recvFunctional(PacketPtr pkt) override {}

       private:
        PimCore* pimcore;
    };

    InteruptPort interuptPort;

   public:
    class DataPort : public RequestPort {
       private:
        PimCore* pimcore;
        PacketPtr blockedPacket;

       public:
        DataPort(const std::string& name, PimCore* owner)
            : RequestPort(name, owner),
              pimcore(owner),
              blockedPacket(nullptr) {}

       protected:
        bool recvTimingResp(PacketPtr pkt);
        void recvReqRetry() override {}
        void recvRangeChange() override {}
    };

    DataPort dataPort;

   protected:
    enum State { Idle, Busy };
    State dataReadState;
    State computeState;
    State dataWriteState;

   public:
    PimCore(const PimCoreParams& p);
    bool isIdle();
    void start();
    void finish();

   public:
    RequestPtr req;
};
}  // namespace gem5
#endif
