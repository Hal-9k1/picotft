#include "picotft/rendering/ImageBuffer.hpp"

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
ImageBuffer::ImageBuffer(int width, int height, int channels, int bytesPerPixel,
  const char *pLinearBuffer)
  : width(width), 
    height(height),
    channelCount(channels),
    bitsPerChannel(bytesPerPixel * 8 / channelCount),
    // BAD, but it's embedded so whatever. it's user error to call mutating methods on an
    // ImageBuffer initialized from const memory:
    pBuffer(const_cast<char *>(pLinearBuffer)),
    layout(LINEAR)
{ }

void ImageBuffer::getRaw(int x, int y, void *pOut)
{
  // linear storage, for now. consider tiles for better memory locality.
  std::memcpy(pOut, addressRaw(x, y), bytesPerPixel);
}
void ImageBuffer::setRaw(int x, int y, void *pData)
{
  // the implementation knows the const from addressRaw is artificial UNLESS the image memory is
  // read only, but in that case it's an error to call this function anyways.
  std::memcpy(const_cast<char *>(addressRaw(x, y)), pData, bytesPerPixel);
}
const char *ImageBuffer::addressRaw(int x, int y)
{
  return pBuffer + (x + y * width) * bytesPerPixel;
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
  unsigned char *pMem = new unsigned char[3 * bytesPerPixel];
  unsigned char *pLeftLerpColor = pMem;
  unsigned char *pRightLerpColor = pMem + bytesPerPixel;
  unsigned char *pResult = pMem + bytesPerPixel * 2;
  lerpColors(reinterpret_cast<const unsigned char *>(addressRaw(fx, fy)),
    reinterpret_cast<const unsigned char *>(addressRaw(fx, cy)), dy, pLeftLerpColor);
  lerpColors(reinterpret_cast<const unsigned char *>(addressRaw(cx, fy)),
    reinterpret_cast<const unsigned char *>(addressRaw(cx, cy)), dy, pRightLerpColor);
  lerpColors(pLeftLerpColor, pRightLerpColor, dx, pResult);
  std::memcpy(pOut, pResult, bytesPerPixel);
  delete[] pMem;
}
void ImageBuffer::copyFromLinear(const char *pLinearBuffer, int srcX, int srcY, int width,
  int height, int dstX, int dstY)
{
  for (int i = 0; i < height; ++i)
  {
    std::memcpy(pBuffer + bytesPerPixel * (dstX + (i + dstY) * width),
      pLinearBuffer + bytesPerPixel * (srcX + (i + srcY) * width),
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
  unsigned char *pMem = new unsigned char[bytesPerPixel * 2];
  unsigned char *pResultBuf = pMem;
  unsigned char *pMaskBuf = pMem + bytesPerPixel;
  valueToMsbBuffer(~(~0u << bitsPerChannel), pMaskBuf);
  for (int i = 0; i < bytesPerPixel; ++i)
  {
    // rshift by shift:
    int shiftCharCount = shift / 8;
    int shiftBitCount = shift % 8;
    unsigned char shiftedLow = shiftCharCount > i ? 0 : pColor[i - shiftCharCount];
    unsigned char shiftedHigh = shiftCharCount + 1 > i ? 0 : pColor[i - shiftCharCount - 1];
    unsigned char shiftedChar = shiftedLow >> shiftBitCount | shiftedHigh << (8 - shiftBitCount);
    // mask against pMaskBuf:
    pResultBuf[i] = shiftedChar & pMaskBuf[i];
  }
  int value = msbBufferToValue(pResultBuf);
  delete[] pMaskBuf;
  delete[] pResultBuf;
  return value;
}
void ImageBuffer::setChannel(unsigned char *pColor, int channel, int value)
{
  int shift = bitsPerChannel * (channelCount - channel - 1);
  unsigned char *pBuf = new unsigned char[bytesPerPixel];
  valueToMsbBuffer(value << shift, pBuf);
  for (int i = 0; i < bytesPerPixel; ++i)
  {
    pColor[i] |= pBuf[i];
  }
  delete[] pBuf;
}
void ImageBuffer::lerpColors(const unsigned char *pColorA, const unsigned char *pColorB, float fac,
  unsigned char *pOut)
{
  for (int i = 0; i < channelCount; ++i)
  {
    int channelA = getChannel(pColorA, i);
    int channelB = getChannel(pColorB, i);
    int lerped = channelA + static_cast<float>(channelB - channelA) * fac;
    setChannel(pOut, i, lerped);
  }
}
void ImageBuffer::valueToMsbBuffer(int value, unsigned char *pBuf)
{
  for (int i = 0; i < bytesPerPixel; ++i)
  {
    pBuf[bytesPerPixel - 1 - i] = (value >> (8 * i)) & 0xFF;
  }
}
int ImageBuffer::msbBufferToValue(const unsigned char *pBuf)
{
  int value = 0;
  for (int i = 0; i < bytesPerPixel; ++i)
  {
    value |= (pBuf[bytesPerPixel - 1 - i]) << (i * 8);
  }
  return value;
}
