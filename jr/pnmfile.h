/* basic image I/O */

#ifndef PNM_FILE_H
#define PNM_FILE_H

#include <cstdlib>
#include <climits>
#include <cstring>
#include <fstream>
#include "image.h"
#include "misc.h"

#define BUF_SIZE 256

class pnm_error { };

static void read_packed(unsigned char *data, int size, std::ifstream &f) {
  unsigned char c = 0;
  
  int bitshift = -1;
  for (int pos = 0; pos < size; pos++) {
    if (bitshift == -1) {
      c = f.get();
      bitshift = 7;
    }
    data[pos] = (c >> bitshift) & 1;
    bitshift--;
    }
}

static void write_packed(unsigned char *data, int size, std::ofstream &f) {
  unsigned char c = 0;
  
  int bitshift = 7;
  for (int pos = 0; pos < size; pos++) {
      c = c | (data[pos] << bitshift);
      bitshift--;
      if ((bitshift == -1) || (pos == size-1)) {
	f.put(c);
	bitshift = 7;
	c = 0;
      }
  }
}

/* read PNM field, skipping comments */ 
static void pnm_read(std::ifstream &file, char *buf) {
  char doc[BUF_SIZE];
  char c;
  
  file >> c;
  while (c == '#') {
    file.getline(doc, BUF_SIZE);
    file >> c;
  }
  file.putback(c);
  
  file.width(BUF_SIZE);
  file >> buf;
  file.ignore();
}

static image *loadPBM(const char *name) {
  char buf[BUF_SIZE];
  
  /* read header */
  std::ifstream file(name, std::ios::in | std::ios::binary);
  pnm_read(file, buf);
  if (strncmp(buf, "P4", 2))
    throw pnm_error();
    
  pnm_read(file, buf);
  int width = atoi(buf);
  pnm_read(file, buf);
  int height = atoi(buf);
  
  /* read data */
  image *im = new image(width, height, false);
  for (int i = 0; i < height; i++)
    read_packed(imPtr(im, 0, i), width, file);
  
  return im;
}

static void savePBM(image *im, const char *name) {
  int width = im->width();
  int height = im->height();
  std::ofstream file(name, std::ios::out | std::ios::binary);

  file << "P4\n" << width << " " << height << "\n";
  for (int i = 0; i < height; i++)
    write_packed(imPtr(im, 0, i), width, file);
}

static image *loadPGM(const char *name) {
  char buf[BUF_SIZE];
  
  /* read header */
  std::ifstream file(name, std::ios::in | std::ios::binary);
  pnm_read(file, buf);
  if (strncmp(buf, "P5", 2))
    throw pnm_error();

  pnm_read(file, buf);
  int width = atoi(buf);
  pnm_read(file, buf);
  int height = atoi(buf);

  pnm_read(file, buf);
  if (atoi(buf) > UCHAR_MAX)
    throw pnm_error();

  /* read data */
  image *im = new image(width, height, false);
  file.read((char *)imPtr(im, 0, 0), width * height * sizeof(uchar));

  return im;
}

static void savePGM(image *im, const char *name) {
  int width = im->width();
  int height = im->height();
  std::ofstream file(name, std::ios::out | std::ios::binary);

  file << "P5\n" << width << " " << height << "\n" << UCHAR_MAX << "\n";
  file.write((char *)imPtr(im, 0, 0), width * height * sizeof(uchar));
}

static image *loadPPM(const char *name) {
  char buf[BUF_SIZE], doc[BUF_SIZE];
  
  /* read header */
  std::ifstream file(name, std::ios::in | std::ios::binary);
  pnm_read(file, buf);
  if (strncmp(buf, "P6", 2))
    throw pnm_error();

  pnm_read(file, buf);
  int width = atoi(buf);
  pnm_read(file, buf);
  int height = atoi(buf);

  pnm_read(file, buf);
  if (atoi(buf) > UCHAR_MAX)
    throw pnm_error();

  /* read data */
  image *im = new image(width, height, true);
  file.read((char *)imPtr(im, 0, 0), width * height * 3 * sizeof(uchar));

  return im;
}

static void savePPM(image *im, const char *name) {
  int width = im->width();
  int height = im->height();
  std::ofstream file(name, std::ios::out | std::ios::binary);

  file << "P6\n" << width << " " << height << "\n" << UCHAR_MAX << "\n";
  file.write((char *)imPtr(im, 0, 0), width * height * 3 * sizeof(uchar));
}

void load_image(image **im, const char *name) {
  char buf[BUF_SIZE];
  
  /* read header */
  std::ifstream file(name, std::ios::in | std::ios::binary);
  char c;
  file.seekg(1, ios::beg);
  c = file.peek();
  file.seekg(0, ios::beg);
  pnm_read(file, buf);
  if (strncmp(buf, "VLIB", 9))
    throw pnm_error();

  pnm_read(file, buf);
  int width = atoi(buf);
  pnm_read(file, buf);
  int height = atoi(buf);
  //if second character in document is 6, then this is RGB
  char num = (c==6);
  

  /* read data */
  *im = new image(width, height, num);
  file.read((char *)imPtr((*im), 0, 0), width * height * sizeof(uchar));
}

void save_image(image *im, const char *name) {
  int width = im->width();
  int height = im->height();
  std::ofstream file(name, std::ios::out | std::ios::binary);

  file << "VLIB\n" << width << " " << height << "\n";
  file.write((char *)imPtr(im, 0, 0), width * height * sizeof(uchar));
}

#endif
