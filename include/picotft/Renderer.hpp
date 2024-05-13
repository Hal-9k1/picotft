#ifndef PICOTFT_RENDERER_HPP
#define PICOTFT_RENDERER_HPP
#include "picotft/include/Display.hpp"
#include "picotft/include/RenderObject.hpp"
#include <map>
#include <mutex>
#include <unordered_set>

class Renderer
{
public:
  Renderer(const Display &display, int tilesX, int tilesY);
  void addObject(RenderObject *pObject);
  void removeObject(RenderObject *pObject);
  void render();

private:
  const Display &display;
  int tilesX;
  int tilesY;
  int tileWidth;
  int tileHeight;
  std::map<float, std::unordered_set<RenderObject *>> *pTiles;
  std::mutex tileWriteMutex;

  static void alternateCoreRender();
  void coreRender(int core);
  void renderTile(int tileIdx);
  void getTileRect(int tileX, int tileY, RectF &rect);
  void getNthTile(int tileIdx, int &tileX, int &tileY);
};
#endif // PICOTFT_RENDERER_HPP
