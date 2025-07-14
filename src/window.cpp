#include <glad/gl.h>
#include "window.h"
#include "renderer.h"
#include "game.h"
#include <iostream>

bool initGLFW() 
{
	if (!glfwInit()) 
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return false;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	return true;
}

GLFWwindow* createWindow(int width, int height, const char* title)
{
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	else {
		glfwMakeContextCurrent(window);
		// マウスコールバックを設定
		glfwSetMouseButtonCallback(window, mouseCallback);
	}
	return window;
}

bool initGLAD()
{
	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) 
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	return true;
}

void cleanup(GLFWwindow* window) 
{
	if (window)
		glfwDestroyWindow(window);
	glfwTerminate();
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window)) 
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		
		// R キーでゲームリセット
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
			resetGame();

		// AI更新
		updateAI();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// ゲームを描画
		renderGame();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
