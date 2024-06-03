#ifndef PICOTFT_PINCONFIG_HPP
#define PICOTFT_PINCONFIG_HPP

#include "pico/types.h"

struct PinConfig
{
  uint busBase;
  uint chipSelCmdSwitchBase;
  uint writeStrobe;
  uint readStrobe;
  uint reset;
};

#endif // PICOTFT_PINCONFIG_HPP
