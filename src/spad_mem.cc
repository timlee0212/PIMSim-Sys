#include "src/spad_mem.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/SpadMem.hh"

namespace gem5 {

SpadMem::SpadMem(const SpadMemParams &params)
    : ClockedObject(params),
      dataPort(name() + ".dataPort", this),
      addrRanges(params.addrRanges.begin(), params.addrRanges.end()),
      wordNum(params.wordNum),wordSize(params.wordSize),bankNum(params.bankNum)
    {
    for (int i=0;i<params.bankNum;i++){
        dataBank.push_back(DataBank(params.wordNum,params.wordSize));
        busy.push_back(false);
    }

    DPRINTF(SpadMem, "Created the SpadMem object.\n");
    DPRINTF(SpadMem, "Num of banks: %d.\n",bankNum);
    DPRINTF(SpadMem, "Num of words per bank: %d.\n",wordNum);   
    DPRINTF(SpadMem, "Size of words per bank: %d.\n",wordSize);            
}

bool 
SpadMem::DataPort::recvTimingReq(PacketPtr pkt) {
    return spad->handleRequest(pkt);
}

bool 
SpadMem::handleRequest(PacketPtr pkt) {
    //Addr addr = pkt->getAddr();
    if (busy[0]) {
        DPRINTF(SpadMem, "Got request for addr %#x, but the memory is busy.\n",
                pkt->getAddr());
        return false;
    }
    DPRINTF(SpadMem, "Got request for addr %#x\n", pkt->getAddr());
    busy[0] = true;
    DPRINTF(SpadMem, "start access.\n");
    schedule(new EventFunctionWrapper([this, pkt] { accessTiming(pkt); },
                                      name() + ".accessEvent", true),
             clockEdge(Cycles(1)));
    return true;
}

void SpadMem::sendResponse(PacketPtr pkt) {
    assert(busy[0]);
    DPRINTF(SpadMem, "Sending resp for addr %#x\n", pkt->getAddr());
    busy[0] = false;
    dataPort.sendTimingResp(pkt);
}

void 
SpadMem::accessTiming(PacketPtr pkt) {
    accessFunctional(pkt);
    DPRINTF(SpadMem, "packet: %s\n", pkt->print());
    pkt->makeResponse();
    sendResponse(pkt);
    busy[0] = false;
    DPRINTF(SpadMem, "Finish access.\n");
}

void 
SpadMem::accessFunctional(Addr addr, int size, uint8_t *data,
                               bool isRead) {                                 
    if(isRead){
        dataBank[0].readData(addr,data,size);
    }
    else{        
        dataBank[0].writeData(addr,data,size);
    }
}

int
SpadMem::addressDecode(Addr addr){
    
    return 0;
}

}  // namespace gem5
