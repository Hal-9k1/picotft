#ifndef PICOTFT_IMAGEBUFFER_HPP
#define PICOTFT_IMAGEBUFFER_HPP
template<typename T>
class ImageBuffer
{
public:
  ImageBuffer(int width, int height, int channels);
  T &accessRaw(int x, int y);
  T getInterpolated(float x, float y);
  void copyFromLinear(const char *pLinearBuffer, int x, int y, int width, int height);
  template <typename U>
  void copyFromImage(const ImageBuffer<U> &src, int x, int y, int width, int height);
  void getSize(int &width, int &height);

private:
  T *pBuffer;
  int width;
  int height;
  int channelCount;

  int getChannel(const T &color, int channel);
  int setChannel(T &color, int channel, int value);
};

template class ImageBuffer<std::uint8_t>;
template class ImageBuffer<std::uint16_t>;
template class ImageBuffer<std::uint32_t>;
#endif // PICOTFT_IMAGEBUFFER_HPP
