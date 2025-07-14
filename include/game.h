#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <GLFW/glfw3.h>

// ゲーム定数
#define BOARD_SIZE 6
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// マスの状態
enum CellValue {
    INVALID = 0,
    PLUS_ONE = 1,
    MINUS_ONE = -1,
    PLUS_TWO = 2
};

// プレイヤー
enum Player {
    PLAYER_RED = 1,
    PLAYER_BLUE = 2,
    PLAYER_TIE = 0  // 引き分け
};

// 列の状態
enum ColumnState {
    EMPTY = 0,
    PAINTED_RED = 1,
    PAINTED_BLUE = 2
};

// 演出状態
enum EffectState {
    NO_EFFECT = 0,
    EFFECT_WAITING = 1,    // 0.5秒待機
    EFFECT_DARKENING = 2   // 暗転演出
};

// ゲーム状態
typedef struct {
    CellValue board[BOARD_SIZE][BOARD_SIZE];  // 6x6のボード
    ColumnState columnStates[BOARD_SIZE];     // 各列の状態
    Player currentPlayer;                     // 現在のプレイヤー
    int redScore;                             // 赤のスコア
    int blueScore;                            // 青のスコア
    bool gameOver;                            // ゲーム終了フラグ
    int paintedColumns;                       // 塗られた列の数
    bool waitingForAI;                        // AI待機中フラグ
    double aiStartTime;                       // AI思考開始時間
    EffectState effectState;                  // 演出状態
    double effectStartTime;                   // 演出開始時間
    bool plusTwoTriggered;                    // +2変化が発生したかのフラグ
} GameState;

// ゲーム関数
void initGame();
void initBoard();
void resetGame();
bool selectColumn(int col);
void calculateScore();
bool isGameOver();
Player getWinner();
void switchPlayer();
int countUnpaintedColumns();
void triggerPlusTwoEffect();
void applyPlusTwoChange();

// AI関数
int getBestColumnForBlue();
void makeAIMove();
void updateAI();  // AI待機時間を管理

// マウス入力
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
int getColumnFromMousePos(double mouseX, double mouseY);

// ゲーム状態の取得
GameState* getGameState();

#endif // GAME_H
