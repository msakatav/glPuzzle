#include <glad/gl.h>
#include "window.h"
#include "renderer.h"

int main()
{
	if (!initGLFW())
		return -1;

	GLFWwindow* window = createWindow(800, 600, "OpenGL Triangle");
	if (!window)
		return -1;

	if (!initGLAD())
	{
		cleanup(window);
		return -1;
	}

	glViewport(0, 0, 800, 600);

	setupShaders();
	setupTriangle();

	mainLoop(window);

	// 解放
	cleanupTriangle();
	cleanupShaders();
	cleanup(window);
	return 0;
}
