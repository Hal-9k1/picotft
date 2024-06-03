#include "UpdatableDisplayValue.hpp"

#include <cstdint>
#include "picotft/DisplayIO.hpp"

UpdatableDisplayValue::UpdatableDisplayValue(DisplayIO *pDisplayIO, std::uint8_t cmdByte)
  : pDisplayIO(pDisplayIO),
    cmdByte(cmdByte),
    modifyCounter(0),
    modifyRequestCounter(0)
{ }
int UpdatableDisplayValue::set(std::uint8_t value)
{
  if (modifyCounter != modifyRequestCounter)
  {
    pDisplayIO->cancelWriteRequest(writeTicket);
    modifyCounter = modifyRequestCounter;
  }
  this->brightness = brightness;
  DisplayCmdWrite cmdWrite{};
  cmdWrite.cmdByte = cmdByte;
  cmdWrite.dataLen = 1;
  cmdWrite.pData = this->brightness;
  cmdWrite.pRequestData = &modifyCounter;
  cmdWrite.handler = handleWrite;
  ++modifyRequestCounter;
  writeTicket = pDisplayIO->writeCmdEx(cmdWrite);
  return writeTicket;
}
std::uint8_t UpdatableDisplayValue::get()
{
  return value;
}
void UpdatableDisplayValue::handleWrite(bool completed, const std::uint8_t *pData,
  void *pRequestData)
{
  (void)pData;
  if (completed)
  {
    ++*static_cast<int *>(pRequestData);
  }
}
