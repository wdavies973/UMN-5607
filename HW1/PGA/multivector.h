//CSCI 5607 PGA Library
//This header defines the following 2D PGA operations:
// -wedge product, dot product, vee product, dual, reverse

#ifndef MULTIVECTOR_H
#define MULTIVECTOR_H

#include <cmath>

#include "primitives.h"

// Resources:
  // - http://www.math.umd.edu/~immortal/MATH431/lecturenotes/ch_geometricalgebra.pdf
  // - https://bivector.net/
  // We'll need to know what these operations do in order to use them.

/*
Cliff's Notes PGA

Scalars
  - Grade 0
  - Span 1 dimension only (reals)
Vectors
  - Grade 1
  - Span n dimensions, have n basis'
Bivectors
  - Grade 2
  - Comes about from the "wedge product"
  - b ^ a = -(a ^ b)
  - a ^ a = 0
  - Think of a bivector as a span. It's a parallelogram
  - A parallelogram is defined by two vectors.
  - Wedge product implies an orientation. (following)
Trivectors
  - Grade 3
  - Following from the definition of bivector, its a parallelopiped
  - Orientation defined by right hand rule

Operations:
  - Wedge product moves up a dimension
  - Inner product (dot) moves down a dimension
  - Geometric product
    - ab = a * b + a ^ b (geometric vector is dot product plus wedge product)
    - Geometric product of sames is 1
    - Geometric of different is bivector

*/

// Generally speaking, a multivector is an abstract on vectors that
// allow standard operations to be done more easily.

// A multivector contains scalars, bivectors, trivectors, etc.
// It looks like this specific implementation handles up to bivectors.
struct MultiVector {
  //Constructor
  // s - scalar
  // x,y,w - vectors
  // yw, wx, xy - bi vector components
  // trivector scalar
  MultiVector(float s = 0, float x = 0, float y = 0, float w = 0, float yw = 0, float wx = 0, float xy = 0, float wxy = 0) :
    s(s), x(x), y(y), w(w), yw(yw), wx(wx), xy(xy), wxy(wxy) {
  }

  MultiVector(Point2D p) : s(0), x(0), y(0), w(0), yw(p.x), wx(p.y), xy(1), wxy(0) {}
  MultiVector(Dir2D d) : s(0), x(0), y(0), w(0), yw(d.x), wx(d.y), xy(0), wxy(0) {}
  MultiVector(Motor2D m) : s(m.s), x(0), y(0), w(0), yw(m.yw), wx(m.wx), xy(m.xy), wxy(0) {}
  MultiVector(Line2D l) : s(0), x(l.x), y(l.y), w(l.w), yw(0), wx(0), xy(0), wxy(0) {}

  float s; //Scalar
  float x, y, w; //Vector Components
  float yw, wx, xy; //BiVector Components
  float wxy; //Pseudoscalar

  // What if we had the vector 1,1, x =1, y = 1

  MultiVector times(MultiVector rhs) {
    float _s = s * rhs.s + x * rhs.x + y * rhs.y - xy * rhs.xy;
    float _x = s * rhs.x + x * rhs.s - y * rhs.xy + xy * rhs.y;
    float _y = s * rhs.y + x * rhs.xy + y * rhs.s - xy * rhs.x;
    float _w = s * rhs.w - x * rhs.wx + y * rhs.yw + w * rhs.s - yw * rhs.y + wx * rhs.x - xy * rhs.wxy - wxy * rhs.xy;
    float _yw = s * rhs.yw + x * rhs.wxy + y * rhs.w - w * rhs.y + yw * rhs.s - wx * rhs.xy + xy * rhs.wx + wxy * rhs.x;
    float _wx = s * rhs.wx - x * rhs.w + y * rhs.wxy + w * rhs.x + yw * rhs.xy + wx * rhs.s - xy * rhs.yw + wxy * rhs.y;
    float _xy = s * rhs.xy + x * rhs.y - y * rhs.x + xy * rhs.s;
    float _wxy = s * rhs.wxy + x * rhs.yw + y * rhs.wx + w * rhs.xy + yw * rhs.x + wx * rhs.y + xy * rhs.w + wxy * rhs.s;
    return MultiVector(_s, _x, _y, _w, _yw, _wx, _xy, _wxy);
  }

  MultiVector add(MultiVector rhs) {
    float _s = s + rhs.s;
    float _x = x + rhs.x;
    float _y = y + rhs.y;
    float _w = w + rhs.w;
    float _yw = yw + rhs.yw;
    float _wx = wx + rhs.wx;
    float _xy = xy + rhs.xy;
    float _wxy = wxy + rhs.wxy;
    return MultiVector(_s, _x, _y, _w, _yw, _wx, _xy, _wxy);
  }

  MultiVector sub(MultiVector rhs) {
    float _s = s - rhs.s;
    float _x = x - rhs.x;
    float _y = y - rhs.y;
    float _w = w - rhs.w;
    float _yw = yw - rhs.yw;
    float _wx = wx - rhs.wx;
    float _xy = xy - rhs.xy;
    float _wxy = wxy - rhs.wxy;
    return MultiVector(_s, _x, _y, _w, _yw, _wx, _xy, _wxy);
  }

  // Scales the multivector, it's homogenous so this shouldn't have an effect
  MultiVector mul(float f) {
    return MultiVector(s * f, x * f, y * f, w * f, yw * f, wx * f, xy * f, wxy * f);
  }

  // Scales the multivector, it's homoegenous so this shouldn't have an effect
  MultiVector div(float f) {
    return MultiVector(s / f, x / f, y / f, w / f, yw / f, wx / f, xy / f, wxy / f);
  }

  // Computes the wedge product
  MultiVector wedge(MultiVector rhs) {
    float _s = s * rhs.s;
    float _x = s * rhs.x + x * rhs.s;
    float _y = s * rhs.y + y * rhs.s;
    float _w = s * rhs.w + w * rhs.s;
    float _yw = s * rhs.yw + y * rhs.w - w * rhs.y + yw * rhs.s;
    float _wx = s * rhs.wx - x * rhs.w + w * rhs.x + wx * rhs.s;
    float _xy = s * rhs.xy + x * rhs.y - y * rhs.x + xy * rhs.s;
    float _wxy = s * rhs.wxy + x * rhs.yw + y * rhs.wx + w * rhs.xy + yw * rhs.x + wx * rhs.y + xy * rhs.w + wxy * rhs.s;
    return MultiVector(_s, _x, _y, _w, _yw, _wx, _xy, _wxy);
  }

  // Finds the dot product
  MultiVector dot(MultiVector rhs) { // dot(ei,ej) = 1 if i==j, 0 elsewise
    float _s = s * rhs.s + x * rhs.x + y * rhs.y - xy * rhs.xy;
    float _x = s * rhs.x - y * rhs.xy + xy * rhs.y - x * rhs.s;
    float _y = s * rhs.y + x * rhs.xy - xy * rhs.x + y * rhs.s;
    float _w = s * rhs.w - x * rhs.wx + y * rhs.yw + w * rhs.s - yw * rhs.y + wx * rhs.x - xy * rhs.wxy - wxy * rhs.xy;
    float _yw = s * rhs.yw + x * rhs.wxy + yw * rhs.s + wxy * rhs.x;
    float _wx = s * rhs.wx - y * rhs.wxy + wx * rhs.s + wxy * rhs.y;
    float _xy = s * rhs.xy + xy * rhs.s;
    float _wxy = s * rhs.wxy + wxy * rhs.s;
    return MultiVector(_s, _x, _y, _w, _yw, _wx, _xy, _wxy);
  }

  // The magnitude of the wedge product
  float magnitude() {
    return std::sqrt(this->times(reverse()).s);
  }

  MultiVector normalized() {
    return this->times(1 / magnitude());
  }

  MultiVector vee(MultiVector rhs) {
    return dual().wedge(rhs.dual()).dual();
  }

  MultiVector reverse() {
    return MultiVector(s, x, y, w, -yw, -wx, -xy, -wxy);
  }

  MultiVector dual() {
    return MultiVector(wxy, yw, wx, xy, x, y, w, s);
  }

  MultiVector transform(MultiVector m) {
    MultiVector self = *this;
    return m.times(self).times(m.reverse());
  }

  operator std::string() const {
    char result[200];
    sprintf(result, "%.2f + %.2fx + %.2fy + %.2fw + %.2fyw + %.2fwx + %.2fxy + %.2fwxy", s, x, y, w, yw, wx, xy, wxy);
    return result;
  }

  void print(const char* title = "") {
    printf("MutiVector:%s %s\n", title, std::string(*this).c_str());
  }
};


//MultiVector products
inline MultiVector wedge(MultiVector lhs, MultiVector rhs) {
  return lhs.wedge(rhs);
}

inline MultiVector dot(MultiVector lhs, MultiVector rhs) {
  return lhs.dot(rhs);
}

//Various Operator Overloads
inline MultiVector operator*(MultiVector lhs, MultiVector rhs) {
  return lhs.times(rhs);
}

inline MultiVector operator+(MultiVector lhs, MultiVector rhs) {
  return lhs.add(rhs);
}

inline MultiVector operator-(MultiVector lhs, MultiVector rhs) {
  return lhs.sub(rhs);
}

inline MultiVector operator/(MultiVector lhs, float f) {
  return lhs.div(f);
}

inline MultiVector operator*(MultiVector lhs, float f) {
  return lhs.mul(f);
}

inline MultiVector operator*(float f, MultiVector rhs) {
  return rhs.mul(f);
}

inline bool operator>(MultiVector lhs, float f) {
  return lhs.s > f;
}

inline bool operator>(float f, MultiVector rhs) {
  return f > rhs;
}

inline bool operator<(MultiVector lhs, float f) {
  return lhs.s < f;
}

inline bool operator<(float f, MultiVector rhs) {
  return f < rhs;
}

//Motor
inline MultiVector transform(MultiVector lhs, MultiVector m) {
  return m * lhs * m.reverse();
}

#endif