#ifndef PICOTFT_DISPLAY_HPP
#define PICOTFT_DISPLAY_HPP
#include <cstdint>

#include "picotft/DisplayIO.hpp"

class Display
{
public:
  enum DisplayOrientation
  {
    PORTRAIT = 0,
    LANDSCAPE = 1
  };
  Display(DisplayIO *pDisplayIO, DisplayOrientation initialOrientation);
  
  void setBacklightBrightness(std::uint8_t brightness);
  void setDisplayOn(bool on);
  void setSleepOn(bool on);
  void setInvertOn(bool on);
  void setOrientation(DisplayOrientation orientation);
  void writePixelBlock(std::uint16_t startCol, std::uint16_t endCol, std::uint16_t startRow,
    std::uint16_t endRow, bool bigEndian, const std::uint8_t *pPixelData);
  void clear(std::uint16_t color);
  void writeRepeatingPixelBlock(std::uint16_t startCol, std::uint16_t endCol,
    std::uint16_t startRow, std::uint16_t endRow, bool bigEndian, std::size_t pixelCount,
    const std::uint8_t *pPixelData);
  void getSize(int &width, int &height);

private:
  DisplayIO *pDisplayIO;
  std::uint8_t dmacByte; // Device Memory Access Control

  void setWriteWindow(std::uint16_t startCol, std::uint16_t endCol, std::uint16_t startRow,
    std::uint16_t endRow);
  void writeDmacByte();
};

#endif // PICOTFT_DISPLAY_HPP
