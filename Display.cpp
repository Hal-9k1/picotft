#include "Display.hpp"

#include <cstdint>
#include <cstddef>
#include "DisplayIO.hpp"

Display::Display(DisplayIO *pDisplayIO)
  : pDisplayIO(pDisplayIO)
{
  // set the pixel format
  // 0x50: use R5G6B5 format for DPI interface (for direct writing to the display, which we probably
  //   won't use so I'm not sure if it's necessary to set this)
  // 0x05: use R5G6B5 format for DBI interface (for writing to display RAM)
  uint8_t pixelFormat = 0x55;
  pDisplayIO->writeCmd(0x3A, 1, &pixelFormat);
  //setDisplayOn(true);
  //setBacklightBrightness(255); // max brightness
}

void Display::setBacklightBrightness(std::uint8_t brightness)
{
  pDisplayIO->writeCmd(0x51, 1, &brightness);
}
void Display::setDisplayOn(bool on)
{
  pDisplayIO->writeCmd(on ? 0x29 : 0x28, 0, nullptr);
}
void Display::setSleepOn(bool on)
{
  pDisplayIO->writeCmd(on ? 0x10 : 0x11, 0, nullptr);
}
void Display::writePixelBlock(std::uint16_t startCol, std::uint16_t endCol, std::uint16_t startRow,
  std::uint16_t endRow, std::uint16_t *pPixelData)
{
  std::uint8_t colAddrParam[] = {
    (startCol & 0xFF00) >> 8,
    startCol & 0x00FF,
    (endCol & 0xFF00) >> 8,
    endCol & 0x00FF
  };
  pDisplayIO->writeCmd(0x2A, 4, colAddrParam);
  std::uint8_t pageAddrParam[] = {
    (startRow & 0xFF00) >> 8,
    startRow & 0x00FF,
    (endCol & 0xFF00) >> 8,
    endCol & 0x00FF
  };
  pDisplayIO->writeCmd(0x2B, 4, pageAddrParam);
  std::size_t numPixels = (endCol - startCol) * (endRow - startRow);
  // remember 2 bytes per pixel:
  std::uint8_t *pPixelBytes = new int[numPixels * 2];
  // this loop SUCKS. if this becomes a bottleneck consider some weird aliasing trick (can you do
  // that with uint8_t instead of char?) or maybe just memcpy the whole array if host and LCD
  // endianness match
  for (std::size_t i = 0; i < numPixels; ++i)
  {
    pPixelBytes[i] = (pPixelData[i] & 0xFF00) >> 8;
    pPixelBytes[i + 1] = pPixelData[i] & 0x00FF;
  }
  pDisplayIO->writeCmd(0x2C, numPixels * 2, pPixelBytes);
}
