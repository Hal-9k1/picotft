#ifndef PICOTFT_RENDERER_HPP
#define PICOTFT_RENDERER_HPP
#include "picotft/include/Display.hpp"
#include "picotft/include/RenderObject.hpp"
#include <map>
#include <vector>

class Renderer
{
public:
  Renderer(const Display &display);
  void addObject(RenderObject *pObject);
  void removeObject(RenderObject *pObject);
  void render();

private:
  const Display &display;
  std::map<float, std::unordered_set<RenderObject *>> objectBuckets;
};
#endif // PICOTFT_RENDERER_HPP
