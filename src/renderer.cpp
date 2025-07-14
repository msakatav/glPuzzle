#include "renderer.h"
#include <iostream>
#include <stdio.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// 頂点シェーダのソース
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	vertexColor = aColor;
}
)";

// フラグメントシェーダのソース
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main()
{
	FragColor = vec4(vertexColor, 1.0);
}
)";

// テクスチャ用頂点シェーダのソース
const char* textureVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	TexCoord = aTexCoord;
}
)";

// テクスチャ用フラグメントシェーダのソース
const char* textureFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D ourTexture;

void main()
{
	FragColor = texture(ourTexture, TexCoord);
	if(FragColor.a < 0.1)
		discard;
}
)";

unsigned int VAO, VBO, shaderProgram;
unsigned int gameVAO, gameVBO;
unsigned int textureShaderProgram;
unsigned int textureVAO, textureVBO;
unsigned int plusOneTexture, minusOneTexture, plusTwoTexture;

// テキスト描画用の変数
unsigned int textVAO, textVBO;
unsigned int digitTextures[10];  // 0-9の数字テクスチャ

void setupGameRenderer()
{
	glGenVertexArrays(1, &gameVAO);
	glGenBuffers(1, &gameVBO);
	
	// テクスチャ用VAO/VBOの設定
	glGenVertexArrays(1, &textureVAO);
	glGenBuffers(1, &textureVBO);
}

unsigned int loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);  // OpenGL用に上下反転
	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		stbi_image_free(data);
		printf("テクスチャ読み込み成功: %s (%dx%d, %d channels)\n", path, width, height, nrChannels);
	}
	else
	{
		printf("テクスチャ読み込み失敗: %s\n", path);
		printf("エラー詳細: %s\n", stbi_failure_reason());
		printf("ファイルの確認を行ってください。\n");
		// テクスチャIDを0にして失敗を示す
		glDeleteTextures(1, &textureID);
		return 0;
	}
	
	return textureID;
}

void renderTexture(unsigned int texture, float x1, float y1, float x2, float y2)
{
	if (texture == 0) return;  // テクスチャが無効な場合は何もしない
	
	// 現在のシェーダーを保存
	GLint currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
	
	// テクスチャ用頂点データ
	float vertices[] = {
		// 位置           // テクスチャ座標
		x1, y1, 0.0f,    0.0f, 1.0f,  // 左下
		x2, y1, 0.0f,    1.0f, 1.0f,  // 右下
		x2, y2, 0.0f,    1.0f, 0.0f,  // 右上
		x1, y1, 0.0f,    0.0f, 1.0f,  // 左下
		x2, y2, 0.0f,    1.0f, 0.0f,  // 右上
		x1, y2, 0.0f,    0.0f, 0.0f   // 左上
	};
	
	// テクスチャシェーダとVAOを使用
	glUseProgram(textureShaderProgram);
	glBindVertexArray(textureVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	
	// 頂点属性の設定
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	// テクスチャをバインド
	glBindTexture(GL_TEXTURE_2D, texture);
	
	// アルファブレンディングを有効化
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// 描画
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	// 状態を戻す
	glDisable(GL_BLEND);
	glUseProgram(currentProgram);
}

void setupTextRenderer()
{
	// テキスト描画用のVAO/VBOを設定
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
}

void cleanupTextRenderer()
{
	glDeleteVertexArrays(1, &textVAO);
	glDeleteBuffers(1, &textVBO);
}

void renderDigit(int digit, float x, float y, float size, float color[3])
{
	if (digit < 0 || digit > 9) return;
	
	// 簡単な7セグメント風の数字描画
	// 各数字のセグメントパターンを定義
	bool segments[10][7] = {
		{1,1,1,1,1,1,0}, // 0
		{0,1,1,0,0,0,0}, // 1  
		{1,1,0,1,1,0,1}, // 2
		{1,1,1,1,0,0,1}, // 3
		{0,1,1,0,0,1,1}, // 4
		{1,0,1,1,0,1,1}, // 5
		{1,0,1,1,1,1,1}, // 6
		{1,1,1,0,0,0,0}, // 7
		{1,1,1,1,1,1,1}, // 8
		{1,1,1,1,0,1,1}  // 9
	};
	
	float segWidth = size * 0.6f;    // セグメント幅を小さく
	float segHeight = size * 0.08f;  // セグメント高さを調整
	float vertSegHeight = size * 0.35f;
	float vertSegWidth = size * 0.08f;
	
	glUseProgram(shaderProgram);
	glBindVertexArray(gameVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gameVBO);
	
	// 7つのセグメントを描画
	float segPositions[7][4] = {
		{x, y + size, x + segWidth, y + size - segHeight},           // 上
		{x + segWidth, y + size, x + segWidth - vertSegWidth, y + size/2}, // 右上
		{x + segWidth, y + size/2, x + segWidth - vertSegWidth, y},         // 右下
		{x, y, x + segWidth, y + segHeight},                         // 下
		{x, y + size/2, x + vertSegWidth, y},                       // 左下
		{x, y + size, x + vertSegWidth, y + size/2},                // 左上
		{x, y + size/2, x + segWidth, y + size/2 - segHeight}       // 中央
	};
	
	for (int i = 0; i < 7; i++) {
		if (segments[digit][i]) {
			float x1 = segPositions[i][0];
			float y1 = segPositions[i][1];
			float x2 = segPositions[i][2];
			float y2 = segPositions[i][3];
			
			float vertices[] = {
				x1, y1, 0.0f,    color[0], color[1], color[2],
				x2, y1, 0.0f,    color[0], color[1], color[2],
				x2, y2, 0.0f,    color[0], color[1], color[2],
				x1, y1, 0.0f,    color[0], color[1], color[2],
				x2, y2, 0.0f,    color[0], color[1], color[2],
				x1, y2, 0.0f,    color[0], color[1], color[2]
			};
			
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
}

void renderText(const char* text, float x, float y, float scale, float color[3])
{
	float currentX = x;
	for (int i = 0; text[i] != '\0'; i++) {
		if (text[i] >= '0' && text[i] <= '9') {
			renderDigit(text[i] - '0', currentX, y, scale, color);
			currentX += scale * 1.2f; // 数字間のスペースを広げる
		} else if (text[i] == '-') {
			// マイナス記号を描画
			float minusY = y + scale * 0.5f;
			float minusHeight = scale * 0.08f;
			float minusWidth = scale * 0.6f;
			
			float vertices[] = {
				currentX, minusY, 0.0f, color[0], color[1], color[2],
				currentX + minusWidth, minusY, 0.0f, color[0], color[1], color[2],
				currentX + minusWidth, minusY - minusHeight, 0.0f, color[0], color[1], color[2],
				currentX, minusY, 0.0f, color[0], color[1], color[2],
				currentX + minusWidth, minusY - minusHeight, 0.0f, color[0], color[1], color[2],
				currentX, minusY - minusHeight, 0.0f, color[0], color[1], color[2]
			};
			
			glUseProgram(shaderProgram);
			glBindVertexArray(gameVAO);
			glBindBuffer(GL_ARRAY_BUFFER, gameVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			
			currentX += scale * 0.8f; // マイナス記号の幅
		} else if (text[i] == '+') {
			// プラス記号を描画
			renderChar(text[i], currentX, y, scale, color);
			currentX += scale * 1.0f; // プラス記号の幅
		} else if (text[i] == ' ') {
			currentX += scale * 0.8f; // スペースの幅
		} else {
			// アルファベット文字をビットマップで描画
			renderChar(text[i], currentX, y, scale, color);
			currentX += scale * 1.0f; // 文字間のスペース
		}
	}
}

void renderChar(char c, float x, float y, float scale, float color[3])
{
	// 各文字を簡単なビットマップパターンで描画
	// 8x8ピクセルのパターンを使用
	
	// 文字パターンのデータ（8x8、1=描画、0=なし）
	bool patterns[256][8][8];
	memset(patterns, 0, sizeof(patterns));
	
	// 文字 'R'
	bool R[8][8] = {
		{1,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,1,1,1,1,1,0,0},
		{1,0,0,1,0,0,0,0},
		{1,0,0,0,1,0,0,0},
		{1,0,0,0,0,1,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'E'
	bool E[8][8] = {
		{1,1,1,1,1,1,1,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,1,1,1,1,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,1,1,1,1,1,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'D'
	bool D[8][8] = {
		{1,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,1,1,1,1,1,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'B'
	bool B[8][8] = {
		{1,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,1,1,1,1,1,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'L'
	bool L[8][8] = {
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,1,1,1,1,1,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'U'
	bool U[8][8] = {
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{0,1,1,1,1,1,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'W'
	bool W[8][8] = {
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,1,0,0,1,0},
		{1,0,1,0,1,0,1,0},
		{1,1,0,0,0,1,1,0},
		{1,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'I'
	bool I[8][8] = {
		{0,1,1,1,1,1,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,1,1,1,1,1,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'N'
	bool N[8][8] = {
		{1,0,0,0,0,0,1,0},
		{1,1,0,0,0,0,1,0},
		{1,0,1,0,0,0,1,0},
		{1,0,0,1,0,0,1,0},
		{1,0,0,0,1,0,1,0},
		{1,0,0,0,0,1,1,0},
		{1,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'S'
	bool S[8][8] = {
		{0,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{0,1,1,1,1,0,0,0},
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,1,0},
		{0,1,1,1,1,1,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'T'
	bool T[8][8] = {
		{1,1,1,1,1,1,1,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'P'
	bool P[8][8] = {
		{1,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 '+'
	bool Plus[8][8] = {
		{0,0,0,0,0,0,0,0},
		{0,0,0,1,1,0,0,0},
		{0,0,0,1,1,0,0,0},
		{0,1,1,1,1,1,1,0},
		{0,0,0,1,1,0,0,0},
		{0,0,0,1,1,0,0,0},
		{0,0,0,1,1,0,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'O'
	bool O[8][8] = {
		{0,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{0,1,1,1,1,1,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'A'
	bool A[8][8] = {
		{0,0,1,1,1,0,0,0},
		{0,1,0,0,0,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,1,1,1,1,1,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'C'
	bool C[8][8] = {
		{0,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,1,0},
		{0,1,1,1,1,1,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'G'
	bool G[8][8] = {
		{0,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,1,1,1,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{0,1,1,1,1,1,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'M'
	bool M[8][8] = {
		{1,0,0,0,0,0,1,0},
		{1,1,0,0,0,1,1,0},
		{1,0,1,0,1,0,1,0},
		{1,0,0,1,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'F'
	bool F[8][8] = {
		{1,1,1,1,1,1,1,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,1,1,1,1,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'H'
	bool H[8][8] = {
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,1,1,1,1,1,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'V'
	bool V[8][8] = {
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{0,1,0,0,0,1,0,0},
		{0,0,1,0,1,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'Y'
	bool Y[8][8] = {
		{1,0,0,0,0,0,1,0},
		{0,1,0,0,0,1,0,0},
		{0,0,1,0,1,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'K'
	bool K[8][8] = {
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,1,0,0},
		{1,0,0,0,1,0,0,0},
		{1,0,0,1,0,0,0,0},
		{1,0,1,0,0,0,0,0},
		{1,1,0,0,0,0,0,0},
		{1,0,1,0,0,0,0,0},
		{1,0,0,1,0,0,0,0}
	};
	
	// 文字 'J'
	bool J[8][8] = {
		{1,1,1,1,1,1,1,0},
		{0,0,0,0,1,0,0,0},
		{0,0,0,0,1,0,0,0},
		{0,0,0,0,1,0,0,0},
		{0,0,0,0,1,0,0,0},
		{1,0,0,0,1,0,0,0},
		{0,1,1,1,0,0,0,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'Q'
	bool Q[8][8] = {
		{0,1,1,1,1,1,0,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,0,0,0,1,0},
		{1,0,0,1,0,0,1,0},
		{1,0,0,0,1,0,1,0},
		{0,1,1,1,1,1,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'X'
	bool X[8][8] = {
		{1,0,0,0,0,0,1,0},
		{0,1,0,0,0,1,0,0},
		{0,0,1,0,1,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,1,0,1,0,0,0},
		{0,1,0,0,0,1,0,0},
		{1,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// 文字 'Z'
	bool Z[8][8] = {
		{1,1,1,1,1,1,1,0},
		{0,0,0,0,0,1,0,0},
		{0,0,0,0,1,0,0,0},
		{0,0,0,1,0,0,0,0},
		{0,0,1,0,0,0,0,0},
		{0,1,0,0,0,0,0,0},
		{1,1,1,1,1,1,1,0},
		{0,0,0,0,0,0,0,0}
	};
	
	// パターンを配列にコピー
	memcpy(patterns['R'], R, sizeof(R));
	memcpy(patterns['E'], E, sizeof(E));
	memcpy(patterns['D'], D, sizeof(D));
	memcpy(patterns['B'], B, sizeof(B));
	memcpy(patterns['L'], L, sizeof(L));
	memcpy(patterns['U'], U, sizeof(U));
	memcpy(patterns['W'], W, sizeof(W));
	memcpy(patterns['I'], I, sizeof(I));
	memcpy(patterns['N'], N, sizeof(N));
	memcpy(patterns['S'], S, sizeof(S));
	memcpy(patterns['T'], T, sizeof(T));
	memcpy(patterns['P'], P, sizeof(P));
	memcpy(patterns['+'], Plus, sizeof(Plus));
	memcpy(patterns['O'], O, sizeof(O));
	memcpy(patterns['A'], A, sizeof(A));
	memcpy(patterns['C'], C, sizeof(C));
	memcpy(patterns['G'], G, sizeof(G));
	memcpy(patterns['M'], M, sizeof(M));
	memcpy(patterns['F'], F, sizeof(F));
	memcpy(patterns['H'], H, sizeof(H));
	memcpy(patterns['V'], V, sizeof(V));
	memcpy(patterns['Y'], Y, sizeof(Y));
	memcpy(patterns['K'], K, sizeof(K));
	memcpy(patterns['J'], J, sizeof(J));
	memcpy(patterns['Q'], Q, sizeof(Q));
	memcpy(patterns['X'], X, sizeof(X));
	memcpy(patterns['Z'], Z, sizeof(Z));
	
	// スペースは何も描画しない
	if (c == ' ') return;
	
	// 該当する文字のパターンを取得
	if (c < 0 || c >= 256) return;
	
	float pixelSize = scale / 8.0f;
	
	glUseProgram(shaderProgram);
	glBindVertexArray(gameVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gameVBO);
	
	// 8x8のピクセルを描画
	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			if (patterns[c][row][col]) {
				float px = x + col * pixelSize;
				float py = y + (7 - row) * pixelSize; // Y軸を反転
				
				float vertices[] = {
					px, py, 0.0f, color[0], color[1], color[2],
					px + pixelSize, py, 0.0f, color[0], color[1], color[2],
					px + pixelSize, py + pixelSize, 0.0f, color[0], color[1], color[2],
					px, py, 0.0f, color[0], color[1], color[2],
					px + pixelSize, py + pixelSize, 0.0f, color[0], color[1], color[2],
					px, py + pixelSize, 0.0f, color[0], color[1], color[2]
				};
				
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
	}
}

void renderScore(int redScore, int blueScore)
{
	// スコアを文字列に変換（マイナスにも対応）
	char redScoreStr[20], blueScoreStr[20];
	sprintf(redScoreStr, "%d", redScore);
	sprintf(blueScoreStr, "%d", blueScore);
	
	// 色の定義
	float redColor[3] = {1.0f, 0.3f, 0.3f};
	float blueColor[3] = {0.3f, 0.3f, 1.0f};
	
	// スコア表示位置とサイズ
	float scoreSize = 0.15f;
	float leftX = -0.9f;
	float rightX = 0.6f;
	float scoreY = 0.8f;
	
	// 赤スコア表示
	renderText(redScoreStr, leftX, scoreY, scoreSize, redColor);
	
	// 青スコア表示  
	renderText(blueScoreStr, rightX, scoreY, scoreSize, blueColor);
}

void renderGameOverScreen(Player winner)
{
	// 暗転オーバーレイを描画
	float overlayColor[3] = {0.0f, 0.0f, 0.0f}; // 黒
	float overlayAlpha = 0.8f; // 透明度
	
	float overlayVertices[] = {
		// 画面全体を覆う四角形
		-1.0f, -1.0f, 0.0f,    overlayColor[0], overlayColor[1], overlayColor[2],
		 1.0f, -1.0f, 0.0f,    overlayColor[0], overlayColor[1], overlayColor[2],
		 1.0f,  1.0f, 0.0f,    overlayColor[0], overlayColor[1], overlayColor[2],
		-1.0f, -1.0f, 0.0f,    overlayColor[0], overlayColor[1], overlayColor[2],
		 1.0f,  1.0f, 0.0f,    overlayColor[0], overlayColor[1], overlayColor[2],
		-1.0f,  1.0f, 0.0f,    overlayColor[0], overlayColor[1], overlayColor[2]
	};
	
	glUseProgram(shaderProgram);
	glBindVertexArray(gameVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gameVBO);
	
	// アルファブレンディングを有効化
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// オーバーレイ描画
	glBufferData(GL_ARRAY_BUFFER, sizeof(overlayVertices), overlayVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glDisable(GL_BLEND);
	
	// 勝敗テキストを描画
	float textScale = 0.25f;
	float textColor[3] = {1.0f, 1.0f, 1.0f}; // 白色
	
	if (winner == PLAYER_TIE) {
		// "TIE"を中央に表示
		renderText("TIE", -0.3f, -0.1f, textScale, textColor);
	} else if (winner == PLAYER_RED) {
		// "RED WINS"を中央に表示
		float redColor[3] = {1.0f, 0.3f, 0.3f};
		renderText("RED", -0.35f, 0.1f, textScale, redColor);
		renderText("WINS", -0.4f, -0.2f, textScale, textColor);
	} else if (winner == PLAYER_BLUE) {
		// "BLUE WINS"を中央に表示
		float blueColor[3] = {0.3f, 0.3f, 1.0f};
		renderText("BLUE", -0.4f, 0.1f, textScale, blueColor);
		renderText("WINS", -0.4f, -0.2f, textScale, textColor);
	}
	
	// "Press R to restart"メッセージ
	float smallTextScale = 0.08f;  // より小さなサイズに変更
	// 文字列の長さを計算して中央配置
	const char* restartText = "PRESS R TO RESTART";
	int textLength = strlen(restartText);
	float textWidth = textLength * smallTextScale * 1.0f;  // 概算の文字幅
	float centerX = -textWidth / 2.0f;  // 中央配置のX座標
	renderText(restartText, centerX, -0.7f, smallTextScale, textColor);
}

void setupTextures()
{
	// 画像を読み込み（c_puzzleディレクトリから相対パス）
	plusOneTexture = loadTexture("img/plus_1.png");
	minusOneTexture = loadTexture("img/neg_1.png");
	plusTwoTexture = loadTexture("img/plus_2.png");
}

void renderGame()
{
	GameState* game = getGameState();
	
	// ボードの描画（平面表示、画面の70%サイズ）
	float boardScale = 0.7f;  // ボードを画面の70%サイズに
	float cellSize = (2.0f * boardScale) / BOARD_SIZE;  
	float startX = -boardScale;
	float startY = boardScale;
	
	glUseProgram(shaderProgram);
	glBindVertexArray(gameVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gameVBO);
	
	// 各セルを描画
	for (int row = 0; row < BOARD_SIZE; row++) {
		float y1 = startY - row * cellSize;
		float y2 = y1 - cellSize;
		
		for (int col = 0; col < BOARD_SIZE; col++) {
			float x1 = startX + col * cellSize;
			float x2 = x1 + cellSize;
			
			// 列の背景色を決定
			float bgColor[3] = {0.8f, 0.8f, 0.8f};  // デフォルト（グレー）
			if (game->columnStates[col] == PAINTED_RED) {
				bgColor[0] = 1.0f; bgColor[1] = 0.3f; bgColor[2] = 0.3f;  // 赤
			} else if (game->columnStates[col] == PAINTED_BLUE) {
				bgColor[0] = 0.3f; bgColor[1] = 0.3f; bgColor[2] = 1.0f;  // 青
			}
			
			// セルの色を決定（全て背景色ベース）
			float cellColor[3] = {bgColor[0], bgColor[1], bgColor[2]};
			
			// +1/-1マスも無効マスも同じ背景色を使用
			
			// 四角形の頂点データ
			float vertices[] = {
				// 位置           // 色
				x1, y1, 0.0f,    cellColor[0], cellColor[1], cellColor[2],
				x2, y1, 0.0f,    cellColor[0], cellColor[1], cellColor[2],
				x2, y2, 0.0f,    cellColor[0], cellColor[1], cellColor[2],
				x1, y1, 0.0f,    cellColor[0], cellColor[1], cellColor[2],
				x2, y2, 0.0f,    cellColor[0], cellColor[1], cellColor[2],
				x1, y2, 0.0f,    cellColor[0], cellColor[1], cellColor[2]
			};
			
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
			
			// 頂点属性の設定
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			
			glDrawArrays(GL_TRIANGLES, 0, 6);
			
			// テクスチャの描画（+1/+2/-1マスの場合、テクスチャが有効な場合のみ）
			if (game->board[row][col] == PLUS_ONE && plusOneTexture != 0) {
				renderTexture(plusOneTexture, x1, y1, x2, y2);
			} else if (game->board[row][col] == PLUS_TWO && plusTwoTexture != 0) {
				renderTexture(plusTwoTexture, x1, y1, x2, y2);
			} else if (game->board[row][col] == MINUS_ONE && minusOneTexture != 0) {
				renderTexture(minusOneTexture, x1, y1, x2, y2);
			}
		}
	}
	
	// グリッド線の描画（黒い線）
	float gridColor[3] = {0.1f, 0.1f, 0.1f};
	
	// 横線
	for (int row = 0; row <= BOARD_SIZE; row++) {
		float y = startY - row * cellSize;
		float line[] = {
			startX, y, 0.0f, gridColor[0], gridColor[1], gridColor[2],
			startX + BOARD_SIZE * cellSize, y, 0.0f, gridColor[0], gridColor[1], gridColor[2]
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glDrawArrays(GL_LINES, 0, 2);
	}
	
	// 縦線
	for (int col = 0; col <= BOARD_SIZE; col++) {
		float x = startX + col * cellSize;
		float line[] = {
			x, startY, 0.0f, gridColor[0], gridColor[1], gridColor[2],
			x, startY - BOARD_SIZE * cellSize, 0.0f, gridColor[0], gridColor[1], gridColor[2]
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glDrawArrays(GL_LINES, 0, 2);
	}
	
	// ゲーム終了時のみコンソール出力（一度だけ）
	static bool gameEndOutputShown = false;
	if (game->gameOver && !gameEndOutputShown) {
		gameEndOutputShown = true;
		printf("\033[2J\033[H"); // 画面クリア
		printf("GAME OVER!\n");
		printf("===============================\n");
		printf("FINAL BOARD:\n");
		// 列ヘッダーを表示
		printf("     ");
		for (int col = 0; col < BOARD_SIZE; col++) {
			if (game->columnStates[col] == PAINTED_RED) {
				printf("RED ");
			} else if (game->columnStates[col] == PAINTED_BLUE) {
				printf("BLU ");
			} else {
				printf("--- ");
			}
		}
		printf("\n");
		
		for (int row = 0; row < BOARD_SIZE; row++) {
			printf("     ");
			for (int col = 0; col < BOARD_SIZE; col++) {
				if (game->board[row][col] == PLUS_ONE) {
					printf(" +1 ");
				} else if (game->board[row][col] == MINUS_ONE) {
					printf(" -1 ");
				} else {
					printf("  0 ");
				}
			}
			printf("\n");
		}
		printf("===============================\n");
		printf("Red Score: %d\n", game->redScore);
		printf("Blue Score: %d\n", game->blueScore);
		Player winner = getWinner();
		if (winner == PLAYER_TIE) {
			printf("RESULT: TIE!\n");
		} else {
			printf("WINNER: %s!\n", (winner == PLAYER_RED) ? "RED" : "BLUE");
		}
		printf("===============================\n");
		printf("Press R to restart game\n");
		fflush(stdout);
	}
	
	// ゲームがリセットされたらフラグをリセット
	if (!game->gameOver) {
		gameEndOutputShown = false;
	}
	
	// スコア表示
	renderScore(game->redScore, game->blueScore);
	
	// +2変化演出の処理
	if (game->effectState != NO_EFFECT) {
		double currentTime = glfwGetTime();
		double elapsedTime = currentTime - game->effectStartTime;
		
		if (game->effectState == EFFECT_WAITING) {
			// 0.5秒待機
			if (elapsedTime >= 0.5) {
				// 0.5秒経過したら暗転開始
				game->effectState = EFFECT_DARKENING;
				game->effectStartTime = currentTime;
			}
		} else if (game->effectState == EFFECT_DARKENING) {
			// 暗転演出（2秒間）
			if (elapsedTime < 2.0) {
				// 暗いオーバーレイを描画
				float alpha = 0.6f;
				float overlay[] = {
					-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
					 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
					 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
					-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
					 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
					-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f
				};
				
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				
				glUseProgram(shaderProgram);
				glBindVertexArray(gameVAO);
				glBindBuffer(GL_ARRAY_BUFFER, gameVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(overlay), overlay, GL_DYNAMIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisable(GL_BLEND);
				
				// 暗転中は"+2"テキストを中央に大きく表示
				float textColor[3] = {1.0f, 1.0f, 0.0f}; // 黄色
				float textScale = 0.5f; // 大きなサイズ
				
				// "+2"の文字幅を計算して中央に配置
				// +文字 + 2文字 = 約2文字分の幅
				float totalWidth = textScale * 1.0f + textScale * 1.2f; // +の幅 + 2の幅
				float centerX = -totalWidth / 2.0f;
				
				renderText("+2", centerX, 0.0f, textScale, textColor);
			} else {
				// 2秒経過したら演出終了、実際の変化を適用
				applyPlusTwoChange();
				game->effectState = NO_EFFECT;
			}
		}
	}
	
	// ゲーム終了時は暗転オーバーレイを表示
	if (game->gameOver) {
		Player winner = getWinner();
		renderGameOverScreen(winner);
	}
}

void cleanupGameRenderer()
{
	glDeleteVertexArrays(1, &gameVAO);
	glDeleteBuffers(1, &gameVBO);
	glDeleteVertexArrays(1, &textureVAO);
	glDeleteBuffers(1, &textureVBO);
}

unsigned int compileShader(unsigned int type, const char* source)
{
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	// エラーチェック
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "Shader compile error:\n" << infoLog << std::endl;
	}
	return shader;
}

void setupShaders()
{
	unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// エラーチェック
	int success;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "Shader linking error:\n" << infoLog << std::endl;
	}

	// シェーダーはリンク後に削除可能
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	// テクスチャシェーダー
	unsigned int textureVertexShader = compileShader(GL_VERTEX_SHADER, textureVertexShaderSource);
	unsigned int textureFragmentShader = compileShader(GL_FRAGMENT_SHADER, textureFragmentShaderSource);

	textureShaderProgram = glCreateProgram();
	glAttachShader(textureShaderProgram, textureVertexShader);
	glAttachShader(textureShaderProgram, textureFragmentShader);
	glLinkProgram(textureShaderProgram);

	// テクスチャシェーダーのエラーチェック
	glGetProgramiv(textureShaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(textureShaderProgram, 512, NULL, infoLog);
		std::cerr << "Texture shader linking error:\n" << infoLog << std::endl;
	}

	glDeleteShader(textureVertexShader);
	glDeleteShader(textureFragmentShader);
	
	// テクスチャを初期化
	setupTextures();
	
	// テキストレンダラーを初期化
	setupTextRenderer();
}

void cleanupShaders()
{
	glDeleteProgram(shaderProgram);
	glDeleteProgram(textureShaderProgram);
	cleanupTextRenderer();
}

void cleanupTextures()
{
	glDeleteTextures(1, &plusOneTexture);
	glDeleteTextures(1, &minusOneTexture);
}


