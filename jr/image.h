/* a simple image class */

#ifndef IMAGE_H
#define IMAGE_H

#include <cstring>

class image {
 public:
  /* create an image */
  image(const int width, const int height, const bool colored);

  /* delete an image */
  ~image();

  /* init an image */
  void init(const unsigned char &val);

  /* copy an image */
  image *copy() const;
  
  /* get the width of an image. */
  int width() const { return w; }
  
  /* get the height of an image. */
  int height() const { return h; }
  
  /* image data. */
  unsigned char *data;
  
  /* row pointers. */
  unsigned char **access;
  
  int w, h, pixelCount;
  bool isRGB;
};

/* use imRef to access image data. */
#define imRef(im, x, y) (im->access[y][x])
  
/* use imPtr to get pointer to image data. */
#define imPtr(im, x, y) &(im->access[y][x])

image::image(const int width, const int height, const bool colored) {
  w = width;
  h = height;
  isRGB = colored;
  pixelCount = isRGB?3:1;
  data = new unsigned char[w * h * pixelCount];  // allocate space for image data
  access = new unsigned char*[h];   // allocate space for row pointers
  
  // initialize row pointers
  for (int i = 0; i < h; i++)
    access[i] = data + (i * w * pixelCount);  
}

image::~image() {
  delete [] data; 
  delete [] access;
}

void image::init(const unsigned char &val) {
  unsigned char *ptr = imPtr(this, 0, 0);
  unsigned char *end = imPtr(this, w-1, h-1);
  while (ptr <= end)
    *ptr++ = val;
}


image *image::copy() const {
  image *im = new image(w, h, isRGB);
  memcpy(im->data, data, w * h * pixelCount);
  return im;
}

#endif
  
