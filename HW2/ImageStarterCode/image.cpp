//CSCI 5607 HW 2 - Image Conversion Instructor: S. J. Guy <sjguy@umn.edu>
//In this assignment you will load and convert between various image formats.
//Additionally, you will manipulate the stored image data by quantizing, cropping, and supressing channels

#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <fstream>
using namespace std;

void write_ppm(char* imgName, int width, int height, int bits, const uint8_t* data) {
  //Open the texture image file
  ofstream ppmFile;
  ppmFile.open(imgName);
  if(!ppmFile) {
    printf("ERROR: Could not creat file '%s'\n", imgName);
    exit(1);
  }

  //Set this as an ASCII PPM (first line is P3)
  string PPM_style = "P3\n";
  ppmFile << PPM_style; //Read the first line of the header    

  //Write out the texture width and height
  ppmFile << width << " " << height << "\n";

  if(bits < 1 || bits > 8) {
    printf("ERROR: Unsupported bit value\n");
    exit(1);
  }

  //Set's the 3rd line to 255 (ie., assumes this is an 8 bit/pixel PPM)
  int maximum = ((2 << (bits - 1)) - 1);
  ppmFile << maximum << "\n";

  int scale = 255 / maximum;

  int r, g, b, a;
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      r = data[i * width * 4 + j * 4 + 0] / scale;  //Red
      g = data[i * width * 4 + j * 4 + 1] / scale;  //Green
      b = data[i * width * 4 + j * 4 + 2] / scale;  //Blue
      a = data[i * width * 4 + j * 4 + 3] / scale;  //Alpha
      ppmFile << r << " " << g << " " << b << " ";
    }
  }

  ppmFile.close();
}

uint8_t* read_ppm(char* imgName, int& width, int& height) {
  //Open the texture image file
  ifstream ppmFile;
  ppmFile.open(imgName);
  if(!ppmFile) {
    printf("ERROR: Image file '%s' not found.\n", imgName);
    exit(1);
  }

  //Check that this is an ASCII PPM (first line is P3)
  string PPM_style;
  ppmFile >> PPM_style; //Read the first line of the header    
  if(PPM_style != "P3") {
    printf("ERROR: PPM Type number is %s. Not an ASCII (P3) PPM file!\n", PPM_style.c_str());
    exit(1);
  }

  //Read in the texture width and height
  ppmFile >> width >> height;
  unsigned char* img_data = new unsigned char[4 * width * height];

  int maximum;
  ppmFile >> maximum;

  if(maximum < 1 || maximum > 255) {
    printf("ERROR: PPM bit depths only support between 0-255.\n");
    exit(1);
  }

  int scaler = 255 / maximum;

  int r, g, b;
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      ppmFile >> r >> g >> b;

      r *= scaler;
      g *= scaler;
      b *= scaler;

      img_data[i * width * 4 + j * 4] = r;  //Red
      img_data[i * width * 4 + j * 4 + 1] = g;  //Green
      img_data[i * width * 4 + j * 4 + 2] = b;  //Blue
      img_data[i * width * 4 + j * 4 + 3] = 255;  //Alpha
    }
  }

  return img_data;
}

/**
 * Image
 **/
Image::Image(int width_, int height_) {

  assert(width_ > 0);
  assert(height_ > 0);

  width = width_;
  height = height_;
  num_pixels = width * height;
  sampling_method = IMAGE_SAMPLING_POINT;

  data.raw = new uint8_t[num_pixels * 4];
  int b = 0; //which byte to write to
  for(int j = 0; j < height; j++) {
    for(int i = 0; i < width; i++) {
      data.raw[b++] = 0;
      data.raw[b++] = 0;
      data.raw[b++] = 0;
      data.raw[b++] = 0;
    }
  }

  assert(data.raw != NULL);
}

Image::Image(const Image& src) {
  width = src.width;
  height = src.height;
  num_pixels = width * height;
  sampling_method = IMAGE_SAMPLING_POINT;

  data.raw = new uint8_t[num_pixels * 4];

  //memcpy(data.raw, src.data.raw, num_pixels);
  *data.raw = *src.data.raw;
}

Image::Image(char* fname) {

  int lastc = strlen(fname);
  if(string(fname + lastc - 3) == "ppm") {
    data.raw = read_ppm(fname, width, height);
  } else {
    int numComponents; //(e.g., Y, YA, RGB, or RGBA)
    data.raw = stbi_load(fname, &width, &height, &numComponents, 4);
  }

  if(data.raw == NULL) {
    printf("Error loading image: %s", fname);
    exit(-1);
  }

  num_pixels = width * height;
  sampling_method = IMAGE_SAMPLING_POINT;

}

Image::~Image() {
  delete data.raw;
  data.raw = NULL;
}

void Image::Write(char* fname) {

  int lastc = strlen(fname);

  switch(fname[lastc - 1]) {
  case 'm': //ppm
    write_ppm(fname, width, height, export_depth, data.raw);
    break;
  case 'g': //jpeg (or jpg) or png
    if(fname[lastc - 2] == 'p' || fname[lastc - 2] == 'e') //jpeg or jpg
      stbi_write_jpg(fname, width, height, 4, data.raw, 95);  //95% jpeg quality
    else //png
      stbi_write_png(fname, width, height, 4, data.raw, width * 4);
    break;
  case 'a': //tga (targa)
    stbi_write_tga(fname, width, height, 4, data.raw);
    break;
  case 'p': //bmp
  default:
    stbi_write_bmp(fname, width, height, 4, data.raw);
  }
}

void Image::Brighten(double factor) {
  int x, y;
  for(x = 0; x < Width(); x++) {
    for(y = 0; y < Height(); y++) {
      Pixel p = GetPixel(x, y);
      Pixel scaled_p = p * factor;
      GetPixel(x, y) = scaled_p;
    }
  }
}

Image* Image::Crop(int x, int y, int w, int h) {
  Image* img = new Image(w, h);
  int u, v;
  for(u = 0; u < w; u++) {
    for(v = 0; v < h; v++) {
      img->SetPixel(u, v, GetPixel(x + u, y + v));
    }
  }

  return img;
}

void Image::ExtractChannel(int channel) {
  // Set all channels but "channel" to 0.
  int x, y;
  for(x = 0; x < Width(); x++) {
    for(y = 0; y < Height(); y++) {
      Pixel p = GetPixel(x, y);

      if(channel != 0) p.r = 0;
      if(channel != 1) p.g = 0;
      if(channel != 2) p.b = 0;

      GetPixel(x, y) = p;
    }
  }
}

void Image::Quantize(int nbits) {
  // Set all channels but "channel" to 0.
  int x, y;
  for(x = 0; x < Width(); x++) {
    for(y = 0; y < Height(); y++) {
      Pixel p = GetPixel(x, y);

      GetPixel(x, y) = PixelQuant(p, nbits);
    }
  }
}
