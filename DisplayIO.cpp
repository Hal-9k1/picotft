#include "picotft/DisplayIO.hpp"

#include <cassert>
#include <cstdint>
#include "pico/time.h"
#include "pico/types.h"
#include "pico/stdio.h"
#include "DisplayCmdWrite.hpp"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/structs/pio.h"
#include "hardware/sync.h"

#include "gen_cmd_write.pio.h"

DisplayIO *DisplayIO::pInstance;

DisplayIO::DisplayIO(const PinConfig &pinConfig, uint readBufferLength)
  : writeTicketsDone(0),
    writeTicketsIssued(0),
    writeTicketSizeBits(7),
    maxWritesReached(false),
    nextDmaReady(false)
{
  maxWriteTicket = ~(~0u << writeTicketSizeBits);
  // leave out most significant bit for signalling:
  int subIdSizeBits = sizeof(int) * CHAR_BIT - writeTicketSizeBits - 1;
  maxSubId = ~(~0u << subIdSizeBits);
  writeRequests.resize(maxWriteTicket + 1); // zero initialized

  int sm = pio_claim_unused_sm(pio0, false);
  if (sm == -1)
  {
    pio = pio1;
    writePioSm = pio_claim_unused_sm(pio1, true);
  }
  else
  {
    pio = pio0;
    writePioSm = sm;
  }
  uint genCmdWriteProgOffset = pio_add_program(pio, &gen_cmd_write_program);
  gen_cmd_write_program_init(pio, writePioSm, genCmdWriteProgOffset, pinConfig.chipSelCmdSwitchBase,
    pinConfig.busBase, pinConfig.writeStrobe);

  irq_set_exclusive_handler(DMA_IRQ_0, dmaIrqHandler);

  cmdWriteDmaConfig = dma_channel_get_default_config(cmdWriteDma);
  channel_config_set_dreq(&cmdWriteDmaConfig, pio_get_dreq(pio, writePioSm, true));
  channel_config_set_transfer_data_size(&cmdWriteDmaConfig, DMA_SIZE_32);
  cmdWriteDma = dma_claim_unused_channel(true);
  dma_channel_set_config(cmdWriteDma, &cmdWriteDmaConfig, false);
  dma_channel_set_write_addr(cmdWriteDma, &pio->txf[writePioSm], false);
  dma_channel_set_trans_count(cmdWriteDma, 2. false);
  dma_channel_set_irq0_enabled(cmdWriteDma, true);

  paramWriteDmaConfig = dma_channel_get_default_config(paramWriteDma);
  channel_config_set_dreq(&paramWriteDmaConfig, pio_get_dreq(pio, writePioSm, true));
  paramWriteDma = dma_claim_unused_channel(true);
  dma_channel_set_config(paramWriteDma, &paramWriteDmaConfig, false);
  dma_channel_set_write_addr(paramWriteDma, &pio->txf[writePioSm], false);
  dma_channel_set_irq0_enabled(paramWriteDma, true);

  gpio_init(pinConfig.reset);
  gpio_set_dir(pinConfig.reset, GPIO_OUT);
  gpio_put(pinConfig.reset, true);
  // black magic reset sequence:
  sleep_ms(100);
  gpio_put(pinConfig.reset, false);
  sleep_us(10);
  gpio_put(pinConfig.reset, true);
  sleep_ms(120);

  if (pSingleton)
  {
    panic("picotft/DisplayIO::DisplayIO: created duplicate singleton DisplayIO.");
  }
  pSingleton = this;
}

int DisplayIO::writeCmd(std::uint8_t cmdByte)
{
  DisplayCmdWrite write{};
  write.cmdByte = cmdByte;
  return writeCmdEx(write);
}
int DisplayIO::writeCmdEx(const DisplayCmdWrite &write)
{
  if (maxWritesReached)
  {
    panic("picotft/DisplayIO::writeCmdEx: exceeded maximum number of display write requests.");
  }
  int ticket = writeTicketsIssued;
  WriteRequest &request = writeRequests[ticket];
  request.write = write;
  int transferSize = write.swapBytePairs ? 2
    : write.isAlignPaddedToFourBytes ? 4
    : 1;
  int paddedSize = (write.dataLen + transferSize - 1) / transferSize;
  int paddingSize = paddedSize - write.dataLen; // shouldn't exceed 3
  request.header[0] = write.dataLen;
  request.header[1] = write.cmdByte << 24 | paddingSize << 22;

  writeTicketsIssued = (writeTicketsIssued + 1) & maxWriteTicket;
  if (writeTicketsIssued == writeTicketsDone)
  {
    maxWritesReached = true;
  }
  if (ticket == writeTicketsIssued)
  {
    // dma stalled, restart it
    processNextWriteTicket(false);
  }
  return ticket | request.subId << writeTicketSizeBits;
}
void DisplayIO::waitForWriteTicket(int ticket)
{
  while (!isWriteTicketCompleted(ticket))
  {
    __wfe();
  }
}
void DisplayIO::isWriteTicketCompleted(int ticket)
{
  int subId = static_cast<unsigned int>(ticket) >> static_cast<unsigned int>(writeTicketSizeBits);
  int index = ticket & ~(~0u << writeTicketSizeBits);
  int curSubId = writeRequests[index].subId;
  if (curSubId <= subId)
  {
    curSubId += maxSubId + 1;
  }
  return curSubId - subId < (maxSubId + 1) / 2;
}
void DisplayIO::cancelWriteTicket(int ticket)
{
  int subId = static_cast<unsigned int>(ticket) >> static_cast<unsigned int>(writeTicketSizeBits);
  int index = ticket & ~(~0u << writeTicketSizeBits);
  if (writeTickets[index].subId == subId)
  {
    writeTickets[index].write.cmdByte = 0;
  }
}
void DisplayIO::dmaIrqHandler()
{
  if (dma_channel_get_irq0_status(pSingleton->cmdWriteDma))
  {
    dma_channel_acknowledge_irq0(pSingleton->cmdWriteDma);
    // -1 to get previous ticket, +1 to turn maxWriteTicket into numWriteTickets, so they cancel:
    int oldTicket = (pSingleton->writeTicketsDone + pSingleton->maxWriteTicket)
      & pSingleton->maxWriteTicket;
    DisplayCmdWrite &oldRequest = pSingleton->writeRequests[oldTicket].write;
    if (!oldRequest.pData)
    {
      // irqs from cmdWriteDma only trigger the next ticket if there was no data
      pSingleton->processNextWriteTicket(true);
    }
  }
  else if (dma_channel_get_irq0_status(pSingleton->paramWriteDma))
  {
    // if there was data on the last ticket, we definitely need to go to the next one
    dma_channel_acknowledge_irq0(pSingleton->paramWriteData);
    pSingleton->processNextWriteTicket(true);
  }
}
void DisplayIO::processNextWriteTicket(bool chained)
{
  maxWritesReached = false;
  while (true)
  {
    if (chained)
    {
      // don't handle old request if stalled because we've done it already
      int oldTicket = (writeTicketsDone - 1 + maxWriteTicket) & maxWriteTicket;
      WriteRequest &oldRequest = writeRequests[oldTicket];
      oldRequest.subId = (request.subId + 1) & maxSubId;
      if (oldRequest.write.handler)
      {
        oldRequest.write.handler(oldRequest.cmdByte, oldRequest.pData, oldRequest.pRequestData);
      }
      __sev(); // wake up waitForWriteTicket
    }
    if (writeTicketsDone == writeTicketsIssued && !maxWritesReached)
    {
      // if out of requests, stop here and break the dma irq chain
      return;
    }
    maxWritesReached = false;
    if (writeRequests[writeTicketsDone].write.cmdByte)
    {
      // zeroed cmdByte means request was canceled
      if (!nextDmaReady)
      {
        // set up dma now because we didn't do it last call (since there was no next ticket)
        setupDma(writeRequests[writeTicketsDone]);
      }
      dma_channel_start(cmdWriteDma);
    }
    writeTicketsDone = (writeTicketsDone + 1) & maxWriteTicket;
    if (writeTicketsDone != writeTicketsIssued)
    {
      // only if there's a next ticket, start setting up the DMA
      setupDma(writeRequests[writeTicketsDone]);
      nextDmaReady = true;
    }
    else
    {
      nextDmaReady = false;
    }
    if (writeRequests[writeTicketsDone].write.cmdByte)
    {
      return; // if we did start an uncancelled request, stop here
    }
  }
}
void DisplayIO::setupDma(const WriteRequest &request)
{
  channel_config_set_chain_to(&cmdWriteDmaConfig,
    request.write.pData ? paramWriteDma : cmdWriteDma);
  dma_channel_set_config(cmdWriteDma, &cmdWriteDmaConfig, false);
  dma_channel_set_read_addr(cmdWriteDma, request.header, false);
  if (request.write.pData)
  {
    dma_channel_set_read_addr(paramWriteDma, request.write.pData, false);
    int len = request.write.dataLen;
    dma_channel_transfer_size transferSize = DMA_SIZE_8;
    if (request.write.swapBytePairs)
    {
      len /= 2;
      transferSize = DMA_SIZE_16;
    }
    else if (request.write.isAlignPaddedToFourBytes
      || (len % 4 == 0 && !(reinterpret_cast<std::uintptr_t>(request.write.pData) % 4)))
    {
      len = (len + 3) / 4; // round up to next transfer unit
      transferSize = DMA_SIZE_32;
    }
#ifndef PICOTFT_DISPLAYCMD_ALIGNMENT_AWARE
    static_assert(alignof(max_align_t) % 4 == 0, "Warning: user code using"
      " DmaCmdWriteRequest.swapBytePairs or isAlignPaddedToFourBytes must be alignment conscious"
      " when allocating memory for pData; alignof(max_align_t) % 4 != 0. Define"
      " PICOTFT_DISPLAYCMD_ALIGNMENT_AWARE to silence.");
#endif // PICOTFT_DISPLAYCMD_ALIGNMENT_AWARE
    channel_config_set_transfer_data_size(&paramWriteDmaConfig, transferSize);
    channel_config_set_bswap(&paramWriteDmaConfig, request.write.swapBytePairs);
    channel_config_set_ring(&paramWriteDmaConfig, false, request.write.dataWrapLenLog2);
    dma_channel_set_config(paramWriteDmaConfig, &paramWriteDmaConfig, false);
    dma_channel_set_trans_count(paramWriteDma, len, false);
  }
}
