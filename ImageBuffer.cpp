#include "picotft/ImageBuffer.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

ImageBuffer::ImageBuffer(int width, int height, int channels, int bytesPerPixel)
  : width(width),
    height(height),
    channelCount(channels),
    bitsPerChannel(bytesPerPixel * 8 / channelCount),
    pBuffer(new char[width * height * bytesPerPixel]),
    layout(LINEAR)
{ }
ImageBuffer::ImageBuffer(int width, int height, int channels, const char *pLinearBuffer)
  : width(width), 
    height(height),
    channelCount(channels),
    bitsPerChannel(bytesPerPixel * 8 / channelCount),
    // BAD, but it's embedded so whatever. it's user error to call mutating methods on an
    // ImageBuffer initialized from const memory:
    pBuffer(const_cast<char *>(pLinearBuffer)),
    layout(LINEAR)
{ }

void ImageBuffer::getRaw(int x, int y, void *pOut);
{
  // linear storage, for now. consider tiles for better memory locality.
  std::memcpy(pOut, pBuffer + x + y * width, bytesPerPixel);
}
void ImageBuffer::getInterpolated(float x, float y, void *pOut)
{
  // bilinear interpolation
  int fx = std::floor(x);
  int fy = std::floor(y);
  int cx = std::ceil(x);
  int cy = std::ceil(y);
  int dx = x - fx;
  int dy = y - fy;
  return lerpColors(
    lerpColors(accessRaw(fx, fy), accessRaw(fx, cy), dy),
    lerpColors(accessRaw(cx, fy), accessRaw(cx, cy), dy),
    dx);
}
void ImageBuffer::copyFromLinear(const char *pLinearBuffer, int srcX, int srcY, int width,
  int height, int dstX, int dstY)
{
  for (int i = srcY; i < height; ++i)
  {
    std::memcpy(pBuffer + bytesPerPixel * (srcX + i * width),
      pLinearBuffer + bytesPerPixel * (dstX + i * width),
      width);
  }
}
void ImageBuffer::copyFromImage(const ImageBuffer &src, int srcX, int srcY, int width,
  int height, int dstX, int dstY)
{
  // images are currently stored linearly. when we add tile layout support we'll have to check
  // src.layout.
  copyFromLinear(reinterpret_cast<const char *>(src.pBuffer), srcX, srcY, width, height, dstX,
    dstY);
}
void ImageBuffer::getSize(int &width, int &height)
{
  width = this->width;
  height = this->height;
}

int ImageBuffer::getChannel(const unsigned char *pColor, int channel)
{
  if (bytesPerPixel == 2 && channelCount == 2)
  {
    switch (channel)
    {
    case 0:
      return pColor[0] >> 3;
    case 1:
      return (pColor[1] >> 5) | ((pColor[0] & 7) << 3);
    case 2:
      return pColor[1] & 0x1F;
    }
  }
  int shift = bitsPerChannel * (channelCount - channel - 1);
  int mask = ~(~1u << bitsPerChannel);
  return (color >> shift) & mask;
}
int ImageBuffer::setChannel(unsigned char *pColor, int channel, int value)
{
  int shift = bitsPerChannel * (channelCount - channel - 1);
  color |= value << shift;
}
int ImageBuffer::lerpColors(const unsigned char *pColorA, const unsigned char *pColorB, float fac,
  unsigned char *pOut)
{
  for (int i = 0; i < channelCount; ++i)
  {
    int channelA = getChannel(colorA, i);
    int channelB = getChannel(colorB, i);
    int lerped = channelA + static_cast<float>(channelB - channelA) * fac;
    setChannel(res, i, lerped);
  }
}
