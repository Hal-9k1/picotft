#ifndef PICOTFT_DISPLAY_HPP
#define PICOTFT_DISPLAY_HPP
#include <cstdint>

#include "picotft/DisplayIO.hpp"

class Display
{
public:
  Display(DisplayIO *pDisplayIO);
  
  void setBacklightBrightness(std::uint8_t brightness);
  void setDisplayOn(bool on);
  void setSleepOn(bool on);
  void writePixelBlock(std::uint16_t startCol, std::uint16_t endCol, std::uint16_t startRow,
    std::uint16_t endRow, std::uint16_t *pPixelData);

private:
  DisplayIO *pDisplayIO;
}

#endif // PICOTFT_DISPLAY_HPP
