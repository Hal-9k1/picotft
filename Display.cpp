#include "picotft/Display.hpp"

#include <cstdint>
#include <cstddef>
#include "picotft/DisplayIO.hpp"
#include "DisplayCmdWrite.hpp"

Display::Display(DisplayIO *pDisplayIO, DisplayOrientation initialOrientation)
  : pDisplayIO(pDisplayIO),
    colorDepth(2),
    numWriteWindowSlots(4),
    brightnessByte(pDisplayIO, 0x51),
    dmacByte(pDisplayIO, 0x36)
{
  // set the pixel format
  // 0x50: use B5G6R5 format for DPI interface (for direct writing to the display, which we probably
  //   won't use so I'm not sure if it's necessary to set this)
  // 0x05: use B5G6R5 format for DBI interface (for writing to display RAM)
  std::uint8_t pixelFormat = 0x55;
  DisplayCmdWrite formatCmdWrite{};
  formatCmdWriteReq.cmdByte = 0x3A;
  formatCmdWriteReq.dataLen = 1;
  formatCmdWriteReq.pData = &pPixelFormat;
  pDisplayIO->writeCmdEx(formatCmdWriteReq);
  setDisplayOn(true);
  setSleepOn(false);
  setInvertOn(false);
  setBacklightBrightness(255); // max brightness
  writeWindowBuf = new std::uint8_t[numWriteWindowSlots * 8];
  std::uint8_t dmac = static_cast<std::uint8_t>(initialOrientation) << 5;
  pDisplayIO->waitForWriteTicket(dmacByte.set(dmac));
}

void Display::setBacklightBrightness(std::uint8_t brightness)
{
  brightnessByte.set(brightness);
}
void Display::setDisplayOn(bool on)
{
  pDisplayIO->writeCmd(on ? 0x29 : 0x28);
}
void Display::setSleepOn(bool on)
{
  pDisplayIO->writeCmd(on ? 0x10 : 0x11);
}
void Display::setInvertOn(bool on)
{
  pDisplayIO->writeCmd(on ? 0x21 : 0x20);
}
void Display::setOrientation(DisplayOrientation orientation)
{
  std::uint8_t dmac = dmacByte.get();
  if (orientation)
  {
    dmac |= 1 << 5;
  }
  else
  {
    dmac &= ~(1 << 5);
  }
  dmacByte.set(dmac);
}
int Display::writePixelBlock(std::uint16_t startCol, std::uint16_t endCol, std::uint16_t startRow,
  std::uint16_t endRow, bool isAlignPaddedToFourBytes, bool bigEndian, int pixelRepeatLength,
  const std::uint8_t *pPixelData)
{
  setWriteWindow(startCol, endCol, startRow, endRow);
  std::size_t numPixels = (endCol - startCol + 1) * (endRow - startRow + 1);
  DisplayCmdWrite cmdWrite{};
  cmdWrite.cmdByte = 0x2C;
  cmdWrite.dataLen = numPixels * colorDepth;
  if (pixelRepeatLength)
  {
    int byteRepeatLength = pixelRepeatLength * colorDepth;
    while (byteRepeatLength)
    {
      ++cmdWrite.dataWrapLenLogTwo;
      bool set = byteRepeatLength & 1;
      byteRepeatLength >>= 1;
      if (set && byteRepeatLength)
      {
        panic("picotft/Display::writePixelBlock: colorDepth * pixelRepeatLength is not a power of"
          " 2.");
      }
    }
    std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(reinterpret_cast<const void *>(
      pPixelData));
    if (ptr % (pixelRepeatLength * colorDepth))
    {
      panic("picotft/Display::writePixelBlock: pPixelData is not aligned to colorDepth *"
        " pixelRepeatLength bytes.");
    }
  }
  cmdWrite.swapBytePairs = !bigEndian;
  cmdWrite.pData = pPixelData;
  return pDisplayIO->writeCmdEx(cmdWrite);
}
void Display::waitForPixelBlockWrite(int ticket)
{
  pDisplayIO->waitForWriteTicket(ticket);
}
int Display::clear(bool bigEndian, const std::uint8_t *pPixelData)
{
  return writePixelBlock(0, isLandscape() ? 0x01DF : 0x013F, 0, isLandscape() ? 0x013F : 0x1DF,
    bigEndian, 1, pPixelData);
}
void Display::getSize(int &width, int &height)
{
  width = isLandscape() ? 480 : 320;
  height = isLandscape() ? 320 : 480;
}

bool Display::isLandscape()
{
  return dmacByte.get() & (1 << 5);
}
void Display::setWriteWindow(std::uint16_t startCol, std::uint16_t endCol,
  std::uint16_t startRow, std::uint16_t endRow)
{
  if (firstFreeWriteWindowSlot == numWriteWindowSlots)
  {
    panic("picotft/Display::setWriteWindow: no free write window slots.");
  }
  int slot = firstFreeWriteWindowSlot;
  for (++firstFreeWriteWindowSlot; firstFreeWriteWindowSlot < numWriteWindowSlots;
    ++firstFreeWriteWindowSlot)
  {
    bool free = true;
    for (int i = 0; i < 8; ++i)
    {
      if (writeWindowBuf[firstFreeWriteWindowSlot * 8 + i])
      {
        free = false;
        break;
      }
    }
    if (free)
    {
      break;
    }
  }
  std::uint8_t *pColAddrParam = writeWindowBuf[slot * 8];
  pColAddrParam[0] = static_cast<std::uint8_t>(startCol >> 8);
  pColAddrParam[1] = static_cast<std::uint8_t>(startCol & 0xFF);
  pColAddrParam[2] = static_cast<std::uint8_t>(endCol >> 8);
  pColAddrParam[3] = static_cast<std::uint8_t>(endCol & 0xFF);
  std::uint8_t *pPageAddrParam = writeWindowBuf[slot * 8 + 4];
  pPageAddrParam[0] = static_cast<std::uint8_t>(startRow >> 8);
  pPageAddrParam[1] = static_cast<std::uint8_t>(startRow & 0xFF);
  pPageAddrParam[2] = static_cast<std::uint8_t>(endRow >> 8);
  pPageAddrParam[3] = static_cast<std::uint8_t>(endRow & 0xFF);

  DisplayCmdWrite colCmdWrite{};
  colCmdWrite.cmdByte = 0x2A;
  colCmdWrite.dataLen = 4;
  colCmdWrite.isAlignPaddedToFourBytes = true;
  colCmdWrite.pData = isLandscape() ? pColAddrParam : pPageAddrParam;
  DisplayCmdWrite pageCmdWrite{};
  pageCmdWrite.cmdByte = 0x2B;
  pageCmdWrite.dataLen = 4;
  pageCmdWrite.isAlignPaddedToFourBytes = true;
  pageCmdWrite.pData = isLandscape() ? pPageAddrParam : pColAddrParam;
  pageCmdWrite.pRequestData = reinterpret_cast<void *>(slot);
  pageCmdWrite.handler = handleWriteWindowWrite;
  pDisplayIO->writeCmdEx(colCmdWrite);
  pDisplayIO->writeCmdEx(pageCmdWrite);
}
void Display::handleWriteWindowWrite(bool completed, const std::uint8_t *pData, void *pRequestData)
{
  (void)completed;
  std::memset(pData, 0, 8);
  std::intptr_t slotIdx = reinterpret_cast<std::intptr_t>(pRequestData);
  if (firstFreeWriteWindowSlot > slotIdx)
  {
    firstFreeWriteWindowSlot = slotIdx;
  }
}
