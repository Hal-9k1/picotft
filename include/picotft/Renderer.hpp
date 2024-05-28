#ifndef PICOTFT_RENDERER_HPP
#define PICOTFT_RENDERER_HPP
#include "hardware/sync.h"
#include "picotft/Display.hpp"
#include "picotft/RenderObject.hpp"
#include <map>
#include <unordered_set>

class Renderer
{
public:
  Renderer(Display &display, int tilesX, int tilesY);
  void addObject(RenderObject *pObject);
  void removeObject(RenderObject *pObject);
  void render();

private:
  Display &display;
  int tilesX;
  int tilesY;
  int tileWidth;
  int tileHeight;
  std::uint16_t *pPixelBufMemory;
  std::map<float, std::unordered_set<RenderObject *>> *pTiles;
  spin_lock_t *tileWriteSpinlock;

  static void alternateCoreRender();
  void coreRender(int core);
  void renderTile(int tileIdx, std::uint16_t *pBuf);
  void getTileRect(int tileX, int tileY, RectF &rect);
  void getNthTile(int tileIdx, int &tileX, int &tileY);
};
#endif // PICOTFT_RENDERER_HPP
