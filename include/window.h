#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// ウィンドウ関連
bool initGLFW();
GLFWwindow* createWindow(int width, int height, const char* title);
bool initGLAD();
void cleanup(GLFWwindow* window);

// メインループ
void mainLoop(GLFWwindow* window);

#endif // WINDOW_H
