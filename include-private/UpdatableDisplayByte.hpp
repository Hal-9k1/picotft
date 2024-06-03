#ifndef UPDATABLEDISPLAYBYTE_HPP
#define UPDATABLEDISPLAYBYTE_HPP
#include <cstdint>
#include "picotft/DisplayIO.hpp"

class UpdatableDisplayByte
{
public:
  UpdatableDisplayByte(DisplayIO *pDisplayIO, std::uint8_t setCmdByte);
  int set(std::uint8_t value);
  std::uint8_t get();

private:
  static void handleWrite(bool completed, const std::uint8_t *pData, void *pRequestData);

  DisplayIO *pDisplayIO;
  std::uint8_t cmdByte;
  int modifyCounter;
  int modifyRequestCounter;
  int writeTicket;
  std::uint8_t value;
};

#endif // UPDATABLEDISPLAYBYTE_HPP
