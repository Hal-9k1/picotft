#include "DisplayIO.hpp"

#include <cstddef>
#include <pico/types.h>

DisplayIO::DisplayIO(PinConfig pinConfig)
{
  for (uint i = 0; i < 8; ++i)
  {
    gpio_init(pinConfig.bus[i]);
  }
  gpio_init(pinConfig.chipSel);
  gpio_init(pinConfig.writeStrobe);
  gpio_init(pinConfig.readStrobe);
}

void DisplayIO::writeCmd(uint8_t byte)
{

}
void DisplayIO::writeData(uint8_t byte)
{

}
void DisplayIO::writeDataBlock(uint len, uint8_t *pBytes)
{

}

void DisplayIO::writeByte(uint8_t byte)
{
  
}
void DisplayIO::setBusIOMode(bool mode)
{
  for (uint i = 0; i < 8; ++i)
  {
    gpio_set_dir(pinConfig.bus[i], true);
  }
}
