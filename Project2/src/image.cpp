//CSCI 5607 HW 2 - Image Conversion Instructor: S. J. Guy <sjguy@umn.edu>
//In this assignment you will load and convert between various image formats.
//Additionally, you will manipulate the stored image data by quantizing, cropping, and supressing channels

#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <random>
#include <iostream>
#include <fstream>
using namespace std;

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

  memcpy(data.raw, src.data.raw, num_pixels * 4);
  //*data.raw = *src.data.raw;
}

Image::Image(char* fname) {

  int lastc = strlen(fname);
  int numComponents; //(e.g., Y, YA, RGB, or RGBA)
  data.raw = stbi_load(fname, &width, &height, &numComponents, 4);

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

// Adds Gaussian noise to the image.
// The factor is related to the standard deviation
// of the normal distribution. This noise
// is a bit cleaner than uniform noise, more
// closely resembling the real picture.d Des  
void Image::AddNoise(double factor) {
  std::default_random_engine gen;
  std::normal_distribution<double> dist(0, factor * 100);

  int x, y;
  for(x = 0; x < Width(); x++) {
    for(y = 0; y < Height(); y++) {
      Pixel p = GetPixel(x, y);

      p.SetClamp(p.r + dist(gen), p.g + dist(gen), p.b + dist(gen));

      GetPixel(x, y) = p;
    }
  }
}

void Image::ChangeContrast(double factor) {
  int x, y;
  double luminance = 0;
  for(x = 0; x < Width(); x++) {
    for(y = 0; y < Height(); y++) {
      Pixel p = GetPixel(x, y);

      luminance += 0.3 * p.r + 0.59 * p.g + 0.11 * p.b;
    }
  }
  luminance /= (Width() * Height());
  for(x = 0; x < Width(); x++) {
    for(y = 0; y < Height(); y++) {
      SetPixel(x, y, PixelLerp(Pixel(luminance, luminance, luminance, 1), GetPixel(x, y), factor));
    }
  }
}

void Image::ChangeSaturation(double factor) {
  for(int x = 0; x < Width(); x++) {
    for(int y = 0; y < Height(); y++) {
      Pixel p = GetPixel(x, y);
      double luminance = 0.3 * p.r + 0.59 * p.g + 0.11 * p.b;
      SetPixel(x, y, PixelLerp(Pixel(luminance, luminance, luminance, 1), GetPixel(x, y), factor));
    }
  }
}

//For full credit, check that your dithers aren't making the pictures systematically brighter or darker
void Image::RandomDither(int nbits) {
  AddNoise(0.4);
  Quantize(nbits);
}

//This bayer method gives the quantization thresholds for an ordered dither.
//This is a 4x4 dither pattern, assumes the values are quantized to 16 levels.
//You can either expand this to a larger bayer pattern. Or (more likely), scale
//the threshold based on the target quantization levels.
static int Bayer4[4][4] = {
    {15,  7, 13,  5},
    { 3, 11,  1,  9},
    {12,  4, 14,  6},
    { 0,  8,  2, 10}
};

void Image::OrderedDither(int nbits) {
  Image* img_copy = new Image(*this);
}

/* Error-diffusion parameters */
const double
ALPHA = 7.0 / 16.0,
BETA = 3.0 / 16.0,
GAMMA = 5.0 / 16.0,
DELTA = 1.0 / 16.0;

void Image::FloydSteinbergDither(int nbits) {
  Image* img_copy = new Image(*this);

  for(int y = 1; y < Height() - 1; y++) {
    for(int x = 1; x < Width() - 1; x++) {
      // Ref: https://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering
      Pixel o = GetPixel(x, y);
      Pixel n = PixelQuant(o, nbits);
      SetPixel(x, y, n);
      Pixel error = o - n;

      GetPixel(x + 1, y) = GetPixel(x + 1, y) + (error * ALPHA);
      GetPixel(x - 1, y + 1) = GetPixel(x - 1, y + 1) + (error * BETA);
      GetPixel(x, y + 1) = GetPixel(x, y + 1) + (error * GAMMA);
      GetPixel(x + 1, y + 1) = GetPixel(x + 1, y + 1) + (error * DELTA);
    }
  }

  delete img_copy;
}

void Image::Blur(int n) {
  // Verify that n is positive and odd
  if(n < 3 || n % 2 == 0) {
    std::cerr << "Gaussian kernel matrix dimension must be positive and odd";
    exit(-1);
    return;
  }

  Image* img_copy = new Image(*this);
  double* kernel = new double[n * n];
  int r = n / 2;
  double sigma = r / 3.0;
  double c = 2 * sigma * sigma;
  double sum = 0;

  // 1) Create the Gaussian kernel
  for(int i = 0; i < n; i++) {
    for(int j = 0; j < n; j++) {
      int x = j - r;
      int y = i - r;

      // Take the gaussian in each direction
      double gx = std::exp(-x * x / c);
      double gy = std::exp(-y * y / c);

      kernel[i * n + j] = gx * gy;
      sum += kernel[i * n + j];
    }
  }

  // Normalize the Gaussian kernel to sum to one
  for(int i = 0; i < n * n; i++) {
    kernel[i] *= 1.0 / sum;
  }

  // Technically it would be better to do
  // linear decomposition to get separable filters

  // 2) Convolve the image
  for(int y = 0; y < Height(); y++) {
    for(int x = 0; x < Width(); x++) {
      Pixel p;

      for(int i = 0; i < n * n; i++) {
        int row = i / n;
        int col = i % n;

        p = p + img_copy->GetPixelSafe(x - r + col, y - r + row) * kernel[i];
      }

      SetPixel(x, y, p);
    }
  }

  delete img_copy;
}

void Image::Sharpen(int n) {
  Image* img_copy = new Image(*this);
  img_copy->Blur(n);

  for(int y = 0; y < Height(); y++) {
    for(int x = 0; x < Width(); x++) {
      Pixel p = GetPixel(x, y);
      p = p + (p - img_copy->GetPixel(x, y));
      SetPixel(x, y, p);
    }
  }

  delete img_copy;
}

void Image::EdgeDetect() {
  Image* img_copy = new Image(*this);
  int x, y;
  for(x = 0; x < Width(); x++) {
    for(y = 0; y < Height(); y++) {
      Pixel point = GetPixel(x, y);
      double average = (point.r + point.g + point.b) / 3.0;

      img_copy->GetPixel(x, y).SetClamp(average, average, average);
    }
  }

  double filter[9] = {
    -1.0, -1.0, -1.0, -1.0, 8.0, -1.0, -1.0, -1.0, -1.0
  };

  for(x = 0; x < Width(); x++) {
    for(y = 0; y < Height(); y++) {

      Pixel p;

      for(int i = 0; i < 9; i++) {
        int row = i / 3;
        int col = i % 3;

        p = p + img_copy->GetPixelSafe(x - 1 + col, y - 1 + row) * filter[i];
      }

      SetPixel(x, y, p);
    }
  }
}

Image* Image::Scale(double sx, double sy) {
  Image* img_copy = new Image((int)(sx * Width()), (int)(sy * Height()));
  
  for(int x = 0; x < (int)(Width() * sx); x++) {
    for(int y = 0; y < (int)(Height() * sy); y++) {

      img_copy->SetPixel(x, y, Sample(x / sx, y / sy));
    }
  }
  return img_copy;
}

Image* Image::Rotate(double angle) {
  Image* img_copy = new Image(Width(), Height());
  double half_width = Width() / 2;
  double half_height = Height() / 2;

  for(int x = 0; x < Width(); x++) {
    for(int y = 0; y < Height(); y++) {
      double o = std::atan2(y - half_height, x - half_width);
      double n = o - angle;
      double z1 = y - half_height;
      double z2 = x - half_width;
      double s = std::sqrt(z1 * z1 + z2 * z2);

      img_copy->SetPixelSafe(x, y, Sample(half_width + cos(n) * s, half_height + std::sin(n) * s));
    }
  }
  return img_copy;
}

void Image::Fun() {
  /* WORK HERE */
}

/**
 * Image Sample
 **/
void Image::SetSamplingMethod(int method) {
  assert((method >= 0) && (method < IMAGE_N_SAMPLING_METHODS));
  sampling_method = method;
}

Pixel Image::Sample(double u, double v) {
  if(sampling_method == IMAGE_SAMPLING_POINT) {
    Pixel closest = GetPixelSafe((int)u, (int)v);
    return closest;
  } else if(sampling_method == IMAGE_SAMPLING_BILINEAR) {
    // Rows
    Pixel top = PixelLerp(GetPixelSafe((int)u, (int)v), GetPixelSafe((int)std::ceil(u), (int)v), u - (int)u);
    Pixel bottom = PixelLerp(GetPixelSafe((int)u, (int)std::ceil(v)), GetPixelSafe((int)std::ceil(u), (int)std::ceil(v)), u - (int)u);

    // Interpolate vertically
    return PixelLerp(top, bottom, v);
  } else if(sampling_method == IMAGE_SAMPLING_GAUSSIAN) {
    u *= width;
    v *= height;
    Pixel p(0, 0, 0, 255);
    for(int x = u - 3; x < u + 3; x++) {
      for(int y = v - 3; y < v + 3; y++) {
        Pixel temp = GetPixelSafe(x, y);
        double d = -(std::pow(u - x, 2) + std::pow(v - y, 2)) / 2;
        p = p + (1.0 / (2 * 3.14159) * std::exp(d) * temp);
      }
    }
    return p;
  }
  return Pixel();
}