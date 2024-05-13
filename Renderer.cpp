#include "picotft/include/Renderer.hpp"

#include "picotft/include/RenderObject.hpp"
#include "picotft/include/Display.hpp"
#include "pico/multicore.h"
#include <map>
#include <mutex>
#include <unordered_set>

Renderer::Renderer(const Display &display, int tilesX, int tilesY)
  : display(display), tilesX(tilesX), tilesY(tilesY)
{
  pTiles = new std::map<float, std::unordered_set<RenderObject *>>[tilesX * tilesY];
  int displayWidth;
  int displayHeight;
  display.getSize(displayWidth, displayHeight);
  tileWidth = displayWidth / tilesX;
  tileHeight = displayHeight / tilesY;
}

void Renderer::addObject(RenderObject *pObject)
{
  for (int i = 0; i < tilesX * tilesY; ++i)
  {
    int tileX;
    int tileY;
    getNthTile(i, tileX, tileY);
    RectF tileRect;
    getTileRect(tileX, tileY, tileRect);
    if (pObject->doesIntersect(tileRect))
    {
      pTiles[i][pObject->getZ()].insert(pObject);
    }
  }
}
void Renderer::removeObject(RenderObject *pObject);
{
  for (int i = 0; i < tilesX * tilesY; ++i)
  {
    pTiles[i][pObject->getZ()].erase(pObject);
  }
}
void Renderer::render()
{
  multicore_reset_core1();
  multicore_fifo_push_blocking(static_cast<std::uint32_t>(this));
  multicore_launch_core1(alternateCoreRender);
  coreRender(0);
}

static void Renderer::alternateCoreRender()
{
  Renderer *that = static_cast<Renderer *>(multicore_fifo_pop_blocking());
  that->coreRender(1);
}
void Renderer::coreRender(int core)
{
  for (int i = core; i < tilesX * tilesY; i += 2)
  {
    renderTile(i);
  }
}
void Renderer::renderTile(int tileIdx)
{
  ShaderInvocationInfo info;
  int tileXPos;
  int tileYPos;
  getNthTile(tileIdx, tileXPos, tileYPos);
  tileXPos *= tileWidth;
  tileYPos *= tileHeight;
  std::uint16_t *pBuf = new std::uint16_t[tileWidth * tileHeight];
  info.pOutColor = pBuf;
  for (info.screenY = tileYpos; info.screenY < tileYPos + tileHeight; ++info.screenY)
  {
    for (info.screenX = tileXPos; info.screenX < tileXPos + tileWidth; ++info.screenX)
    {
      for (RenderObject *pObj : pTiles[tileIdx])
      {
        if (pObj->doesContainPoint(static_cast<float>(info.screenX),
          static_cast<float>(info.screenY)))
        {
          info.normFragX = static_cast<float>(info.screenX - pObj->getBounds().x)
            / pObj->getBounds().width;
          info.normFragY = static_cast<float>(info.screenY - pObj->getBounds().y)
            / pObj->getBounds().height;
          if (pObj->runShader(info))
          {
            // for now, the first non-discarded fragment is the last one we need to draw.
            // this changes if we decide to support alpha blending or depth buffering (right now we
            // just render front-to-back and hope for the best.)
            break;
          }
        }
      }
      ++info.pOutColor;
    }
  }
  {
    std::lock_guard<std::mutex> lock(tileWriteMutex);
    display.writePixelBlock(tileXPos, tileXPos + tileWidth - 1, tileYPos, tileYPos + tileHeight - 1, 
      reinterpret_cast<std::uint8_t *>(pBuf));
  }
}
void Renderer::getTileRect(int tileX, int tileY, RectF &rect)
{
  rect.x = tileX * tileWidth;
  rect.y = tileY * tileHeight;
  rect.width = tileWidth;
  rect.height = tileHeight;
}
void Renderer::getNthTile(int tileIdx, int &tileX, int &tileY)
{
  tileY = tileIdx / tilesX;
  tileX = tileIdx % tilesX;
}
