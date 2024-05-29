#include "picotft/Display.hpp"

#include <cstdint>
#include <cstddef>
#include "picotft/DisplayIO.hpp"

Display::Display(DisplayIO *pDisplayIO, DisplayOrientation initialOrientation)
  : pDisplayIO(pDisplayIO)
{
  // set the pixel format
  // 0x50: use B5G6R5 format for DPI interface (for direct writing to the display, which we probably
  //   won't use so I'm not sure if it's necessary to set this)
  // 0x05: use B5G6R5 format for DBI interface (for writing to display RAM)
  uint8_t pixelFormat = 0x55;
  pDisplayIO->writeCmd(0x3A, 1, false, &pixelFormat);
  setDisplayOn(true);
  setSleepOn(false);
  setInvertOn(false);
  setBacklightBrightness(255); // max brightness
  dmacByte = static_cast<std::uint8_t>(initialOrientation) << 5;
  writeDmacByte();
}

void Display::setBacklightBrightness(std::uint8_t brightness)
{
  pDisplayIO->writeCmd(0x51, 1, false, &brightness);
}
void Display::setDisplayOn(bool on)
{
  pDisplayIO->writeCmd(on ? 0x29 : 0x28, 0, false, nullptr);
}
void Display::setSleepOn(bool on)
{
  pDisplayIO->writeCmd(on ? 0x10 : 0x11, 0, false, nullptr);
}
void Display::setInvertOn(bool on)
{
  pDisplayIO->writeCmd(on ? 0x21 : 0x20, 0, false, nullptr);
}
void Display::setOrientation(DisplayOrientation orientation)
{
  dmacByte |= static_cast<std::uint8_t>(orientation) << 5;
  writeDmacByte();
}
void Display::writePixelBlock(std::uint16_t startCol, std::uint16_t endCol, std::uint16_t startRow,
  std::uint16_t endRow, bool bigEndian, const std::uint8_t *pPixelData)
{
  setWriteWindow(startCol, endCol, startRow, endRow);
  std::size_t numPixels = (endCol - startCol + 1) * (endRow - startRow + 1);
  pDisplayIO->writeCmd(0x2C, numPixels * 2, !bigEndian, pPixelData);
}
void Display::clear(std::uint16_t color)
{
  std::uint8_t pPixelData[] = {
    static_cast<std::uint8_t>(color >> 8),
    static_cast<std::uint8_t>(color & 0xFF),
  };
  writeRepeatingPixelBlock(0, 0x01DF, 0, 0x013F, true, 1, pPixelData);
}
void Display::writeRepeatingPixelBlock(std::uint16_t startCol, std::uint16_t endCol,
  std::uint16_t startRow, std::uint16_t endRow, bool bigEndian, std::size_t pixelCount,
  const std::uint8_t *pPixelData)
{
  setWriteWindow(startCol, endCol, startRow, endRow);
  // multiply by 2 because 2 bytes per pixel
  std::size_t numWrittenPixels = (endCol - startCol + 1) * (endRow - startRow + 1) * 2;
  pDisplayIO->writeCmdHeader(0x2C);
  for (std::size_t i = 0; i < numWrittenPixels; ++i)
  {
    // swap every 2 bytes (2 bytes per pixel)
    int endianOffset = (!bigEndian && i + 1 < numWrittenPixels) ? !(i % 2) * 2 - 1 : 0;
    pDisplayIO->writeByte(pPixelData[i % (pixelCount * 2) + endianOffset]);
  }
  pDisplayIO->endCmdWrite();
}
void Display::getSize(int &width, int &height)
{
  bool isLandscape = dmacByte & (1 << 5);
  width = isLandscape ? 480 : 320;
  height = isLandscape ? 320 : 480;
}

void Display::setWriteWindow(std::uint16_t startCol, std::uint16_t endCol,
  std::uint16_t startRow, std::uint16_t endRow)
{
  std::uint8_t colAddrParam[] = {
    static_cast<std::uint8_t>(startCol >> 8),
    static_cast<std::uint8_t>(startCol & 0xFF),
    static_cast<std::uint8_t>(endCol >> 8),
    static_cast<std::uint8_t>(endCol & 0xFF)
  };
  pDisplayIO->writeCmd(0x2A, 4, false, colAddrParam);
  std::uint8_t pageAddrParam[] = {
    static_cast<std::uint8_t>(startRow >> 8),
    static_cast<std::uint8_t>(startRow & 0xFF),
    static_cast<std::uint8_t>(endRow >> 8),
    static_cast<std::uint8_t>(endRow & 0xFF)
  };
  pDisplayIO->writeCmd(0x2B, 4, false, pageAddrParam);
}
void Display::writeDmacByte()
{
  pDisplayIO->writeCmd(0x36, 1, false, &dmacByte);
}
