#ifndef PICOTFT_RENDERING_SHADERINVOCATIONINFO_HPP
#define PICOTFT_RENDERING_SHADERINVOCATIONINFO_HPP
#include <cstdint>

struct ShaderInvocationInfo
{
  float normFragX;
  float normFragY;
  float screenX;
  float screenY;
  void *pOutColor;
};
#endif // PICOTFT_RENDERING_SHADERINVOCATIONINFO_HPP
