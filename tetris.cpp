#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;

string ANSI_CODES[] {
	"\033[m",
	"\033[100m",
	"\033[48;5;234m",
	"\033[43m",
	"\033[46m",
	"\033[48;5;166m", //ORANGE
	"\033[44m",
	"\033[42m",
	"\033[41m",
	"\033[45m",
	"\033[H",
	"\033[?25l",
	"\033[?25h"

};

const int fallingDelays[] = { 1000, 900, 800, 700, 600, 500, 300, 200, 100 };

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
	DEFAULT,
	GRAY,
	DARK_GRAY,
	YELLOW,
	CYAN,
	ORANGE,
	BLUE,
	GREEN,
	RED,
	PURPLE,
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

void drawBoard(const Block &fallingBlock, const Block &predictedBlock, const Board gameboard[HEIGHT][WIDTH], int nextBlock, int heldBlock, int score) {
	cout << ANSI_CODES[RESET_CURSOR] << ANSI_CODES[GRAY];
	for (int i = 0; i < 12; i++) cout << SQUARE;
	cout << '\n';
	for (int i = 0; i < HEIGHT; i++) {
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
					cout << ANSI_CODES[DARK_GRAY] <<  SQUARE << ANSI_CODES[DEFAULT];
					hasPrinted = true;
					break;
					ANSI_CODES[predictedBlock.color];
				}
			}
			if (hasPrinted) continue;
			int color = (gameboard[i][j].state == 0) ? DEFAULT : gameboard[i][j].color;
			cout << ANSI_CODES[color] << SQUARE;
		}
		cout << ANSI_CODES[GRAY] << SQUARE << ANSI_CODES[DEFAULT] << '\n';
	}
	cout << ANSI_CODES[GRAY];
	for (int i = 0; i < 12; i++) cout << SQUARE;
	cout << ANSI_CODES[DEFAULT] << '\n';
}

void initializeBoard(Board gameboard[HEIGHT][WIDTH]) {
	for (int i = 0; i < HEIGHT; i++) 
		for (int j = 0; j < WIDTH; j++) 
			gameboard[i][j] = {0,0};
}

bool isCollision(const Block& fallingBlock, const Board gameboard[HEIGHT][WIDTH]) {
	for (auto position : fallingBlock.positions)
		if (gameboard[position.y][position.x].state == 1 || position.x < 0 || position.x >= WIDTH || position.y >= HEIGHT)
			return true;
	return false;
}

bool moveTetromino(Block &fallingBlock, int delta_x, int delta_y, Board gameboard[HEIGHT][WIDTH]) {
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
	Position newPositions[4];

	Position reference = fallingBlock.positions[0];

	for (int i = 0; i < 4; i++) {
		newPositions[i].x = reference.x - (fallingBlock.positions[i].y - reference.y);
		newPositions[i].y = reference.y + (fallingBlock.positions[i].x - reference.x);
	}

	//Kolla om giltig rotation

	for (int i = 0; i < 4; i++) {
		fallingBlock.positions[i] = newPositions[i];
	}
}


void placeTetromino(Block fallingBlock, Board gameboard[HEIGHT][WIDTH]) {
	for (int i = 0; i < 4; i++)
		gameboard[fallingBlock.positions[i].y][fallingBlock.positions[i].x] = { 1, fallingBlock.color };
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

bool spawnNewBlock(Block& fallingBlock, vector<int>& bag, const Board gameboard[HEIGHT][WIDTH]) {
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

	return isCollision(fallingBlock, gameboard);
}

void playerInputs(Block &fallingBlock, Board gameboard[HEIGHT][WIDTH], int &fallingDelay, vector<int> &bag, bool &gameover) {
	if (_kbhit()) {
		switch (_getch()) {
			case ARROW_UP:
				rotateTetromino(fallingBlock, gameboard);
				break;
			case ARROW_LEFT:
				moveTetromino(fallingBlock, -1, 0, gameboard);
				break;
			case ARROW_DOWN:
				fallingDelay = 10;
				break;
			case ARROW_RIGHT:
				moveTetromino(fallingBlock, 1, 0, gameboard);
				break;
			case SPACEBAR:
				while (moveTetromino(fallingBlock, 0, 1, gameboard));
				placeTetromino(fallingBlock, gameboard);
				gameover = spawnNewBlock(fallingBlock, bag, gameboard);
				break;
		}
	}
}

void clearLines(Board gameboard[HEIGHT][WIDTH]) {
	vector<int> fullLines;
	for (int y = 0; y < HEIGHT; y++) {
		bool isFullLine = true;
		
		for (int x = 0; x < WIDTH; x++)
			if (gameboard[y][x].state == 0) isFullLine = false;
		
		if (isFullLine) fullLines.push_back(y);
	}

	for (auto lineIndex : fullLines)
		for (int y = lineIndex; y > 0; y--)
			for (int x = 0; x < WIDTH; x++)
				gameboard[y][x] = gameboard[y - 1][x];

}

Block predictBlock(Block fallingBlock, Board gameboard[HEIGHT][WIDTH]) {
	while (moveTetromino(fallingBlock, 0, 1, gameboard));
	return fallingBlock;
}

void tetris() {
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
	int score = 0, level = 0;
	auto lastTime = chrono::steady_clock::now();
	
	while (!gameover) {
		fallingDelay = fallingDelays[level];

		playerInputs(fallingBlock, gameboard, fallingDelay, bag, gameover);

		predictedBlock = predictBlock(fallingBlock, gameboard);

		if (chrono::steady_clock::now() - lastTime >= chrono::milliseconds(fallingDelay)) {
			if (!moveTetromino(fallingBlock, 0, 1, gameboard)) {
				placeTetromino(fallingBlock, gameboard);
				gameover = spawnNewBlock(fallingBlock, bag, gameboard);
			}
			lastTime = chrono::steady_clock::now();
		}

		clearLines(gameboard);
		drawBoard(fallingBlock, predictedBlock, gameboard, bag[bag.size() - 1], heldBlock, score);
		
		
	}
	

	
}

bool menu() {
	cout << ANSI_CODES[DEFAULT] << ANSI_CODES[SHOW_CURSOR];
	system("cls");
	cout << "Press q/Q to quit or any other button to start";
	switch(_getch()) {
		case 'q':
		case 'Q':
			return false;
		default:
			return true;
	}
}

int main(){
	srand(time(0));
	bool run = menu();
	while (run) {
		tetris();
		run = menu();
	}
	return 0;
}