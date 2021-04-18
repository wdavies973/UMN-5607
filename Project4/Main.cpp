#include "glad/glad.h"  //Include order can matter here

#include <SDL.h>
#include <SDL_opengl.h>
#include <cstdio>
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

//For Visual Studios
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

bool fullscreen = false;
int screenWidth = 800;
int screenHeight = 600;

const GLchar* vertexSource =
"#version 150 core\n"
"in vec3 position;"
"in vec3 inColor;"
"out vec3 Color;"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"
"void main() {"
"   Color = inColor;"
"   gl_Position = proj * view * model * vec4(position, 1.0);"
"}";

const GLchar* fragmentSource =
"#version 150 core\n"
"in vec3 Color;"
"out vec4 outColor;"
"void main() {"
"   outColor = vec4(Color, 1.0);"
"}";

// The player's position
glm::vec3 playerPosition(3.f, 0.f, 0.f);
glm::vec3 playerFacing(0.f, 0.f, 0.f);
glm::vec3 playerUp(0.f, 0.f, 1.f);
glm::vec3 playerVerticalVelocity(0.f, 0.f, 0.f);

bool movingLeft = false, movingRight = false, movingForward = false, movingBack = false;

void loadShader(GLuint shaderId, const GLchar* shaderSource) {
  glShaderSource(shaderId, 1, &shaderSource, NULL);
  glCompileShader(shaderId);

  GLint status;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);

  if(!status) {
    char buffer[512];
    glGetShaderInfoLog(shaderId, 512, NULL, buffer);
    cout << "Shader Compile Failed. Info:" << endl << endl << buffer << endl;
  }
}

char** loadMap(string path, int* width, int* height) {
  ifstream file;
  file.open(path);

  if(!file) {
    cout << "Failed to load map" << endl;
    exit(-1);
  }

  file >> *width; file >> *height;

  char** map = new char* [*height];

  for(int row = 0; row < *height; row++) {
    map[row] = new char[*width];

    for(int col = 0; col < *width; col++) {
      file >> map[row][col];
    }
  }

  return map;
}

void loadCube(GLuint shaderProgram) {
  // Load data to be rendered
  GLfloat vertices[] = { -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,-0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,-0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };

  GLuint vbo;
  // Create a new vertex buffer object
  glGenBuffers(1, &vbo);
  // Bind the buffer (make it active)
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // Send vertices to the buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

  // Now, create VAO which maps the vertices to the shader program
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
  glEnableVertexAttribArray(posAttrib);

  GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(colAttrib);
}


int main(int argc, char* argv[]) {
  // Setup the window using SDL2

  int mapWidth, mapHeight;
  char** map = loadMap("map.txt", &mapWidth, &mapHeight);

  cout << "Map size: " << mapWidth << "x" << mapHeight << endl;

  for(int row = 0; row < mapHeight; row++) {
    for(int col = 0; col < mapWidth; col++) {
      cout << map[row][col];
    }
    cout << endl;
  }

  SDL_Init(SDL_INIT_VIDEO);
  SDL_version comp;
  SDL_version linked;
  SDL_VERSION(&comp);
  SDL_GetVersion(&linked);
  cout << "\nCompiled against SDL version " << comp.major << "." << comp.minor << "." << comp.patch << endl;
  cout << "Linked SDL version " << linked.major << "." << linked.minor << "." << linked.patch << endl;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  SDL_Window* window = SDL_CreateWindow("Project4", 1500, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
  if(!window) {
    cerr << "Could not create window: " << SDL_GetError() << endl;
    return EXIT_FAILURE;
  }
  float aspect = screenWidth / (float)screenHeight;

  SDL_GLContext context = SDL_GL_CreateContext(window);
  SDL_SetRelativeMouseMode(SDL_TRUE);
  

  // Ensure OpenGL is active
  if(gladLoadGLLoader(SDL_GL_GetProcAddress)) {
    printf("\nOpenGL loaded\n");
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n\n", glGetString(GL_VERSION));
  } else {
    printf("ERROR: Failed to initialize OpenGL context.\n");
    return -1;
  }

  // Load and compile vertex & fragment shaders
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  loadShader(vertexShader, vertexSource);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  loadShader(fragmentShader, fragmentSource);

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);

  glBindVertexArray(0);

  // Game loop
  glEnable(GL_DEPTH_TEST);

  SDL_Event windowEvent;
  bool quit = false;

  Uint32 lastTick = 0, delta = 0;

  while(!quit) {

    int mouseXDelta = 0;
    int mouseYDelta = 0;

    while(SDL_PollEvent(&windowEvent)) {
      if(windowEvent.type == SDL_QUIT) quit = true; //Exit event loop
      //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
      //Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
      if(windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
        quit = true; //Exit event loop
      if(windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) //If "f" is pressed
        fullscreen = !fullscreen;
      SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Set to full screen 

      if(windowEvent.type == SDL_KEYDOWN) {
        switch(windowEvent.key.keysym.sym) {
        case SDLK_w:
          movingForward = true;
          break;
        case SDLK_s:
          movingBack = true;
          break;
        case SDLK_a:
          movingLeft = true;
          break;
        case SDLK_d:
          movingRight = true;
          break;
        case SDLK_SPACE:
          playerVerticalVelocity.z = 0.03;
          break;
        }
      } else if(windowEvent.type == SDL_KEYUP) {
        switch(windowEvent.key.keysym.sym) {
        case SDLK_w:
          movingForward = false;
          break;
        case SDLK_s:
          movingBack = false;
          break;
        case SDLK_a:
          movingLeft = false;
          break;
        case SDLK_d:
          movingRight = false;
          break;

        }
      }

      if(windowEvent.type == SDL_MOUSEMOTION) {
        mouseXDelta = windowEvent.motion.xrel;
        mouseYDelta = windowEvent.motion.yrel;

        /*cout << "X: " << x << endl;
        cout << "Y: " << y << endl;*/
      }
    }
    
    // Based off inputs, make adjustments to the player position, facing direction, and up vector.

    // 1) For horizontal look, rotate the facing vector around the up vector. Position stays indentical.
    
    glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float time = SDL_GetTicks() / 1000.f;

    //yaw += mouseXDelta * delta;
    if(mouseXDelta != 0) {
      glm::mat4 rotateYaw = glm::rotate(glm::mat4(1.0f), mouseXDelta * (delta / 1000.f), playerUp);
      playerFacing = glm::vec3(glm::vec4(playerFacing - playerPosition, 1.0f) * rotateYaw);
    }

    if(mouseYDelta != 0) {
      glm::vec3 right = glm::cross(playerFacing - playerPosition, playerUp);
      glm::mat4 rotatePitch = glm::rotate(glm::mat4(1.0f), mouseYDelta * (delta / 1000.f), right);
      playerFacing = glm::vec3(glm::vec4(playerFacing - playerPosition, 1.0f) * rotatePitch);
      //playerUp = glm::cross(right, playerFacing - playerPosition);
    }
    
    // Compute vector for movement
    float at = 0;
    float strafe = 0;
    if(movingForward) at += (delta / 1000.f);
    if(movingBack) at -= (delta / 1000.f);
    if(movingLeft) strafe -= (delta / 1000.f);
    if(movingRight) strafe += (delta / 1000.f);

    glm::vec3 unit = glm::normalize(playerFacing - playerPosition);
    unit.z = 0;
    playerPosition += unit * at;
    /*playerPosition = glm::vec3(playerFacing);
    playerPosition.x -= 3.f;*/
    glm::vec3 right = glm::normalize(glm::cross(playerFacing - playerPosition, playerUp));
    right.z = 0;
    playerPosition += right * strafe;

    playerVerticalVelocity.z -= 0.1 * (delta / 1000.f);

    playerPosition.z += playerVerticalVelocity.z;

    if(playerPosition.z < 0) playerPosition.z = 0;

    int playerRow = (int)(playerPosition.x + 0.5);
    int playerCol = (int)(playerPosition.y + 0.5);

    if(playerRow < 1 || map[playerRow - 1][playerCol] == 'W') {
      if(playerPosition.x <= playerRow + 0.05) {
        playerPosition.x = playerRow + 0.05;
      }
    } 
    
    if(playerRow >= mapHeight - 1 || map[playerRow + 1][playerCol] == 'W') {
      if(playerPosition.x >= (playerRow + 1 - 0.95)) {
        playerPosition.x = playerRow + 1 - 0.95;
      }
    }

    if(playerCol < 1 || map[playerRow][playerCol - 1] == 'W') {
      if(playerPosition.y <= playerCol + 0.05) {
        playerPosition.y = playerCol + 0.05;
      }
    }

    if(playerCol >= mapWidth - 1 || map[playerRow][playerCol + 1] == 'W') {
      if(playerPosition.y >= (playerCol + 1 - 0.95)) {
        playerPosition.y = playerCol + 1 - 0.95;
      }
    }

    
    
    //glm::mat4 model = glm::mat4(1);
    //model = glm::rotate(model, time * 3.14f / 2, glm::vec3(0.0f, 1.0f, 1.0f));
    //model = glm::rotate(model, time * 3.14f / 4, glm::vec3(1.0f, 0.0f, 0.0f));
    GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    //glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

    glm::mat4 view = glm::lookAt(
      playerPosition,  //Cam Position
      playerFacing,  //Look at point
      playerUp); //Up

    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(3.14f / 4, aspect, 0.01f, 10.0f); //FOV, aspect, near, far

    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    /*glDrawArrays(GL_TRIANGLES, 0, 36);*/

    // Draw all perimeter walls
    for(int col = 0; col < mapWidth; col++) {
      glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(-1, col, 0.f));
      glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);

      model = glm::translate(glm::mat4(1), glm::vec3(mapHeight, col, 0.f));
      glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    
    for(int row = 0; row < mapHeight; row++) {
      glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(row, -1, 0.f));
      glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);

      model = glm::translate(glm::mat4(1), glm::vec3(row, mapWidth, 0.f));
      glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Draw walls, each wall can be drawn as a block.
    // Each block is 1x1.
    for(int row = 0; row < mapHeight; row++) {
      for(int col = 0; col < mapWidth; col++) {
        if(map[row][col] == 'W') {
          glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(row, col, 0.f));
          glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
          glDrawArrays(GL_TRIANGLES, 0, 36);

          //cout << "Box is located at" << row << "," << col << endl;
        }

        // Floor
        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(row, col, -1.f));
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
      }
    }
    
    SDL_GL_SwapWindow(window); //Double buffering

    Uint32 ticks = SDL_GetTicks();
    delta = ticks - lastTick;
    lastTick = ticks;
  }

  glDeleteProgram(shaderProgram);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  delete map;
}