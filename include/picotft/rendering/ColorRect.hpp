#ifndef PICOTFT_RENDERING_COLORRECT_HPP
#define PICOTFT_RENDERING_COLORRECT_HPP
#include "picotft/rendering/RectF.hpp"
#include "picotft/rendering/RenderObject.hpp"
#include "picotft/rendering/ShaderInvocationInfo.hpp"

class ColorRect : public RenderObject
{
public:
  ColorRect(const RectF &bounds, float z, int colorDepth, const char *pColor);
  bool runShader(const ShaderInvocationInfo &info) override;
  int getColorDepth() override;
private:
  int colorDepth;
  char *pColor;
};
#endif // PICOTFT_RENDERING_COLORRECT_HPP
