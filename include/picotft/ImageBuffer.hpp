#ifndef PICOTFT_IMAGEBUFFER_HPP
#define PICOTFT_IMAGEBUFFER_HPP
class ImageBuffer
{
public:
  ImageBuffer(int width, int height, int channels, int bytesPerPixel);
  ImageBuffer(int width, int height, int channels, int bytesPerPixel, const char *pLinearBuffer);
  void getRaw(int x, int y, void *pOut);
  void setRaw(int x, int y, void *pData);
  void getInterpolated(float x, float y, void *pOut);
  void copyFromLinear(const char *pLinearBuffer, int x, int y, int width, int height);
  void copyFromImage(const ImageBuffer &src, int x, int y, int width, int height);
  void getSize(int &width, int &height);

private:
  enum Layout
  {
    LINEAR
  };

  int width;
  int height;
  int channelCount;
  int bitsPerChannel;
  int bytesPerPixel;
  char *pBuffer;
  Layout layout;

  int getChannel(const char *pColor, int channel);
  int setChannel(char *pColor, int channel, int value);
};
#endif // PICOTFT_IMAGEBUFFER_HPP
