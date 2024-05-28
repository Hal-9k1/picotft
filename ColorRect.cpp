#include "picotft/ColorRect.hpp"

#include "picotft/RectF.hpp"
#include "picotft/RenderObject.hpp"
#include "picotft/ShaderInvocationInfo.hpp"

ColorRect::ColorRect(const RectF &bounds, float z, int colorDepth, const char *pColor)
  : RenderObject(bounds, z), colorDepth(colorDepth)
{
  this->pColor = new char[colorDepth];
  std::memcpy(this->pColor, pColor, colorDepth);
}
bool ColorRect::runShader(const ShaderInvocationInfo &info)
{
  std::memcpy(info.pOutColor, pColor, colorDepth);
}
int ColorRect::getColorDepth()
{
  return colorDepth;
}
