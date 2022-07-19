#ifndef __PIM_DNN_SPAD_MEM_HH_
#define __PIM_DNN_SPAD_MEM_HH_

#include "mem/port.hh"
#include "params/SpadMem.hh"
#include "sim/clocked_object.hh"
#include "debug/SpadMem.hh"

namespace gem5 {

class DataBank {
 public:
    DataBank(int word_num, int word_size) 
    : wordNum(word_num),
      wordSize(word_size) {
        memData.assign(word_num*word_size,12);
      }

  void writeData(Addr addr, uint8_t* data, int size) {
    memcpy(&memData[addr], data, size);   
  }

  void readData(Addr addr, uint8_t* data, int size) {
    memcpy(data, &memData[addr], size);
  }

 protected:
  int wordNum;
  int wordSize;
  std::vector<uint8_t> memData;
};    

class SpadMem : public ClockedObject {
   public:
    SpadMem(const SpadMemParams& params);
    ~SpadMem() {}
    Port& getPort(const std::string& if_name, PortID idx) override {
        if (if_name == "dataPort")
            return dataPort;
        else
            fatal("cannot resolve the port name " + if_name);
    }

    void init() override { dataPort.sendRangeChange(); }
    void accessFunctional(Addr addr, int size, uint8_t* data, bool isRead);
    void accessFunctional(PacketPtr pkt) {
        Addr addr = pkt->getAddr();
        uint8_t* data = pkt->getPtr<uint8_t>();
        accessFunctional(addr, pkt->getSize(), data, pkt->isRead());
    }

    void accessTiming(PacketPtr pkt);

   protected:
    class DataPort : public ResponsePort {
       public:
        DataPort(const std::string& name, SpadMem* owner)
            : ResponsePort(name, owner), spad(owner) {}
        AddrRangeList getAddrRanges() const override {
            return spad->addrRanges;
        }

        bool recvTimingReq(PacketPtr pkt);
        void recvRespRetry() override{};

        Tick recvAtomic(PacketPtr pkt) override { return 0; }
        void recvFunctional(PacketPtr pkt) override {}

       private:
        SpadMem* spad;
    };

    // main function to process a mem access request
   public:
    bool handleRequest(PacketPtr pkt);
    void sendResponse(PacketPtr pkt);

   public:

    int addressDecode(Addr addr);

    std::vector<bool> busy;
    DataPort dataPort;
    AddrRangeList addrRanges;

    std::vector<DataBank> dataBank;
    
   private:
    int wordNum;
    int wordSize;
    int bankNum;



};

}  // namespace gem5

#endif
