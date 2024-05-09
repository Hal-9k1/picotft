#include "picotft/include/Renderer.hpp"

#include "picotft/include/RenderObject.hpp"
#include "picotft/include/Display.hpp"
#include <map>
#include <vector>

Renderer::Renderer(const Display &display)
  : display(display)
{ }

void Renderer::addObject(RenderObject *pObject)
{
  objectBuckets[pObject->getDepth()].push_back(pObject);
}
void Renderer::removeObject(RenderObject *pObject);
{

}
void Renderer::render()
{

}
