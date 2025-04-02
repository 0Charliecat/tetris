#include <ncurses.h>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <random>

const std::string BLOCK = "[]";
const int WIDTH = 10, HEIGHT = 20;

// Color pairs
const int COLOR_BORDER = 1;
const int COLOR_BLOCK = 2;
const int COLOR_TEXT = 3;
const int COLOR_SCORE = 4;

struct Tetromino {
	int x, y;
	std::vector<std::vector<int>> blocks;
	int color;  // Add color property

	Tetromino(int x, int y, const std::vector<std::vector<int>>& blocks, int color)
		: x(x), y(y), blocks(blocks), color(color) {}

	void rotate() {
		std::vector<std::vector<int>> newBlocks(4, std::vector<int>(4, 0));
		for (int i = 0; i < blocks.size(); i++) {
			for (int j = 0; j < blocks[i].size(); j++) {
				newBlocks[j][blocks.size() - 1 - i] = blocks[i][j];
			}
		}
		blocks = newBlocks;
	}

	bool isValidMove(int newX, int newY, const std::vector<std::vector<int>>& board) const {
		for (int i = 0; i < blocks.size(); i++) {
			for (int j = 0; j < blocks[i].size(); j++) {
				if (blocks[i][j]) {
					int boardX = newX + j;
					int boardY = newY + i;
					if (boardX < 0 || boardX >= WIDTH || boardY >= HEIGHT || board[boardY][boardX]) {
						return false;
					}
				}
			}
		}
		return true;
	}

	void mergeToBoard(std::vector<std::vector<int>>& board) const {
		for (int i = 0; i < blocks.size(); i++) {
			for (int j = 0; j < blocks[i].size(); j++) {
				if (blocks[i][j]) {
					board[y + i][x + j] = 1;
				}
			}
		}
	}
};

// Define tetrominoes with colors
std::vector<Tetromino> tetrominoes = {
	{0, 0, {{1, 1, 1, 1}}, COLOR_RED},     // I
	{0, 0, {{1, 1}, {1, 1}}, COLOR_GREEN}, // O
	{0, 0, {{0, 1, 0}, {1, 1, 1}}, COLOR_YELLOW}, // T
	{0, 0, {{1, 1, 0}, {0, 1, 1}}, COLOR_BLUE},   // Z
	{0, 0, {{0, 1, 1}, {1, 1, 0}}, COLOR_MAGENTA}, // S
	{0, 0, {{1, 1, 1}, {1, 0, 0}}, COLOR_CYAN},    // L
	{0, 0, {{1, 1, 1}, {0, 0, 1}}, COLOR_WHITE}    // J
};

class Game {
public:
	Game() : board(HEIGHT, std::vector<int>(WIDTH, 0)), 
			   currentPiece(0, 0, tetrominoes[0].blocks, tetrominoes[0].color),
			   nextPiece(0, 0, tetrominoes[0].blocks, tetrominoes[0].color),
			   score(0), gameOver(false) {
		init();
	}

	~Game() {
		cleanup();
	}

	void init() {
		initscr();
		noecho();
		curs_set(0);
		keypad(stdscr, TRUE);
		timeout(100);
		initColors();
		
		currentPiece = getNewPiece();
		nextPiece = getNewPiece();
		lastFallTime = std::chrono::steady_clock::now();
	}

	void cleanup() {
		endwin();
	}

	void initColors() {
		start_color();
		init_pair(COLOR_BORDER, COLOR_WHITE, COLOR_BLACK);
		init_pair(COLOR_BLOCK, COLOR_CYAN, COLOR_CYAN);
		init_pair(COLOR_TEXT, COLOR_WHITE, COLOR_BLACK);
		init_pair(COLOR_SCORE, COLOR_GREEN, COLOR_BLACK);
	}

	void drawBoard() const {
		clear();

		// Draw game area border using ASCII characters
		attron(COLOR_PAIR(COLOR_BORDER));
		// Draw corners
		mvprintw(0, 0, "+");
		mvprintw(0, (WIDTH + 1) * 2, "+");
		mvprintw(HEIGHT + 1, 0, "+");
		mvprintw(HEIGHT + 1, (WIDTH + 1) * 2, "+");

		// Draw horizontal borders
		for (int x = 1; x <= WIDTH; x++) {
			mvprintw(0, x * 2, "--");
			mvprintw(HEIGHT + 1, x * 2, "--");
		}

		// Draw vertical borders
		for (int y = 1; y <= HEIGHT; y++) {
			mvprintw(y, 0, "|");
			mvprintw(y, (WIDTH + 1) * 2, "|");
		}
		attroff(COLOR_PAIR(COLOR_BORDER));

		// Draw board content
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				if (board[y][x]) {
					attron(COLOR_PAIR(COLOR_BLOCK));
					mvprintw(y + 1, (x + 1) * 2, BLOCK.c_str());
					attroff(COLOR_PAIR(COLOR_BLOCK));
				}
			}
		}

		// Draw current piece
		attron(COLOR_PAIR(currentPiece.color));
		for (int i = 0; i < currentPiece.blocks.size(); i++) {
			for (int j = 0; j < currentPiece.blocks[i].size(); j++) {
				if (currentPiece.blocks[i][j]) {
					mvprintw(currentPiece.y + i + 1, (currentPiece.x + j + 1) * 2, BLOCK.c_str());
				}
			}
		}
		attroff(COLOR_PAIR(currentPiece.color));

		// Draw score and next piece preview
		attron(COLOR_PAIR(COLOR_TEXT));
		mvprintw(2, WIDTH * 2 + 4, "NEXT PIECE:");
		mvprintw(HEIGHT - 2, WIDTH * 2 + 4, "SCORE: %d", score);
		attroff(COLOR_PAIR(COLOR_TEXT));

		// Draw next piece preview
		attron(COLOR_PAIR(nextPiece.color));
		for (int i = 0; i < nextPiece.blocks.size(); i++) {
			for (int j = 0; j < nextPiece.blocks[i].size(); j++) {
				if (nextPiece.blocks[i][j]) {
					mvprintw(4 + i, WIDTH * 2 + 4 + j * 2, BLOCK.c_str());
				}
			}
		}
		attroff(COLOR_PAIR(nextPiece.color));

		refresh();
	}

	void clearFullRows() {
		for (int y = HEIGHT - 1; y >= 0; y--) {
			if (std::all_of(board[y].begin(), board[y].end(), [](int cell) { return cell == 1; })) {
				board.erase(board.begin() + y);  
				board.insert(board.begin(), std::vector<int>(WIDTH, 0)); 
				y++; // Recheck the same row
				score += 10;
			}
		}
	}

	Tetromino getNewPiece() const {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<> dis(0, tetrominoes.size() - 1);
		
		int shapeIndex = dis(gen);
		int color = COLOR_RED + shapeIndex;
		return Tetromino(WIDTH / 2 - 2, 0, tetrominoes[shapeIndex].blocks, color);
	}

	void showGameOver() const {
		clear();
		attron(COLOR_PAIR(COLOR_TEXT));
		mvprintw(HEIGHT/2 - 2, WIDTH - 4, "GAME OVER!");
		mvprintw(HEIGHT/2, WIDTH - 6, "Score: %d", score);
		mvprintw(HEIGHT/2 + 2, WIDTH - 8, "Press 'q' to quit");
		attroff(COLOR_PAIR(COLOR_TEXT));
		refresh();
	}

	void handleInput() {
		int ch = getch();
		if (ch == 'q') {
			gameOver = true;
			return;
		}

		if (ch == KEY_LEFT && currentPiece.isValidMove(currentPiece.x - 1, currentPiece.y, board)) {
			currentPiece.x--;
		} else if (ch == KEY_RIGHT && currentPiece.isValidMove(currentPiece.x + 1, currentPiece.y, board)) {
			currentPiece.x++;
		} else if (ch == KEY_DOWN && currentPiece.isValidMove(currentPiece.x, currentPiece.y + 1, board)) {
			currentPiece.y++;
		} else if (ch == ' ') {
			Tetromino rotated = currentPiece;
			rotated.rotate();
			if (rotated.isValidMove(rotated.x, rotated.y, board)) {
				currentPiece.rotate();
			}
		}
	}

	void update() {
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFallTime).count() > 500) {
			if (currentPiece.isValidMove(currentPiece.x, currentPiece.y + 1, board)) {
				currentPiece.y++;
			} else {
				currentPiece.mergeToBoard(board);
				currentPiece = nextPiece;
				nextPiece = getNewPiece();
				if (!currentPiece.isValidMove(currentPiece.x, currentPiece.y, board)) {
					gameOver = true;
				}
			}
			clearFullRows();
			lastFallTime = now;
		}
	}

	void run() {
		while (!gameOver) {
			drawBoard();
			handleInput();
			update();
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		showGameOver();
		while (getch() != 'q') {}
	}
};

int main() {
	Game game;
	game.run();
	return 0;
}