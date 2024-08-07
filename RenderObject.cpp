#include "picotft/RenderObject.hpp"

#include "picotft/RectF.hpp"

const RectF &RenderObject::getBounds()
{
  return bounds;
}
float RenderObject::getZ()
{
  return z;
}
bool RenderObject::doesIntersect(const RectF &intersectRect)
{
  return bounds.x < intersectRect.x + intersectRect.width
    && bounds.x + bounds.width > intersectRect.x
    && bounds.y < intersectRect.y + intersectRect.height
    && bounds.y + bounds.height > intersectRect.y;
}
bool RenderObject::doesContainPoint(float x, float y)
{
  return bounds.x < x && bounds.x + bounds.width > x
    && bounds.y < y && bounds.y + bounds.height > y;
}
RenderObject::RenderObject(const RectF &bounds, float z)
  : bounds(bounds), z(z)
{ }
