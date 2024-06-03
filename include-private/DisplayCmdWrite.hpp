#ifndef DISPLAYCMDWRITE_HPP
#define DISPLAYCMDWRITE_HPP
#include <cstdint>

struct DisplayCmdWrite
{
  std::uint8_t cmdByte;
  int dataLen;
  bool isAlignPaddedToFourBytes; // enables faster transfers if swapBytePairs is not set
  int dataWrapLenLogTwo; // when not zero, assumes pData is aligned to and usefully fills 2^n bytes
  bool swapBytePairs; // assumes pData is aligned and padded to 2 bytes
  const std::uint8_t *pData;
  void *pRequestData; // additional userdata given to the handler
  void (*handler)(bool completed, const std::uint8_t *pData, void *pRequestData);
};
#endif // DISPLAYCMDWRITE_HPP
