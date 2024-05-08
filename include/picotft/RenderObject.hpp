#ifndef PICOTFT_RENDEROBJECT_HPP
#define PICOTFT_RENDEROBJECT_HPP
#include "picotft/include/RectF.hpp"
#include "picotft/include/ShaderInvocationInfo.hpp"

class RenderObject
{
public:
  const RectF &getBounds();
  float getZ();
  bool doesIntersect(const RectF &intersectRect);
  bool doesContainPoint(float x, float y);
  virtual void runShader(const ShaderInvocationInfo &info) = 0;
protected:
  RenderObject(const RectF &bounds, float z);
private:
  RectF bounds;
  float z;
};
#endif // PICOTFT_RENDEROBJECT_HPP
