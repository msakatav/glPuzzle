#include "game.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

static GameState gameState;

GameState* getGameState() { return &gameState; }

void initGame() {
    srand(time(NULL));
    resetGame();
}

void initBoard() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            int randVal = rand() % 3;
            switch (randVal) {
                case 0: gameState.board[i][j] = INVALID; break;
                case 1: gameState.board[i][j] = PLUS_ONE; break;
                case 2: gameState.board[i][j] = MINUS_ONE; break;
            }
        }
    }
}

void resetGame() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        gameState.columnStates[i] = EMPTY;
    }
    gameState.currentPlayer = PLAYER_RED;
    gameState.redScore = 0;
    gameState.blueScore = 0;
    gameState.gameOver = false;
    gameState.paintedColumns = 0;
    gameState.waitingForAI = false;
    gameState.aiStartTime = 0.0;
    // +2状態もリセット
    gameState.effectState = NO_EFFECT;
    gameState.effectStartTime = 0.0;
    gameState.plusTwoTriggered = false;
    
    // ボードも再生成して+2マスを+1に戻す
    initBoard();
}

// 未塗装の列数を数える
int countUnpaintedColumns() {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (gameState.columnStates[i] == EMPTY) {
            count++;
        }
    }
    return count;
}

// +1を+2に変化させる処理
void triggerPlusTwoEffect() {
    if (gameState.plusTwoTriggered) return;  // 既に発生済み
    
    // 演出開始（実際の変化は演出後に行う）
    gameState.plusTwoTriggered = true;
    gameState.effectState = EFFECT_WAITING;  // まず0.5秒待機
    gameState.effectStartTime = glfwGetTime();
}

// 実際に+1を+2に変更する関数
void applyPlusTwoChange() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gameState.board[i][j] == PLUS_ONE) {
                gameState.board[i][j] = PLUS_TWO;
            }
        }
    }
}

bool selectColumn(int col) {
    if (col < 0 || col >= BOARD_SIZE) return false;

    // 交互にしか塗れない
    if (gameState.columnStates[col] == PAINTED_RED && gameState.currentPlayer != PLAYER_BLUE) return false;
    if (gameState.columnStates[col] == PAINTED_BLUE && gameState.currentPlayer != PLAYER_RED) return false;

    // 初めて塗る場合のみカウント
    bool firstPaint = (gameState.columnStates[col] == EMPTY);

    // まず現在のマス配置でスコア計算（選択前の状態で）
    if (gameState.currentPlayer == PLAYER_RED) {
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (gameState.board[i][col] == PLUS_ONE) {
                gameState.redScore++;  // 赤が+1マス → 赤のスコア+1
            } else if (gameState.board[i][col] == PLUS_TWO) {
                gameState.redScore += 2;  // 赤が+2マス → 赤のスコア+2
            } else if (gameState.board[i][col] == MINUS_ONE) {
                gameState.blueScore--; // 赤が-1マス → 青のスコア-1
            }
        }
    } else {
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (gameState.board[i][col] == PLUS_ONE) {
                gameState.blueScore++; // 青が+1マス → 青のスコア+1
            } else if (gameState.board[i][col] == PLUS_TWO) {
                gameState.blueScore += 2; // 青が+2マス → 青のスコア+2
            } else if (gameState.board[i][col] == MINUS_ONE) {
                gameState.redScore--;  // 青が-1マス → 赤のスコア-1
            }
        }
    }

    // 列を塗る
    if (gameState.currentPlayer == PLAYER_RED) {
        gameState.columnStates[col] = PAINTED_RED;
    } else {
        gameState.columnStates[col] = PAINTED_BLUE;
    }
    if (firstPaint) gameState.paintedColumns++;

    // その列のマスを再生成
    for (int i = 0; i < BOARD_SIZE; i++) {
        int randVal = rand() % 3;
        switch (randVal) {
            case 0: gameState.board[i][col] = INVALID; break;
            case 1: 
                // +2変化が発生済みなら+2を生成、そうでなければ+1を生成
                gameState.board[i][col] = gameState.plusTwoTriggered ? PLUS_TWO : PLUS_ONE; 
                break;
            case 2: gameState.board[i][col] = MINUS_ONE; break;
        }
    }

    // 残り3列になったら+1を+2に変化
    if (countUnpaintedColumns() == 3 && !gameState.plusTwoTriggered) {
        triggerPlusTwoEffect();
    }

    if (isGameOver()) {
        gameState.gameOver = true;
    } else {
        switchPlayer();
    }
    return true;
}

void calculateScore() {
    gameState.redScore = 0;
    gameState.blueScore = 0;
    
    // スコアの計算
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (gameState.columnStates[i] == PAINTED_RED) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (gameState.board[j][i] == PLUS_ONE) {
                    gameState.redScore++;  // 赤が+1マス → 赤のスコア+1
                } else if (gameState.board[j][i] == PLUS_TWO) {
                    gameState.redScore += 2;  // 赤が+2マス → 赤のスコア+2
                } else if (gameState.board[j][i] == MINUS_ONE) {
                    gameState.blueScore--; // 赤が-1マス → 青のスコア-1
                }
            }
        } else if (gameState.columnStates[i] == PAINTED_BLUE) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (gameState.board[j][i] == PLUS_ONE) {
                    gameState.blueScore++; // 青が+1マス → 青のスコア+1
                } else if (gameState.board[j][i] == PLUS_TWO) {
                    gameState.blueScore += 2; // 青が+2マス → 青のスコア+2
                } else if (gameState.board[j][i] == MINUS_ONE) {
                    gameState.redScore--;  // 青が-1マス → 赤のスコア-1
                }
            }
        }
    }
}

bool isGameOver() {
    return gameState.paintedColumns >= BOARD_SIZE;
}

Player getWinner() {
    if (gameState.redScore > gameState.blueScore) return PLAYER_RED;
    if (gameState.blueScore > gameState.redScore) return PLAYER_BLUE;
    return PLAYER_TIE;
}

void switchPlayer() {
    gameState.currentPlayer = (gameState.currentPlayer == PLAYER_RED) ? PLAYER_BLUE : PLAYER_RED;
    
    // 青プレイヤーのターンになったらAI待機状態にする
    if (gameState.currentPlayer == PLAYER_BLUE) {
        gameState.waitingForAI = true;
        gameState.aiStartTime = glfwGetTime();  // 現在時刻を記録
    }
}

int getBestColumnForBlue() {
    int bestColumn = -1;
    int minInvalidCells = BOARD_SIZE + 1;  // 最大値+1で初期化
    int bestScore = -1000;  // スコアも考慮
    
    // 残り一列で青のスコアが高い場合の特別処理
    int unpaintedCount = countUnpaintedColumns();
    if (unpaintedCount == 1 && gameState.blueScore > gameState.redScore) {
        // 塗られていない列を探して選択
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (gameState.columnStates[col] == EMPTY) {
                return col;  // 勝利確定のためその列を選択
            }
        }
    }
    
    // 各列を評価
    for (int col = 0; col < BOARD_SIZE; col++) {
        // その列が選択可能かチェック
        if (gameState.columnStates[col] == PAINTED_BLUE && gameState.currentPlayer != PLAYER_RED) continue;
        if (gameState.columnStates[col] == PAINTED_RED && gameState.currentPlayer != PLAYER_BLUE) continue;
        
        // この列の空白マス（INVALID）数を数える
        int invalidCount = 0;
        int currentScore = 0;
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gameState.board[j][col] == INVALID) {
                invalidCount++;
            } else if (gameState.board[j][col] == PLUS_ONE) {
                currentScore += 1;  // 青が+1マス → 青のスコア+1
            } else if (gameState.board[j][col] == PLUS_TWO) {
                currentScore += 2;  // 青が+2マス → 青のスコア+2
            } else if (gameState.board[j][col] == MINUS_ONE) {
                currentScore -= 1;  // 青が-1マス → 赤のスコア-1（青には悪い）
            }
        }
        
        // 空白マスが少ない列を優先、同じ場合はスコアで決定
        bool isBetter = false;
        if (invalidCount < minInvalidCells) {
            isBetter = true;
        } else if (invalidCount == minInvalidCells && currentScore > bestScore) {
            isBetter = true;
        }
        
        if (isBetter) {
            bestColumn = col;
            minInvalidCells = invalidCount;
            bestScore = currentScore;
        }
    }
    
    return bestColumn;
}

void updateAI() {
    if (gameState.waitingForAI && !gameState.gameOver) {
        double currentTime = glfwGetTime();
        if (currentTime - gameState.aiStartTime >= 1.0) {  // 1秒待機
            int col = getBestColumnForBlue();
            if (col != -1) {
                selectColumn(col);
            }
            gameState.waitingForAI = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (gameState.gameOver) return;
        
        // 赤プレイヤーのターンでないなら無視
        if (gameState.currentPlayer != PLAYER_RED) return;
        
        // マウス座標を取得
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        // ウィンドウサイズを取得
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        
        // 正規化座標に変換 (-1 to 1)
        float normalizedX = (float)(2.0 * xpos / width - 1.0);
        float normalizedY = (float)(1.0 - 2.0 * ypos / height);
        
        // ゲームボードの座標計算
        float cellSize = 1.6f / BOARD_SIZE;  // セルのサイズ
        float startX = -0.8f;
        float startY = 0.8f;
        
        // どの列をクリックしたかを計算
        int col = (int)((normalizedX - startX) / cellSize);
        
        // 有効な範囲内で、かつゲームボード内のクリックかチェック
        if (col >= 0 && col < BOARD_SIZE && 
            normalizedX >= startX && normalizedX <= startX + BOARD_SIZE * cellSize &&
            normalizedY <= startY && normalizedY >= startY - BOARD_SIZE * cellSize) {
            selectColumn(col);
        }
    }
}
