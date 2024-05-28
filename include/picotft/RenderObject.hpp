#ifndef PICOTFT_RENDEROBJECT_HPP
#define PICOTFT_RENDEROBJECT_HPP
#include "picotft/RectF.hpp"
#include "picotft/ShaderInvocationInfo.hpp"

class RenderObject
{
public:
  const RectF &getBounds();
  float getZ();
  bool doesIntersect(const RectF &intersectRect);
  bool doesContainPoint(float x, float y);
  virtual bool runShader(const ShaderInvocationInfo &info) = 0;
  virtual int getColorDepth() = 0;
protected:
  RenderObject(const RectF &bounds, float z);
private:
  RectF bounds;
  float z;
};
#endif // PICOTFT_RENDEROBJECT_HPP
