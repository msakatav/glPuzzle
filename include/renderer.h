#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h>
#include "game.h"
#include "renderer.h"
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

// ゲームボード描画関連
void setupGameRenderer();
void renderGame();
void cleanupGameRenderer();

// シェーダー関連
unsigned int compileShader(unsigned int type, const char* source);
void setupShaders();
void cleanupShaders();

// テクスチャ関連
void setupTextures();
void cleanupTextures();
unsigned int loadTexture(const char* path);
void renderTexture(unsigned int texture, float x1, float y1, float x2, float y2);

// テキスト描画関連
void setupTextRenderer();
void cleanupTextRenderer();
void renderChar(char c, float x, float y, float scale, float color[3]);
void renderText(const char* text, float x, float y, float scale, float color[3]);
void renderScore(int redScore, int blueScore);
void renderGameOverScreen(Player winner);

// グローバル変数の宣言
extern unsigned int shaderProgram;
extern unsigned int textureShaderProgram;
extern const char* vertexShaderSource;
extern const char* fragmentShaderSource;
extern const char* textureVertexShaderSource;
extern const char* textureFragmentShaderSource;
extern unsigned int gameVAO, gameVBO;
extern unsigned int textureVAO, textureVBO;
extern unsigned int plusOneTexture, minusOneTexture, plusTwoTexture;

// テキスト描画用の変数
extern unsigned int textVAO, textVBO;
extern unsigned int digitTextures[10];  // 0-9の数字テクスチャ


#endif // RENDERER_H
