#ifndef PICOTFT_DISPLAYIO_HPP
#define PICOTFT_DISPLAYIO_HPP
#include <cstdint>
#include "DisplayCmdWrite.hpp"
#include "pico/types.h"
#include "picotft/PinConfig.hpp"
#include "hardware/pio.h"

class DisplayIO
{
public:
  DisplayIO(const PinConfig &pinConfig);

  int writeCmd(std::uint8_t cmdByte);
  int writeCmdEx(const DisplayCmdWrite &request);
  void waitForWriteTicket(int ticket);
  void isWriteTicketCompleted(int ticket);
  void cancelWriteTicket(int ticket);
  // TODO: reading
private:
  struct WriteRequest
  {
    DisplayCmdWrite writeInfo;
    int header[2];
    int subId;
  };
  static DisplayIO *pSingleton; // ew
  static void dmaIrqHandler();

  PinConfig pinConfig;
  int writeTicketsDone;
  int writeTicketsIssued;
  int writeTicketSizeBits;
  int maxWriteTicket;
  bool maxWritesReached;
  bool nextDmaReady;
  std::vector<WriteRequest> writeRequests;
  PIO pio;
  uint writePioSm;
  uint cmdWriteDma;
  uint paramWriteDma;
  dma_channel_config cmdWriteDmaConfig;
  dma_channel_config paramWriteDmaConfig;

  void processNextWriteTicket();
  void setupDma(const WriteRequest &request);
};
#endif // PICOTFT_DISPLAYIO_HPP
