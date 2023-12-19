#ifndef PICOTFT_DISPLAYIO_HPP
#define PICOTFT_DISPLAYIO_HPP

#include <pico/types.h>

#include "PinConfig.hpp"

class DisplayIO
{
public:
  DisplayIO(PinConfig pinConfig);

  void writeCmd(uint8_t byte);
  void writeData(uint8_t byte);
  void writeDataBlock(unsigned short len, uint8_t *pBytes);

private:
  void writeByte(uint8_t byte);
  void pullHigh(uint pin);
  void pullLow(uint pin);
  void driveHigh(uint pin);
  void driveLow(uint pin);
}
#endif // PICOTFT_DISPLAYIO_HPP
