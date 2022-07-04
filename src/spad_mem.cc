#include "src/spad_mem.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/SpadMem.hh"

namespace gem5 {

SpadMem::SpadMem(const SpadMemParams &params)
    : ClockedObject(params), busy(false), dataPort(name() + ".dataPort", this),
      addrRanges(params.addrRanges.begin(), params.addrRanges.end()) {
  DPRINTF(SpadMem, "Created the SpadMem object.\n");
}

bool SpadMem::DataPort::recvTimingReq(PacketPtr pkt) {
  return spad->handleRequest(pkt);
}

bool SpadMem::handleRequest(PacketPtr pkt) {
  if (busy) {
    DPRINTF(SpadMem, "Got request for addr %#x, but the memory is busy.\n",
            pkt->getAddr());
    return false;
  }
  DPRINTF(SpadMem, "Got request for addr %#x\n", pkt->getAddr());
  busy = true;
  DPRINTF(SpadMem, "start access.\n");
  schedule(new EventFunctionWrapper([this, pkt] { accessTiming(pkt); },
                                    name() + ".accessEvent", true),
           clockEdge(Cycles(1)));
  return true;
}

void SpadMem::sendResponse(PacketPtr pkt) {
  assert(busy);
  DPRINTF(SpadMem, "Sending resp for addr %#x\n", pkt->getAddr());
  busy = false;
  dataPort.sendTimingResp(pkt);
}

void SpadMem::accessTiming(PacketPtr pkt) {
  accessFunctional(pkt);
  DPRINTF(SpadMem, "packet: %s\n", pkt->print());
  pkt->makeResponse();
  sendResponse(pkt);
  busy = false;
  DPRINTF(SpadMem, "Finish access.\n");
}

void SpadMem::accessFunctional(Addr addr, int size, uint8_t *data,
                               bool isRead) {
  DPRINTF(SpadMem, "Process an access request.\n");
}

} // namespace gem5
