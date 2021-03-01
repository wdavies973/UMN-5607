
//Set the global scene parameter variables
//TODO: Set the scene parameters based on the values in the scene file

#ifndef PARSE_VEC3_H
#define PARSE_VEC3_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

//Camera & Scene Parmaters (Global Variables)
//Here we set default values, override them in parseSceneFile()

//Image Parmaters
int img_width = 800, img_height = 600;
std::string imgName = "raytraced.png";

//Camera Parmaters
vec3 eye = vec3(0, 0, 0);
vec3 forward = vec3(0, 0, -1).normalized();
vec3 up = vec3(0, 1, 0).normalized();
vec3 right = vec3(-1, 0, 0).normalized();
float halfAngleVFOV = 35;

//Scene (Sphere) Parmaters
vec3 spherePos = vec3(0, 0, 2);
float sphereRadius = 1;

void parseSceneFile(std::string fileName)
{
  std::ifstream file;
  file.open(fileName);

  if (!file)
  {
    std::cout << "ERROR: Image file " << fileName << " not found." << std::endl;
    exit(1);
  }

  std::string cmd;

  while (file >> cmd)
  {
    // Check if the line is a comment
    if (cmd == "#")
    {
      // Flush the rest of the line out
      std::getline(file, cmd);
      continue;
    }
    else if (cmd == "sphere:")
    {
      file >> spherePos.x;
      file >> spherePos.y;
      file >> spherePos.z;
      file >> sphereRadius;
      std::cout << "Sphere (x,y,z,r): (" << spherePos.x << "," << spherePos.y << "," 
        << spherePos.z << "," << sphereRadius << ")" <<std::endl;
    }
    else if (cmd == "image_resolution:")
    {
      file >> img_width;
      file >> img_height;
      std::cout << "Resolution: " << img_width << " x " << img_height << std::endl;
    }
    else if (cmd == "output_image:")
    {
      file >> imgName;
      std::cout << "Output: " << imgName << std::endl;
    }
    else if (cmd == "camera_pos:")
    {
      file >> eye.x;
      file >> eye.y;
      file >> eye.z;

      std::cout << "Camera pos: (" << eye.x << "," << eye.y << "," << eye.z << ")" << std::endl;
    }
    else if (cmd == "camera_fwd:")
    {
      file >> forward.x;
      file >> forward.y;
      file >> forward.z;

      std::cout << "Camera fwd: (" << forward.x << "," << forward.y << "," << forward.z << ")" << std::endl;
    }
    else if (cmd == "camera_up:")
    {
      file >> up.x;
      file >> up.y;
      file >> up.z;

      std::cout << "Camera up: (" << up.x << "," << up.y << "," << up.z << ")" << std::endl;
    }
    else if (cmd == "camera_fov_ha:")
    {
      file >> halfAngleVFOV;

      std::cout << "Half angle: " << halfAngleVFOV << std::endl;
    }
  }

  // 1) Infer right based on up and forward
  right = cross(up, forward);

  // 2) Normalize up, right, forward
  right = right.normalized();
  forward = forward.normalized();
  up = up.normalized();

  // 3) Ensure orthogonal basis (assumes non-coplanar)
  if(dot(forward, up) != 0 || dot(forward, right) != 0 || dot(up, right) != 0) {
    std::cout << "Vectors not orthogonal. Orthogonalizing..." << std::endl;

    // 3a) Make forward orthogonal to right
    forward = forward - (dot(forward, right) / dot(right, right)) * right;
    // 3b) Make up orthogonal to right & forward
    up = up - (dot(up, right) / dot(right, right)) * right - (dot(up, forward) / dot(forward, forward)) * forward;
  }

  printf("Orthagonal Camera Basis:\n");
  printf("forward: %f,%f,%f\n", forward.x, forward.y, forward.z);
  printf("right: %f,%f,%f\n", right.x, right.y, right.z);
  printf("up: %f,%f,%f\n", up.x, up.y, up.z);
}

#endif