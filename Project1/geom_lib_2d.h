//UMN CSCI 5607 2D Geometry Library Homework [HW0]
// davie304

#ifndef GEOM_LIB_H
#define GEOM_LIB_H

#include "pga.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifdef MAX
#undef MAX
#endif
#define MAX max

#ifdef MIN
#undef MIN
#endif
#define MIN min

//Displace a point p on the direction d
//The result is a point
Point2D move(Point2D p, Dir2D d) {
  return p + d;
}

//Compute the displacement vector between points p1 and p2
//The result is a direction 
Dir2D displacement(Point2D p1, Point2D p2) {
  return p2 - p1;
}

//Compute the distance between points p1 and p2
//The result is a scalar 
float dist(Point2D p1, Point2D p2) {
  return (p2 - p1).magnitude();
}

//Compute the perpendicular distance from the point p the the line l
//The result is a scalar 
float dist(Line2D l, Point2D p) {
  return vee(p, l.normalized());
}

//Compute the perpendicular distance from the point p the the line l
//The result is a scalar 
float dist(Point2D p, Line2D l) {
  return vee(p, l.normalized());
}

//Compute the intersection point between lines l1 and l2
//You may assume the lines are not parallel
//The results is a a point that lies on both lines
Point2D intersect(Line2D l1, Line2D l2) {
  return wedge(l1, l2);
}

//Compute the line that goes through the points p1 and p2
//The result is a line 
Line2D join(Point2D p1, Point2D p2) {
  return vee(p1, p2);
}

//Compute the projection of the point p onto line l
//The result is the closest point to p that lies on line l
Point2D project(Point2D p, Line2D l) {
  return dot(l, p) * l;
}

//Compute the projection of the line l onto point p
//The result is a line that lies on point p in the same direction of l
Line2D project(Line2D l, Point2D p) {
  return dot(l, p) * p;
}

//Compute the angle point between lines l1 and l2
//You may assume the lines are not parallel
//The results is a scalar
float angle(Line2D l1, Line2D l2) {
  std::cout << dot(l1.normalized(), l2.normalized()) << std::endl;

  return std::atan2(l2.y - l1.y, l2.x - l1.x);
}

//Compute if the line segment p1->p2 intersects the line segment a->b
//The result is a boolean
bool segmentSegmentIntersect(Point2D p1, Point2D p2, Point2D a, Point2D b) {
  // Find the line segment running through the points
  Line2D l1 = join(p1, p2);
  Line2D l2 = join(a, b);

  Point2D intersection = intersect(l1, l2);

  // Check if this point is contained within both line segments.
  // It's already aligned, so just ensure that the distance between
  // the endpoints is no greater than the length of the line segment.

  // i.e., make sure the point exists on both segments

  // Uses squared magnitude to speed up calculation by avoiding square roots
  float l1Mag = (p2 - p1).magnitudeSqr();
  float l2Mag = (b - a).magnitudeSqr();

  float p1ADist = (intersection - p1).magnitudeSqr();
  float p1BDist = (p2 - intersection).magnitudeSqr();

  float p2ADist = (intersection - a).magnitudeSqr();
  float p2BDist = (b - intersection).magnitudeSqr();

  return (p1ADist + p1BDist) <= l1Mag && (p2ADist + p2BDist) <= l2Mag;
}

//Compute if the point p lies inside the triangle t1,t2,t3
//Your code should work for both clockwise and counterclockwise windings
//The result is a bool
bool pointInTriangle(Point2D p, Point2D t1, Point2D t2, Point2D t3) {
  Line2D l1 = vee(t1, t2);
  Line2D l2 = vee(t2, t3);
  Line2D l3 = vee(t3, t1);

  float f1 = vee(p, l1);
  float f2 = vee(p, l2);
  float f3 = vee(p, l3);

  return (f1 > 0 == f2 > 0) && (f1 > 0 == f3 > 0);
}

bool pointInQuad(Point2D p, Point2D t1, Point2D t2, Point2D t3, Point2D t4) {
  Line2D l1 = vee(t1, t2);
  Line2D l2 = vee(t2, t3);
  Line2D l3 = vee(t3, t4);
  Line2D l4 = vee(t4, t1);

  float f1 = vee(p, l1);
  float f2 = vee(p, l2);
  float f3 = vee(p, l3);
  float f4 = vee(p, l4);

  return (f1 > 0 == f2 > 0) && (f1 > 0 == f3 > 0) && (f1 > 0 == f4 > 0);
}

//Compute the area of the triangle t1,t2,t3
//The result is a scalar
float areaTriangle(Point2D t1, Point2D t2, Point2D t3) {
  return 0.5f * vee(t1, t2, t3);
}

//Compute the distance from the point p to the triangle t1,t2,t3 as defined 
//by it's distance from the edge closest to p.
//The result is a scalar
float pointTriangleEdgeDist(Point2D p, Point2D t1, Point2D t2, Point2D t3) {
  Line2D l1 = vee(t1, t2);
  Line2D l2 = vee(t2, t3);
  Line2D l3 = vee(t3, t1);

  float f1 = vee(p, l1.normalized());
  float f2 = vee(p, l2.normalized());
  float f3 = vee(p, l3.normalized());

  return std::min(std::abs(f1), std::min(std::abs(f2), std::abs(f3)));
}

//Compute the distance from the point p to the closest of three corners of
// the triangle t1,t2,t3
//The result is a scalar
float pointTriangleCornerDist(Point2D p, Point2D t1, Point2D t2, Point2D t3) {
  float d1 = vee(p, t1).magnitude();
  float d2 = vee(p, t2).magnitude();
  float d3 = vee(p, t3).magnitude();

  return std::min(d1, std::min(d2, d3));
}

//Compute if the quad (p1,p2,p3,p4) is convex.
//Your code should work for both clockwise and counterclockwise windings
//The result is a boolean
bool isConvex_Quad(Point2D p1, Point2D p2, Point2D p3, Point2D p4) {
  Line2D l1 = vee(p1, p2);
  Line2D l2 = vee(p2, p3);
  Line2D l3 = vee(p3, p4);
  Line2D l4 = vee(p4, p1);

  return (vee(p3, l1) > 0 && vee(p4, l2) > 0 && vee(p1, l3) > 0)
    || (vee(p3, l1) < 0 && vee(p4, l2) < 0 && vee(p1, l3) < 0);
}

//Compute the reflection of the point p about the line l
//The result is a point
Point2D reflect(Point2D p, Line2D l) {
  return l * p * l;
}

//Compute the reflection of the line d about the line l
//The result is a line
Line2D reflect(Line2D d, Line2D l) {
  return l * d * l;
}

#endif