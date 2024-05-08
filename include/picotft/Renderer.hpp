#ifndef PICOTFT_RENDERER_HPP
#define PICOTFT_RENDERER_HPP
#include "picotft/include/Display.hpp"
#include "picotft/include/RenderObject.hpp"
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
  std::vector<RenderObject *> pObjects;
};
#endif // PICOTFT_RENDERER_HPP
