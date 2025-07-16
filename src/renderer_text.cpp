#include "renderer.h"

// テキスト描画用の変数
unsigned int textVAO, textVBO;
unsigned int digitTextures[10];  // 0-9の数字テクスチャ

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