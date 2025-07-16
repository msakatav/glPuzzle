#include "renderer.h"

unsigned int gameVAO, gameVBO;

void setupGameRenderer()
{
	glGenVertexArrays(1, &gameVAO);
	glGenBuffers(1, &gameVBO);
	
	// テクスチャ用VAO/VBOの設定
	glGenVertexArrays(1, &textureVAO);
	glGenBuffers(1, &textureVBO);
}

void cleanupGameRenderer()
{
	glDeleteVertexArrays(1, &gameVAO);
	glDeleteBuffers(1, &gameVBO);
	glDeleteVertexArrays(1, &textureVAO);
	glDeleteBuffers(1, &textureVBO);
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