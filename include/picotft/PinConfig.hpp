#ifndef PICOTFT_PINCONFIG_HPP
#define PICOTFT_PINCONFIG_HPP

#include "pico/types.h"

struct PinConfig
{
  uint bus[8];
  uint chipSel;
  uint cmdSwitch;
  uint writeStrobe;
  uint readStrobe;
  uint reset;
};

#endif // PICOTFT_PINCONFIG_HPP
