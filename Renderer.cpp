#include "picotft/Renderer.hpp"

#include "hardware/sync.h"
#include "picotft/RenderObject.hpp"
#include "picotft/Display.hpp"
#include "pico/multicore.h"
#include <cstdio>
#include <cstring>
#include <map>
#include <unordered_set>

Renderer::Renderer(Display &display, int tilesX, int tilesY)
  : display(display), tilesX(tilesX), tilesY(tilesY)
{
  pTiles = new std::map<float, std::unordered_set<RenderObject *>>[tilesX * tilesY];
  int displayWidth;
  int displayHeight;
  display.getSize(displayWidth, displayHeight);
  tileWidth = displayWidth / tilesX;
  tileHeight = displayHeight / tilesY;
  pPixelBufMemory = new std::uint16_t[tileWidth * tileHeight * 2]; // one tile buffer per core
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
void Renderer::removeObject(RenderObject *pObject)
{
  for (int i = 0; i < tilesX * tilesY; ++i)
  {
    pTiles[i][pObject->getZ()].erase(pObject);
  }
}
void Renderer::render()
{
  altCoreCompleted = false;
  uint spinlockId = static_cast<uint>(spin_lock_claim_unused(true));
  tileWriteSpinlock = spin_lock_instance(spinlockId);
  multicore_reset_core1();
  static_assert(sizeof(this) == sizeof(std::uint32_t),
    "Can't push pointer onto multicore FIFO if not building on 32-bit machine!");
  multicore_launch_core1(alternateCoreRender);
  multicore_fifo_push_blocking(reinterpret_cast<std::uint32_t>(this));
  coreRender(0);
  while (!altCoreCompleted)
  {
    __wfe();
  }
  spin_lock_unclaim(spinlockId);
}

void Renderer::alternateCoreRender()
{
  std::printf("Begin alternateCoreRender");
  Renderer *that = reinterpret_cast<Renderer *>(multicore_fifo_pop_blocking());
  that->coreRender(1);
}
void Renderer::coreRender(int core)
{
  std::printf("Core %d render began\n", core);
  for (int i = core; i < tilesX * tilesY; i += 2)
  {
    renderTile(i, pPixelBufMemory + core * tileWidth * tileHeight);
  }
  if (core)
  {
    altCoreCompleted = true;
    __sev();
  }
  std::printf("Core %d render completed\n", core);
}
void Renderer::renderTile(int tileIdx, std::uint16_t *pBuf)
{
  std::memset(pBuf, 0, tileWidth * tileHeight * 2); // clear with black
  ShaderInvocationInfo info;
  int tileXPos;
  int tileYPos;
  getNthTile(tileIdx, tileXPos, tileYPos);
  tileXPos *= tileWidth;
  tileYPos *= tileHeight;
  info.pOutColor = pBuf;
  for (info.screenY = tileYPos; info.screenY < tileYPos + tileHeight; ++info.screenY)
  {
    for (info.screenX = tileXPos; info.screenX < tileXPos + tileWidth; ++info.screenX)
    {
      for (const auto &depthObjSetPair : pTiles[tileIdx])
      {
        for (RenderObject *pObj : depthObjSetPair.second)
        {
          if (pObj->doesContainPoint(info.screenX, info.screenY))
          {
            info.normFragX = (info.screenX - pObj->getBounds().x) / pObj->getBounds().width;
            info.normFragY = (info.screenY - pObj->getBounds().y) / pObj->getBounds().height;
            if (pObj->runShader(info))
            {
              // for now, the first non-discarded fragment is the last one we need to draw.
              // this changes if we decide to support alpha blending or depth buffering (right now we
              // just render front-to-back and hope for the best.)
              break;
            }
          }
        }
      }
      info.pOutColor = static_cast<std::uint16_t *>(info.pOutColor) + 1;
    }
  }
  spin_lock_unsafe_blocking(tileWriteSpinlock);
  std::printf("Tile %d written from core %d\n", tileIdx, tileIdx % 2);
  display.writePixelBlock(tileXPos, tileXPos + tileWidth - 1, tileYPos, tileYPos + tileHeight - 1,
    true, reinterpret_cast<std::uint8_t *>(pBuf));
  spin_unlock_unsafe(tileWriteSpinlock);
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
