#pragma once

#include <ncurses.h>
#include <vector>
#include <string>
#include <chrono>

// Game constants
constexpr int WIDTH = 10;
constexpr int HEIGHT = 20;
constexpr int FALL_SPEED_MS = 500;
constexpr int GAME_TICK_MS = 50;

// Color pairs
constexpr int COLOR_BORDER = 1;
constexpr int COLOR_BLOCK = 2;
constexpr int COLOR_TEXT = 3;
constexpr int COLOR_SCORE = 4;

// Tetromino shapes
const std::vector<std::vector<std::vector<int>>> SHAPES = {
    {{1, 1, 1, 1}},                     // I
    {{1, 1}, {1, 1}},                   // O
    {{0, 1, 0}, {1, 1, 1}},            // T
    {{1, 1, 0}, {0, 1, 1}},            // Z
    {{0, 1, 1}, {1, 1, 0}},            // S
    {{1, 1, 1}, {1, 0, 0}},            // L
    {{1, 1, 1}, {0, 0, 1}}             // J
};

class Tetromino {
public:
    Tetromino(int x, int y, const std::vector<std::vector<int>>& blocks, int color);
    
    void rotate();
    bool isValidMove(int newX, int newY, const std::vector<std::vector<int>>& board) const;
    void mergeToBoard(std::vector<std::vector<int>>& board) const;
    
    int getX() const { return x; }
    int getY() const { return y; }
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    const std::vector<std::vector<int>>& getBlocks() const { return blocks; }
    int getColor() const { return color; }

private:
    int x, y;
    std::vector<std::vector<int>> blocks;
    int color;
};

class Game {
public:
    Game();
    ~Game();

    void run();
    void init();
    void cleanup();

private:
    void initColors();
    void drawBoard() const;
    void clearFullRows();
    Tetromino getNewPiece() const;
    void showGameOver() const;
    void handleInput();
    void update();

    std::vector<std::vector<int>> board;
    Tetromino currentPiece;
    Tetromino nextPiece;
    int score;
    bool gameOver;
    std::chrono::steady_clock::time_point lastFallTime;
}; 