#include "renderer.h"

unsigned int shaderProgram;
unsigned int textureShaderProgram;

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