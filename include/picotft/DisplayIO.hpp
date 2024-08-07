#ifndef PICOTFT_DISPLAYIO_HPP
#define PICOTFT_DISPLAYIO_HPP
#include <cstdint>
#include "pico/types.h"
#include "picotft/PinConfig.hpp"

class DisplayIO
{
public:
  DisplayIO(const PinConfig &pinConfig, uint readBufferLength);
  ~DisplayIO();

  void writeCmd(std::uint8_t cmdByte, uint dataLen, bool swapBytePairs, const std::uint8_t *pDataBytes);
  void writeCmdHeader(std::uint8_t cmdByte);
  void writeByte(std::uint8_t byte);
  void endCmdWrite();
  std::uint8_t waitReadByte();
  bool tryReadByte(std::uint8_t &outByte);

private:
  // I hate singletons but gpio_irq_callback_t doesn't have a userdata parameter and by the looks of
  // things (https://github.com/raspberrypi/pico-sdk/issues/756) won't for a long time
  static DisplayIO *pInstance;

  std::uint8_t *buf;
  uint bufLen;
  uint bufWriteCursor;
  uint bufReadCursor;
  bool bufFull;
  bool bufOverflow;
  PinConfig pinConfig;

  static void handleReadInterrupt(uint gpio, std::uint32_t events);

  void pullHigh(uint pin);
  void pullLow(uint pin);
  void driveHigh(uint pin);
  void driveLow(uint pin);
};
#endif // PICOTFT_DISPLAYIO_HPP
