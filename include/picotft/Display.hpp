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
    std::uint16_t endRow, bool bigEndian, int pixelRepeatLength, const std::uint8_t *pPixelData);
  void clear(bool bigEndian, const std::uint8_t *pPixelData);
  void getSize(int &width, int &height);

private:
  static void handleWriteWindowWrite(bool completed, const std::uint8_t *pData, void *pRequestData);

  DisplayIO *pDisplayIO;
  int colorDepth;
  int numWriteWindowSlots;
  UpdatableDisplayByte brightnessByte;
  UpdatableDisplayByte dmacByte; // Device Memory Access Control
  std::uint8_t *writeWindowBuf;
  int firstFreeWriteWindowSlot;

  bool isLandscape();
  void setWriteWindow(std::uint16_t startCol, std::uint16_t endCol, std::uint16_t startRow,
    std::uint16_t endRow);
};

#endif // PICOTFT_DISPLAY_HPP
