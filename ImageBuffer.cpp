#include "picotft/ImageBuffer.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

template<typename T>
ImageBuffer<T>::ImageBuffer(int width, int height, int channels)
  : width(width), height(height), channelCount(channels), pBuffer(new T[width * height])
{ }

template<typename T>
T &ImageBuffer<T>::accessRaw(int x, int y)
{
  // linear storage, for now. consider tiles for better memory locality
  return pBuffer[x + y * width];
}
template<typename T>
T ImageBuffer<T>::getInterpolated(float x, float y)
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
template<typename T>
void ImageBuffer<T>::copyFromLinear(const char *pLinearBuffer, int srcX, int srcY, int width,
  int height, int dstX, int dstY)
{
  for (int i = srcY; i < height; ++i)
  {
    std::memcpy(pBuffer + sizeof(T) * (srcX + i * width),
      pLinearBuffer + sizeof(T) * (dstX + i * width),
      width);
  }
}
template<typename T>
void ImageBuffer<T>::copyFromImage(const ImageBuffer<T> &src, int srcX, int srcY, int width,
  int height, int dstX, int dstY)
{
  // images are currently stored linearly
  copyFromLinear(reinterpret_cast<const char *>(src.pBuffer), srcX, srcY, width, height, dstX,
    dstY);
}
template<typename T>
void ImageBuffer<T>::getSize(int &width, int &height)
{
  width = this->width;
  height = this->height;
}

template<typename T>
int ImageBuffer<T>::getChannel(const T &color, int channel)
{
  if (sizeof(T) == 2 && channelCount == 2)
  {
    switch (channel)
    {
    case 0:
      return color >> 11;
    case 1:
      return (color >> 5) & 0x3F;
    case 2:
      return color & 0x1F;
    }
  }
  int bitsPerChannel = sizeof(T) * 8 / channelCount;
  int shift = bitsPerChannel * (channelCount - channel - 1);
  int mask = ~(~1u << bitsPerChannel);
  return (color >> shift) & mask;
}
template<typename T>
int ImageBuffer<T>::setChannel(T &color, int channel, int value)
{
  int bitsPerChannel = sizeof(T) * 8 / channelCount;
  int shift = bitsPerChannel * (channelCount - channel - 1);
  color |= value << shift;
}
template<typename T>
int ImageBuffer<T>::lerpColors(const T &colorA, const T &colorB, float fac)
{
  T res;
  for (int i = 0; i < channelCount; ++i)
  {
    int channelA = getChannel(colorA, i);
    int channelB = getChannel(colorB, i);
    int lerped = channelA + static_cast<float>(channelB - channelA) * fac;
    setChannel(res, i, lerped);
  }
  return res;
}
