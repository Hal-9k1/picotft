#include "DisplayIO.hpp"

#include <cstdint>
#include <stdexcept>
#include "pico/types.h"
#include "pico/stdio.h"

DisplayIO::DisplayIO(PinConfig pinConfig, uint readBufferLength)
  : buf(new std::uint8_t[readBufferLength]),
    bufLen(readBufferLength),
    bufWriteCursor(0),
{
  for (uint i = 0; i < 8; ++i)
  {
    gpio_init(pinConfig.bus[i]);
    driveLow(pinConfig.bus[i]);
  }
  gpio_init(pinConfig.chipSel);
  pullHigh(pinConfig.chipSel);

  gpio_init(pinConfig.cmdSwitch);
  pullHigh(pinConfig.cmdSwitch);

  gpio_init(pinConfig.writeStrobe);
  pullHigh(pinConfig.writeStrobe);

  gpio_init(pinConfig.readStrobe);
  pullHigh(pinConfig.readStrobe);
  // note: this callback will be used for ALL IRQs on this core (hence the callback's first
  // parameter "gpio", the gpio that caused the interrupt.)
  gpio_set_irq_enabled_with_callback(pinConfig.readStrobe, GPIO_IRQ_EDGE_RISE, true,
    &handleReadInterrupt);
}

void DisplayIO::writeCmd(std::uint8_t cmdByte, uint dataLen, std::uint8_t *pDataBytes)
{
  driveLow(pinConfig.chipSel);
  driveLow(pinConfig.cmdSwitch);
  writeByte(cmdByte);
  pullHigh(pinConfig.cmdSwitch);
  for (uint i = 0; i < dataLen; ++i)
  {
    writeByte(pDataBytes[i]);
  }
  for (uint i = 0; i < 8; ++i)
  {
    driveLow(pinConfig.bus[i]);
  }
  pullHigh(pinConfig.chipSel);
}
std::uint8_t DisplayIO::waitReadByte()
{
  while (bufReadCursor == bufWriteCursor && !bufFull)
  {
    __wfe();
  }
  bufFull = false;
  std::uint8_t outByte = buf[bufReadCursor];
  bufReadCursor = (bufReadCursor + 1) % bufLen;
  return outByte;
}
bool DisplayIO::tryReadByte(std::uint8_t &outByte)
{
  if (bufReadCursor == bufWriteCursor && !bufFull)
  {
    return false;
  }
  bufFull = false;
  outByte = buf[bufReadCursor];
  bufReadCursor = (bufReadCursor + 1) % bufLen;
  return true;
}

void DisplayIO::handleReadInterrupt(uint gpio, std::uint32_t events)
{
  (void)gpio;
  (void)events;
  if (bufFull)
  {
    bufOverflow = true;
  }
  std::uint8_t byte;
  for (uint i = 0; i < 8; ++i)
  {
    if (gpio_get(pinConfig.bus[i]))
    {
      byte |= 1 << i;
    }
  }
  buf[bufWriteCursor] = byte;
  bufWriteCursor = (bufWriteCursor + 1) % bufLen;
  bufFull = bufReadCursor == bufWriteCursor;
  __sev();
}
void DisplayIO::writeByte(std::uint8_t byte)
{
  driveLow(pinConfig.writeStrobe);
  for (uint i = 0; i < 8; ++i)
  {
    if (byte & (1 << i) == 0)
    {
      driveLow(pinConfig.bus[i]);
    }
    else
    {
      driveHigh(pinConfig.bus[i]);
    }
  }
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
