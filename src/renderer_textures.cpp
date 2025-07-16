#include "renderer.h"

unsigned int textureVAO, textureVBO;
unsigned int plusOneTexture, minusOneTexture, plusTwoTexture;


void setupTextures()
{
	// 画像を読み込み（c_puzzleディレクトリから相対パス）
	plusOneTexture = loadTexture("img/plus_1.png");
	minusOneTexture = loadTexture("img/neg_1.png");
	plusTwoTexture = loadTexture("img/plus_2.png");
}

void cleanupTextures()
{
	glDeleteTextures(1, &plusOneTexture);
	glDeleteTextures(1, &minusOneTexture);
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
