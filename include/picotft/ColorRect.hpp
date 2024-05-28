#ifndef PICOTFT_COLORRECT_HPP
#define PICOTFT_COLORRECT_HPP
#include "picotft/RectF.hpp"
#include "picotft/RenderObject.hpp"
#include "picotft/ShaderInvocationInfo.hpp"

class ColorRect : RenderObject
{
public:
  ColorRect(const RectF &bounds, float z, int colorDepth, const char *pColor);
  bool runShader(const ShaderInvocationInfo &info) override;
  int getColorDepth() override;
private:
  int colorDepth;
  char *pColor;
};
#endif // PICOTFT_COLORRECT_HPP
