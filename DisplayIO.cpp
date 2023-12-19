#include "DisplayIO.hpp"

#include <cstddef>
#include <pico/types.h>
#include <pico/stdio.h>

DisplayIO::DisplayIO(PinConfig pinConfig)
{
  for (uint i = 0; i < 8; ++i)
  {
    gpio_init(pinConfig.bus[i]);
  }
  gpio_init(pinConfig.chipSel);
  gpio_init(pinConfig.cmdSwitch);
  pullHigh(pinConfig.cmdSwitch);
  gpio_init(pinConfig.writeStrobe);
  pullHigh(pinConfig.writeStrobe);
  gpio_init(pinConfig.readStrobe);
  pullHigh(pinConfig.readStrobe);
}

void DisplayIO::writeCmd(uint8_t byte)
{
  driveLow(pinConfig.cmdSwitch);
  writeByte(byte);
  pullHigh(pinConfig.cmdSwitch);
}
void DisplayIO::writeData(uint8_t byte)
{
  
}
void DisplayIO::writeDataBlock(uint len, uint8_t *pBytes)
{

}

void DisplayIO::writeByte(uint8_t byte)
{
  driveLow(pinConfig.writeStrobe);
  
  pullHigh(pinConfig.writeStrobe);
}
void DisplayIO::pullHigh(uint pin)
{
  gpio_set_dir(pin, GPIO_IN);
  gpio_pull_up(pin);
}
void DisplayIO::pullLow(uint pin)
{
  gpio_set_dir(pin, GPIO_IN);
  gpio_pull_down(pin);
}
void DisplayIO::driveHigh(uint pin)
{
  gpio_set_dir(pin, GPIO_OUT);
  gpio_put(pin, true);
}
void DisplayIO::driveLow(uint pin)
{
  gpio_set_dir(pin, GPIO_OUT);
  gpio_put(pin, false);
}
