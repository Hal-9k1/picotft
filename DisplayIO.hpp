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
  void setBusIOMode(bool mode);
}
#endif // PICOTFT_DISPLAYIO_HPP
