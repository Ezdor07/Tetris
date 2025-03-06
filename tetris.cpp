#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;

string ANSI_CODES[]{
	"\033[43m",
	"\033[46m",
	"\033[48;5;166m", //ORANGE
	"\033[44m",
	"\033[42m",
	"\033[41m",
	"\033[45m",
	"\033[m",
	"\033[100m",
	"\033[48;5;235m",
	"\033[H",
	"\033[?25l",
	"\033[?25h"

};

const int fallingDelays[] = { 800, 720, 630, 550, 470, 380, 300, 220, 130, 100, 100, 83, 83, 67, 67, 67, 50, 50, 50, 33 };

enum Shape {
	O,
	I,
	L,
	J,
	S,
	Z,
	T
};

enum BoardInfo {
	HEIGHT = 20,
	WIDTH = 10,
};

enum AnsiCodeIndexes {
	YELLOW,
	CYAN,
	ORANGE,
	BLUE,
	GREEN,
	RED,
	PURPLE,
	DEFAULT,
	GRAY,
	DARK_GRAY,
	RESET_CURSOR,
	HIDE_CURSOR,
	SHOW_CURSOR
};

enum Input {
	ARROW_UP = 72,
	ARROW_DOWN = 80,
	ARROW_LEFT = 75,
	ARROW_RIGHT = 77,
	SPACEBAR = 32,
};

struct Board {
	int state;
	int color;
};

struct Position {
	int x;
	int y;
};

struct Block {
	Position positions[4];
	int color;
};

const string SQUARE = "  ";

void drawTetromino(int block, int row) {
	string output[2];
	switch (block) {
	case O:
		output[0] = ANSI_CODES[DEFAULT] + SQUARE + ANSI_CODES[YELLOW] + SQUARE + SQUARE + ANSI_CODES[DEFAULT] + SQUARE;
		output[1] = output[0];
		break;
	case I:
		output[0] = ANSI_CODES[CYAN] + SQUARE + SQUARE + SQUARE + SQUARE;
		output[1] = ANSI_CODES[DEFAULT] + SQUARE + SQUARE + SQUARE + SQUARE;
		break;
	case L:
		output[0] = ANSI_CODES[DEFAULT] + SQUARE + SQUARE + ANSI_CODES[ORANGE] + SQUARE + ANSI_CODES[DEFAULT] + SQUARE;
		output[1] = ANSI_CODES[ORANGE] + SQUARE + SQUARE + SQUARE + ANSI_CODES[DEFAULT] + SQUARE;
		break;
	case J:
		output[0] = ANSI_CODES[BLUE] + SQUARE + ANSI_CODES[DEFAULT] + SQUARE + SQUARE + SQUARE;
		output[1] = ANSI_CODES[BLUE] + SQUARE + SQUARE + SQUARE + ANSI_CODES[DEFAULT] + SQUARE;
		break;
	case S:
		output[0] = ANSI_CODES[DEFAULT] + SQUARE + ANSI_CODES[GREEN] + SQUARE + SQUARE + ANSI_CODES[DEFAULT] + SQUARE;
		output[1] = ANSI_CODES[GREEN] + SQUARE + SQUARE + ANSI_CODES[DEFAULT] + SQUARE + SQUARE;
		break;
	case Z:
		output[0] = ANSI_CODES[RED] + SQUARE + SQUARE + ANSI_CODES[DEFAULT] + SQUARE + SQUARE;
		output[1] = ANSI_CODES[DEFAULT] + SQUARE + ANSI_CODES[RED] + SQUARE + SQUARE + ANSI_CODES[DEFAULT] + SQUARE;
		break;
	case T:
		output[0] = ANSI_CODES[DEFAULT] + SQUARE + ANSI_CODES[PURPLE] + SQUARE + ANSI_CODES[DEFAULT] + SQUARE + SQUARE;
		output[1] = ANSI_CODES[PURPLE] + SQUARE + SQUARE + SQUARE + ANSI_CODES[DEFAULT] + SQUARE;
		break;
	default:
		output[0] = ANSI_CODES[DEFAULT] + SQUARE + SQUARE + SQUARE + SQUARE;
		output[1] = output[0];
		break;
	}
	cout << output[row] << ANSI_CODES[DEFAULT];
}

void drawBoard(const Block& fallingBlock, const Block& predictedBlock, const Board gameboard[HEIGHT][WIDTH], int nextBlock, int heldBlock, int score, int level) {
	//Återställer marköre och färg
	cout << ANSI_CODES[RESET_CURSOR] << ANSI_CODES[DEFAULT];
	//Skriver ut leveln
	for (int i = 0; i < 10; i++) cout << SQUARE;
	cout << "LEVEL: " << level << '\n';
	//Skiver ut övre ramen
	for (int i = 0; i < 6; i++) cout << SQUARE;
	cout << ANSI_CODES[GRAY];
	for (int i = 0; i < 12; i++) cout << SQUARE;
	cout << '\n';

	for (int i = 0; i < HEIGHT; i++) {

		cout << ANSI_CODES[DEFAULT] << SQUARE;
		if (i == 0) cout << "  HOLD  ";
		else if (i == 2) drawTetromino(heldBlock, 0);
		else if (i == 3) drawTetromino(heldBlock, 1);
		else drawTetromino(-1, 0);
		cout << ANSI_CODES[DEFAULT] << SQUARE;

		cout << ANSI_CODES[GRAY] << SQUARE << ANSI_CODES[DEFAULT];
		for (int j = 0; j < WIDTH; j++) {
			bool hasPrinted = false;
			for (auto position : fallingBlock.positions) {
				if (position.x == j && position.y == i) {
					cout << ANSI_CODES[fallingBlock.color] << SQUARE << ANSI_CODES[DEFAULT];
					hasPrinted = true;
					break;
				}
			}
			for (auto position : predictedBlock.positions) {
				if (position.x == j && position.y == i && !hasPrinted) {
					cout << ANSI_CODES[DARK_GRAY] << SQUARE << ANSI_CODES[DEFAULT];
					hasPrinted = true;
					break;
					ANSI_CODES[predictedBlock.color];
				}
			}
			if (hasPrinted) continue;
			int color = (gameboard[i][j].state == 0) ? DEFAULT : gameboard[i][j].color;
			cout << ANSI_CODES[color] << SQUARE;
		}
		cout << ANSI_CODES[GRAY] << SQUARE;

		cout << ANSI_CODES[DEFAULT] << SQUARE;
		if (i == 0) cout << "  NEXT  ";
		else if (i == 2) drawTetromino(nextBlock, 0);
		else if (i == 3) drawTetromino(nextBlock, 1);
		cout << ANSI_CODES[DEFAULT] << SQUARE << '\n';
	}
	for (int i = 0; i < 6; i++) cout << SQUARE;
	cout << ANSI_CODES[GRAY];
	for (int i = 0; i < 12; i++) cout << SQUARE;
	cout << ANSI_CODES[DEFAULT] << '\n';
	for (int i = 0; i < 10; i++) cout << SQUARE;
	cout << "SCORE: " << score << '\n';
}

void initializeBoard(Board gameboard[HEIGHT][WIDTH]) {
	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++)
			gameboard[i][j] = { 0,0 };
}

bool isCollision(const Block& fallingBlock, const Board gameboard[HEIGHT][WIDTH]) {
	for (auto position : fallingBlock.positions)
		if (gameboard[position.y][position.x].state == 1 || position.x < 0 || position.x >= WIDTH || position.y >= HEIGHT)
			return true;
	return false;
}

bool moveTetromino(Block& fallingBlock, int delta_x, int delta_y, Board gameboard[HEIGHT][WIDTH]) {
	Block newPosition;
	for (int i = 0; i < 4; i++) {
		newPosition.positions[i].x = fallingBlock.positions[i].x + delta_x;
		newPosition.positions[i].y = fallingBlock.positions[i].y + delta_y;
	}
	if (isCollision(newPosition, gameboard)) return false;

	for (int i = 0; i < 4; i++) fallingBlock.positions[i] = newPosition.positions[i];

	return true;
}

void rotateTetromino(Block& fallingBlock, Board gameboard[HEIGHT][WIDTH]) {
	Block newBlock;

	Position reference = fallingBlock.positions[0];

	for (int i = 0; i < 4; i++) {
		newBlock.positions[i].x = reference.x - (fallingBlock.positions[i].y - reference.y);
		newBlock.positions[i].y = reference.y + (fallingBlock.positions[i].x - reference.x);
	}

	int delta_y = 0;
	while (isCollision(newBlock, gameboard) && delta_y > -5) {
		int moveOrder[5] = { 0, 1, -1, 2, -2 };
		for (int i = 0; i < 5; i++) {
			if (moveTetromino(newBlock, moveOrder[i], delta_y, gameboard))
				break;
		}
		delta_y--;
	}

	if (delta_y != -5)
		for (int i = 0; i < 4; i++) fallingBlock.positions[i] = newBlock.positions[i];
}

void fillBag(vector<int>& bag) {
	while (bag.size() != 7) {
		int randomShape = rand() % 7;
		bool alreadyInBag = false;
		for (auto shape : bag)
			if (randomShape == shape) alreadyInBag = true;

		if (!alreadyInBag) bag.push_back(randomShape);
	}
}

void spawnNewBlock(Block& fallingBlock, vector<int>& bag, const Board gameboard[HEIGHT][WIDTH]) {
	char nextShape = bag[bag.size() - 1];
	bag.pop_back();
	switch (nextShape) {
	case O:
		fallingBlock.positions[0] = { 4, 0 };
		fallingBlock.positions[1] = { 5, 0 };
		fallingBlock.positions[2] = { 4, -1 };
		fallingBlock.positions[3] = { 5, -1 };
		fallingBlock.color = YELLOW;
		break;
	case I:
		fallingBlock.positions[0] = { 4, 0 };
		fallingBlock.positions[1] = { 3, 0 };
		fallingBlock.positions[2] = { 5, 0 };
		fallingBlock.positions[3] = { 6, 0 };
		fallingBlock.color = CYAN;
		break;
	case L:
		fallingBlock.positions[0] = { 4, 0 };
		fallingBlock.positions[1] = { 5, -1 };
		fallingBlock.positions[2] = { 3, 0 };
		fallingBlock.positions[3] = { 5, 0 };
		fallingBlock.color = ORANGE;
		break;
	case J:
		fallingBlock.positions[0] = { 5, 0 };
		fallingBlock.positions[1] = { 4, -1 };
		fallingBlock.positions[2] = { 6, 0 };
		fallingBlock.positions[3] = { 4, 0 };
		fallingBlock.color = BLUE;
		break;
	case S:
		fallingBlock.positions[0] = { 4, 0 };
		fallingBlock.positions[1] = { 3, 0 };
		fallingBlock.positions[2] = { 4, -1 };
		fallingBlock.positions[3] = { 5, -1 };
		fallingBlock.color = GREEN;
		break;
	case Z:
		fallingBlock.positions[0] = { 5, 0 };
		fallingBlock.positions[1] = { 6, 0 };
		fallingBlock.positions[2] = { 5, -1 };
		fallingBlock.positions[3] = { 4, -1 };
		fallingBlock.color = RED;
		break;
	case T:
		fallingBlock.positions[0] = { 4, 0 };
		fallingBlock.positions[1] = { 3, 0 };
		fallingBlock.positions[2] = { 5, 0 };
		fallingBlock.positions[3] = { 4, -1 };
		fallingBlock.color = PURPLE;
		break;
	}

	if (bag.empty()) fillBag(bag);
}

void holdBlock(const Block& fallingBlock, vector<int>& bag, int& heldBlock) {
	if (heldBlock != -1) bag.push_back(heldBlock);
	heldBlock = fallingBlock.color;
}

void placeTetromino(Block& fallingBlock, Board gameboard[HEIGHT][WIDTH], vector<int>& bag, bool& gameover, bool& hasHeldBlock) {
	for (int i = 0; i < 4; i++)
		gameboard[fallingBlock.positions[i].y][fallingBlock.positions[i].x] = { 1, fallingBlock.color };

	spawnNewBlock(fallingBlock, bag, gameboard);
	if (isCollision(fallingBlock, gameboard)) gameover = true;
	hasHeldBlock = false;
}

Block predictBlock(Block fallingBlock, Board gameboard[HEIGHT][WIDTH]) {
	while (moveTetromino(fallingBlock, 0, 1, gameboard));
	return fallingBlock;
}

void playerInputs(Block& fallingBlock, Board gameboard[HEIGHT][WIDTH], int& fallingDelay, vector<int>& bag, int& heldBlock, bool& hasHeldBlock, bool& gameover, int& score, chrono::steady_clock::time_point& lastTime) {
	int key = 0;
	if (_kbhit()) {
		key = _getch();
		switch (key) {
		case ARROW_UP:
			rotateTetromino(fallingBlock, gameboard);
			break;
		case ARROW_LEFT:
			moveTetromino(fallingBlock, -1, 0, gameboard);
			break;
		case ARROW_DOWN:
			Block tempBlock = fallingBlock;
			fallingDelay = (moveTetromino(tempBlock, 0, 1, gameboard)) ? 10 : fallingDelay;
			break;
		case ARROW_RIGHT:
			moveTetromino(fallingBlock, 1, 0, gameboard);
			break;
		case SPACEBAR:
			while (moveTetromino(fallingBlock, 0, 1, gameboard)) score += 2;
			placeTetromino(fallingBlock, gameboard, bag, gameover, hasHeldBlock);
			lastTime = chrono::steady_clock::now();
			break;
		case 'w':
		case 'W':
			if (!hasHeldBlock) {
				holdBlock(fallingBlock, bag, heldBlock);
				spawnNewBlock(fallingBlock, bag, gameboard);
				lastTime = chrono::steady_clock::now();
				hasHeldBlock = true;
			}
			break;
		}
	}
}

void clearedLinesAnimation(vector<int> fullLines) {
	for (int i = 0; i < 2; i++) {
		for (int lineIndex : fullLines) {
			cout << "\033[48;5;123m" << "\033[" << lineIndex + 3 << ";" << 15 << "H";
			for (int i = 0; i < 10; i++) cout << SQUARE;
		}
		Sleep(75);
		for (int lineIndex : fullLines) {
			cout << ANSI_CODES[DEFAULT] << "\033[" << lineIndex + 3 << ";" << 15 << "H";
			for (int i = 0; i < 10; i++) cout << SQUARE;
		}
		Sleep(75);
	}
}

void clearLines(Board gameboard[HEIGHT][WIDTH], int& score, int& level, int& linesCleared) {
	vector<int> fullLines;
	for (int y = 0; y < HEIGHT; y++) {
		bool isFullLine = true;
		for (int x = 0; x < WIDTH; x++)
			if (gameboard[y][x].state == 0) isFullLine = false;

		if (isFullLine) fullLines.push_back(y);
	}

	if (!fullLines.empty()) clearedLinesAnimation(fullLines);

	for (int lineIndex : fullLines)
		for (int y = lineIndex; y > 0; y--)
			for (int x = 0; x < WIDTH; x++)
				gameboard[y][x] = gameboard[y - 1][x];

	switch (fullLines.size()) {
	case 1:
		score += 100 * (level + 1);
		break;
	case 2:
		score += 300 * (level + 1);
		break;
	case 3:
		score += 500 * (level + 1);
		break;
	case 4:
		score += 800 * (level + 1);
		break;
	}

	linesCleared += fullLines.size();
	if (linesCleared > (level + 1) * 10) level++;
}

void tetris(int level) {
	system("cls");
	cout << ANSI_CODES[HIDE_CURSOR];
	Board gameboard[HEIGHT][WIDTH];
	initializeBoard(gameboard);

	Block fallingBlock, predictedBlock;

	vector<int> bag;
	fillBag(bag);
	spawnNewBlock(fallingBlock, bag, gameboard);

	bool gameover = false;
	int fallingDelay = 500;
	int heldBlock = -1;
	int score = 0, linesCleared = 0;
	auto lastTime = chrono::steady_clock::now();
	bool hasHeldBlock = false;

	while (!gameover) {
		fallingDelay = (level > 19) ? fallingDelays[19] : fallingDelays[level];
		playerInputs(fallingBlock, gameboard, fallingDelay, bag, heldBlock, hasHeldBlock, gameover, score, lastTime);

		if (chrono::steady_clock::now() - lastTime >= chrono::milliseconds(fallingDelay)) {
			if (!moveTetromino(fallingBlock, 0, 1, gameboard))
				placeTetromino(fallingBlock, gameboard, bag, gameover, hasHeldBlock);

			score += (fallingDelay == 10) ? 2 : 1;
			lastTime = chrono::steady_clock::now();
		}

		predictedBlock = predictBlock(fallingBlock, gameboard);
		drawBoard(fallingBlock, predictedBlock, gameboard, bag[bag.size() - 1], heldBlock, score, level);
		clearLines(gameboard, score, level, linesCleared);
		//Sleep(10);
	}
	cout << "GAMEOVER";
	Sleep(5000);
}

bool menu(int& startingLevel) {
	cout << ANSI_CODES[DEFAULT] << ANSI_CODES[SHOW_CURSOR];
	system("cls");
	cout << "Choose starting level(0-19): ";
	do cin >> startingLevel; while (!(startingLevel >= 0 && startingLevel <= 19));

	cout << "Press q/Q to quit or any other button to start";
	switch (_getch()) {
	case 'q':
	case 'Q':
		return false;
	default:
		return true;
	}
}

int main() {
	srand(time(0));
	int startingLevel;
	bool run = menu(startingLevel);
	while (run) {
		tetris(startingLevel);
		run = menu(startingLevel);
	}
	return 0;
}