#include "src/pim_core.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/PimCore.hh"
#include "mem/port.hh"

namespace gem5{

PimCore::PimCore(const PimCoreParams &params):
  ClockedObject(params),
  dataReadEvent(this),
  computeEvent(this),  
  dataWriteEvent(this),
  interuptPort(name() + ".interuptPort", this),     
  dataPort(name() + ".dataPort", this),  
  dataReadState(Idle),
  computeState(Idle),    
  dataWriteState(Idle)
  //accel(params.accel)
  {
    DPRINTF(PimCore, "Created the PimCore object\n");
    Request::Flags flags;
    Addr addr = 0;
    unsigned int size = 64;
    req = std::make_shared<Request>(addr,size,flags,0);
}

bool
PimCore::isIdle(){
  return (dataReadState==Idle && computeState==Idle && dataWriteState==Idle);
}

void
PimCore::start(){
    DPRINTF(PimCore, "Tiggerred PIM Core.\n");  
    schedule(dataReadEvent,clockEdge(Cycles(1)));
}

void
PimCore::dataRead(){
    dataReadState=Busy;
    DPRINTF(PimCore, "Request for data fetch.\n"); 
    PacketPtr pkt = Packet::createRead(req);  
    pkt->allocate();
    dataPort.sendTimingReq(pkt);
}

bool
PimCore::DataPort::recvTimingResp(PacketPtr pkt){    
  if(pkt->isRead()){  
    pimcore->dataReadState=Idle;
    pimcore->compute();
  }
  else if(pkt->isWrite()){
    pimcore->dataWriteState=Idle;
    pimcore->finish();
   
  }
  return true;
}



void
PimCore::compute(){
    if(computeState==Idle){
        computeState = Busy;
        DPRINTF(PimCore, "Start to compute.\n");    
        schedule(computeEvent,clockEdge(Cycles(100)));
    }
    else if(computeState==Busy){
        computeState = Idle;
        DPRINTF(PimCore, "Finish to compute.\n");    
        schedule(dataWriteEvent,clockEdge(Cycles(1)));
    }
}

void
PimCore::dataWrite(){
    dataWriteState=Busy;
    DPRINTF(PimCore, "Write back data.\n");
    PacketPtr pkt = Packet::createWrite(req);  
    pkt->allocate();
    dataPort.sendTimingReq(pkt);
}

void
PimCore::finish(){
  DPRINTF(PimCore, "Finiish Execution.\n");  
  PacketPtr pkt = Packet::createWrite(req);  
  pkt->allocate();  
  pkt->makeResponse();
  interuptPort.sendTimingResp(pkt);
}

bool
PimCore::InteruptPort::recvTimingReq(PacketPtr pkt)
{
  pimcore->start();
  return true;
}


}