#ifndef PICOTFT_IMAGEBUFFER_HPP
#define PICOTFT_IMAGEBUFFER_HPP
class ImageBuffer
{
public:
  ImageBuffer(int width, int height, int channels, int bytesPerPixel);
  ImageBuffer(int width, int height, int channels, int bytesPerPixel, const char *pLinearBuffer);
  void getRaw(int x, int y, void *pOut);
  void setRaw(int x, int y, void *pData);
  const char *addressRaw(int x, int y);
  void getInterpolated(float x, float y, void *pOut);
  void copyFromLinear(const char *pLinearBuffer, int srcX, int srcY, int width, int height,
    int dstX, int dstY);
  void copyFromImage(const ImageBuffer &src, int srcX, int srcY, int width, int height, int dstX,
    int dstY);
  void getSize(int &width, int &height);

private:
  enum Layout
  {
    LINEAR
  };

  int width;
  int height;
  int channelCount;
  int bitsPerChannel; // not meaningful if channelCount == 2 && bytesPerPixel == 2
  int bytesPerPixel;
  char *pBuffer;
  Layout layout;

  int getChannel(const unsigned char *pColor, int channel);
  void setChannel(unsigned char *pColor, int channel, int value);
  void lerpColors(const unsigned char *pColorA, const unsigned char *pColorB, float fac,
    unsigned char *pOut);
  void valueToMsbBuffer(int value, unsigned char *pBuf);
  int msbBufferToValue(const unsigned char *pBuf);
};
#endif // PICOTFT_IMAGEBUFFER_HPP
