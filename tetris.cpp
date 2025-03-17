#include <ncurses.h>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

const std::string BLOCK = "##";
const int WIDTH = 10, HEIGHT = 20;

struct Tetromino {
	int x, y;
	std::vector<std::vector<int>> blocks;

	void rotate() {
		std::vector<std::vector<int>> newBlocks(4, std::vector<int>(4, 0));
		for (int i = 0; i < blocks.size(); i++) {
			for (int j = 0; j < blocks[i].size(); j++) {
				newBlocks[j][blocks.size() - 1 - i] = blocks[i][j];
			}
		}
		blocks = newBlocks;
	}
};

std::vector<Tetromino> tetrominoes = {
	{0, 0, {{1, 1, 1, 1}}}, // I
	{0, 0, {{1, 1}, {1, 1}}}, // O
	{0, 0, {{0, 1, 0}, {1, 1, 1}}}, // T
	{0, 0, {{1, 1, 0}, {0, 1, 1}}}, // Z
	{0, 0, {{0, 1, 1}, {1, 1, 0}}}, // S
	{0, 0, {{1, 1, 1}, {1, 0, 0}}}, // L
	{0, 0, {{1, 1, 1}, {0, 0, 1}}}  // J
};

bool isValidMove(int newX, int newY, const std::vector<std::vector<int>>& shape, const std::vector<std::vector<int>>& board) {
	for (int i = 0; i < shape.size(); i++) {
		for (int j = 0; j < shape[i].size(); j++) {
			if (shape[i][j]) {
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

void mergePieceToBoard(std::vector<std::vector<int>>& board, const Tetromino& piece) {
	for (int i = 0; i < piece.blocks.size(); i++) {
		for (int j = 0; j < piece.blocks[i].size(); j++) {
			if (piece.blocks[i][j]) {
				board[piece.y + i][piece.x + j] = 1;
			}
		}
	}
}

void drawBoard(const std::vector<std::vector<int>>& board, const Tetromino& piece, const int& score) {
    clear();

    // Draw top border
    mvprintw(0, 0, "+"); 
    mvprintw(0, (WIDTH + 1) * 2, "+");
    for (int x = 1; x <= WIDTH; x++) {
        mvprintw(0, x * 2, "-");
    }

    // Draw side borders and board content
    for (int y = 0; y < HEIGHT; y++) {
        mvprintw(y + 1, 0, "|");  // Left border
        mvprintw(y + 1, (WIDTH + 1) * 2, "|");  // Right border
        for (int x = 0; x < WIDTH; x++) {
            if (board[y][x]) mvprintw(y + 1, (x + 1) * 2, BLOCK.c_str());
        }
    }

    // Draw bottom border
    mvprintw(HEIGHT + 1, 0, "+"); 
    mvprintw(HEIGHT + 1, (WIDTH + 1) * 2, "+");
    for (int x = 1; x <= WIDTH; x++) {
        mvprintw(HEIGHT + 1, x * 2, "-");
    }
	mvprintw(HEIGHT + 2, 0, "Score: %d", score);

    // Draw current Tetromino inside the board
    for (int i = 0; i < piece.blocks.size(); i++) {
        for (int j = 0; j < piece.blocks[i].size(); j++) {
            if (piece.blocks[i][j]) {
                mvprintw(piece.y + i + 1, (piece.x + j + 1) * 2, BLOCK.c_str());
            }
        }
    }

    refresh();
}

void clearFullRows(std::vector<std::vector<int>>& board, int& score) {
	for (int y = HEIGHT - 1; y >= 0; y--) {
		if (std::all_of(board[y].begin(), board[y].end(), [](int cell) { return cell == 1; })) {
			board.erase(board.begin() + y);  
			board.insert(board.begin(), std::vector<int>(WIDTH, 0)); 
			y++; // Recheck the same row
			score += 10;
		}
	}
}

Tetromino getNewPiece() {
	Tetromino newPiece = tetrominoes[rand() % tetrominoes.size()]; // Randdom number generators suck
	newPiece.x = WIDTH / 2 - 2;
	newPiece.y = 0;
	return newPiece;
}

int main() {
	initscr();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	timeout(100);

	std::vector<std::vector<int>> board(HEIGHT, std::vector<int>(WIDTH, 0));
	Tetromino currentPiece = getNewPiece();
	int score = 0;

	auto lastFallTime = std::chrono::steady_clock::now();

	while (true) {
		drawBoard(board, currentPiece, score);
		int ch = getch();
		if (ch == 'q') break;
		else if (ch == KEY_LEFT && isValidMove(currentPiece.x - 1, currentPiece.y, currentPiece.blocks, board)) {
			currentPiece.x--;
		} else if (ch == KEY_RIGHT && isValidMove(currentPiece.x + 1, currentPiece.y, currentPiece.blocks, board)) {
			currentPiece.x++;
		} else if (ch == KEY_DOWN && isValidMove(currentPiece.x, currentPiece.y + 1, currentPiece.blocks, board)) {
			currentPiece.y++;
		} else if (ch == ' ') {
			Tetromino rotated = currentPiece;
			rotated.rotate();
			if (isValidMove(rotated.x, rotated.y, rotated.blocks, board)) {
				currentPiece.rotate();
			}
		}

		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFallTime).count() > 500) {
			if (isValidMove(currentPiece.x, currentPiece.y + 1, currentPiece.blocks, board)) {
				currentPiece.y++;
			} else {
				mergePieceToBoard(board, currentPiece);
				currentPiece = getNewPiece();
				if (!isValidMove(currentPiece.x, currentPiece.y, currentPiece.blocks, board)) {
					break; // Game over condition
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			clearFullRows(board, score);
			lastFallTime = now;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	endwin();
	std::cout << "Game Over!\nScore: " << score << std::endl;
	return 0;
}