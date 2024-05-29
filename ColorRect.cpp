#include "picotft/ColorRect.hpp"

#include <cstring>
#include "picotft/RectF.hpp"
#include "picotft/RenderObject.hpp"
#include "picotft/ShaderInvocationInfo.hpp"

ColorRect::ColorRect(const RectF &bounds, float z, int colorDepth, const char *pColor)
  : RenderObject(bounds, z), colorDepth(colorDepth)
{
  this->pColor = new char[colorDepth];
  for (int i = 0; i < colorDepth; ++i)
  {
    this->pColor[i] = pColor[colorDepth - i - 1];
  }
}
bool ColorRect::runShader(const ShaderInvocationInfo &info)
{
  std::memcpy(info.pOutColor, pColor, colorDepth);
  return true;
}
int ColorRect::getColorDepth()
{
  return colorDepth;
}
