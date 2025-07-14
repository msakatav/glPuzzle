#include <glad/gl.h>
#include "window.h"
#include "renderer.h"
#include "game.h"

int main()
{
	if (!initGLFW())
		return -1;

	GLFWwindow* window = createWindow(800, 600, "Puzzle Game");
	if (!window)
		return -1;

	if (!initGLAD())
	{
		cleanup(window);
		return -1;
	}

	glViewport(0, 0, 800, 600);

	// ゲームを初期化
	initGame();
	
	// レンダラーを初期化
	setupShaders();
	setupGameRenderer();
	setupTextures();

	mainLoop(window);

	// 解放
	cleanupGameRenderer();
	cleanupShaders();
	cleanupTextures();
	cleanup(window);
	return 0;
}
