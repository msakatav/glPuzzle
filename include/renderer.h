#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h>

// 三角形レンダラー関連
void setupTriangle();
void renderTriangle();
void cleanupTriangle();

// シェーダー関連
unsigned int compileShader(unsigned int type, const char* source);
void setupShaders();
void cleanupShaders();

// グローバル変数の宣言
extern unsigned int VAO, VBO, shaderProgram;
extern const char* vertexShaderSource;
extern const char* fragmentShaderSource;

#endif // RENDERER_H
