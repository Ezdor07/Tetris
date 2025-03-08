#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <fstream>
#include <string>

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

struct GameStatistics {
	Block fallingBlock;
	Board gameboard[HEIGHT][WIDTH];
	vector <int> bag;
	int score;
	int level;
	int linesCleared;
	int heldBlock;
};

const string SQUARE = "  ";

void readLeaderboard(vector <int>& scores) {
	ifstream leaderboard("leaderboard.txt");
	if (!leaderboard) {
		cout << "Could not read the file";
		return;
	}
	string scoreStr;
	while (getline(leaderboard, scoreStr)) {
		scores.push_back(stoi(scoreStr));
	}
	leaderboard.close();
	for (int i = 0; i < scores.size(); i++) {
		cout << i + 1 << ". " << scores[i] << '\n';
	}
}

void writeHighscore(int gameScore, vector<int> scores) {
	if (gameScore < scores[scores.size() - 1]) {
		cout << "You did not make the leaderboard";
		return;
	} else cout << "You made the leaderboard\n";
	
	ofstream leaderboard("leaderboard.txt");


	if (!leaderboard) {
		cout << "Lyckades inte skriva till filen";
		return;
	}
	vector <int> newLeaderboard;
	bool hasEntered = false;
	for (int i = 0; i < scores.size(); i++) {
		if (gameScore > scores[i] && !hasEntered) {
			newLeaderboard.push_back(gameScore);
			hasEntered = true;
			i--;
		}
		else {
			newLeaderboard.push_back(scores[i]);
		}
	}
	if (newLeaderboard.size() > 10) newLeaderboard.pop_back();
	for (int score : newLeaderboard) {
		leaderboard << score << endl;
	}

	leaderboard.close();
}

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

void drawBoard(const GameStatistics& game, const Block& predictedBlock, int nextBlock) {
	//Återställer marköre och färg
	cout << ANSI_CODES[RESET_CURSOR] << ANSI_CODES[DEFAULT];
	//Skriver ut leveln
	for (int i = 0; i < 10; i++) cout << SQUARE;
	cout << "LEVEL: " << game.level << '\n';
	//Skiver ut övre ramen
	for (int i = 0; i < 6; i++) cout << SQUARE;
	cout << ANSI_CODES[GRAY];
	for (int i = 0; i < 12; i++) cout << SQUARE;
	cout << '\n';

	for (int i = 0; i < HEIGHT; i++) {

		cout << ANSI_CODES[DEFAULT] << SQUARE;
		if (i == 0) cout << "  HOLD  ";
		else if (i == 2) drawTetromino(game.heldBlock, 0);
		else if (i == 3) drawTetromino(game.heldBlock, 1);
		else drawTetromino(-1, 0);
		cout << ANSI_CODES[DEFAULT] << SQUARE;

		cout << ANSI_CODES[GRAY] << SQUARE << ANSI_CODES[DEFAULT];
		for (int j = 0; j < WIDTH; j++) {
			bool hasPrinted = false;
			for (auto position : game.fallingBlock.positions) {
				if (position.x == j && position.y == i) {
					cout << ANSI_CODES[game.fallingBlock.color] << SQUARE << ANSI_CODES[DEFAULT];
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
			int color = (game.gameboard[i][j].state == 0) ? DEFAULT : game.gameboard[i][j].color;
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
	cout << "SCORE: " << game.score << '\n';
}

void initializeBoard(Board gameboard[HEIGHT][WIDTH]) {
	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++)
			gameboard[i][j] = { 0,0 };
}

bool isCollision(const Block& tetromino, const Board gameboard[HEIGHT][WIDTH]) {
	for (auto position : tetromino.positions)
		if (gameboard[position.y][position.x].state == 1 || position.x < 0 || position.x >= WIDTH || position.y >= HEIGHT)
			return true;
	return false;
}

bool moveTetromino(Block& tetromino, int delta_x, int delta_y, Board gameboard[HEIGHT][WIDTH]) {
	Block newPosition;
	for (int i = 0; i < 4; i++) {
		newPosition.positions[i].x = tetromino.positions[i].x + delta_x;
		newPosition.positions[i].y = tetromino.positions[i].y + delta_y;
	}
	if (isCollision(newPosition, gameboard)) return false;

	for (int i = 0; i < 4; i++) tetromino.positions[i] = newPosition.positions[i];

	return true;
}

void rotateTetromino(GameStatistics& game) {
	if (game.fallingBlock.color == YELLOW) return;
	Block newBlock;

	Position reference = game.fallingBlock.positions[0];

	for (int i = 0; i < 4; i++) {
		newBlock.positions[i].x = reference.x - (game.fallingBlock.positions[i].y - reference.y);
		newBlock.positions[i].y = reference.y + (game.fallingBlock.positions[i].x - reference.x);
	}

	int delta_y = 0;
	while (isCollision(newBlock, game.gameboard) && delta_y > -5) {
		int moveOrder[5] = { 0, 1, -1, 2, -2 };
		for (int i = 0; i < 5; i++) {
			if (moveTetromino(newBlock, moveOrder[i], delta_y, game.gameboard))
				break;
		}
		delta_y--;
	}

	if (delta_y != -5)
		for (int i = 0; i < 4; i++) game.fallingBlock.positions[i] = newBlock.positions[i];
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

void spawnNewBlock(GameStatistics& game) {
	char nextShape = game.bag[game.bag.size() - 1];
	game.bag.pop_back();
	switch (nextShape) {
	case O:
		game.fallingBlock.positions[0] = { 4, 0 };
		game.fallingBlock.positions[1] = { 5, 0 };
		game.fallingBlock.positions[2] = { 4, -1 };
		game.fallingBlock.positions[3] = { 5, -1 };
		game.fallingBlock.color = YELLOW;
		break;
	case I:
		game.fallingBlock.positions[0] = { 4, 0 };
		game.fallingBlock.positions[1] = { 3, 0 };
		game.fallingBlock.positions[2] = { 5, 0 };
		game.fallingBlock.positions[3] = { 6, 0 };
		game.fallingBlock.color = CYAN;
		break;
	case L:
		game.fallingBlock.positions[0] = { 4, 0 };
		game.fallingBlock.positions[1] = { 5, -1 };
		game.fallingBlock.positions[2] = { 3, 0 };
		game.fallingBlock.positions[3] = { 5, 0 };
		game.fallingBlock.color = ORANGE;
		break;
	case J:
		game.fallingBlock.positions[0] = { 5, 0 };
		game.fallingBlock.positions[1] = { 4, -1 };
		game.fallingBlock.positions[2] = { 6, 0 };
		game.fallingBlock.positions[3] = { 4, 0 };
		game.fallingBlock.color = BLUE;
		break;
	case S:
		game.fallingBlock.positions[0] = { 4, 0 };
		game.fallingBlock.positions[1] = { 3, 0 };
		game.fallingBlock.positions[2] = { 4, -1 };
		game.fallingBlock.positions[3] = { 5, -1 };
		game.fallingBlock.color = GREEN;
		break;
	case Z:
		game.fallingBlock.positions[0] = { 5, 0 };
		game.fallingBlock.positions[1] = { 6, 0 };
		game.fallingBlock.positions[2] = { 5, -1 };
		game.fallingBlock.positions[3] = { 4, -1 };
		game.fallingBlock.color = RED;
		break;
	case T:
		game.fallingBlock.positions[0] = { 4, 0 };
		game.fallingBlock.positions[1] = { 3, 0 };
		game.fallingBlock.positions[2] = { 5, 0 };
		game.fallingBlock.positions[3] = { 4, -1 };
		game.fallingBlock.color = PURPLE;
		break;
	}

	if (game.bag.empty()) fillBag(game.bag);
}

void holdBlock(GameStatistics& game) {
	if (game.heldBlock != -1) game.bag.push_back(game.heldBlock);
	game.heldBlock = game.fallingBlock.color;
}

void placeTetromino(GameStatistics& game, bool& gameover, bool&hasHeldBlock) {
	for (int i = 0; i < 4; i++)
		game.gameboard[game.fallingBlock.positions[i].y][game.fallingBlock.positions[i].x] = { 1, game.fallingBlock.color };

	spawnNewBlock(game);
	if (isCollision(game.fallingBlock, game.gameboard)) gameover = true;
	hasHeldBlock = false;
}

Block predictBlock(GameStatistics game) {
	while (moveTetromino(game.fallingBlock, 0, 1, game.gameboard));
	return game.fallingBlock;
}

void saveGame(GameStatistics game) {
	game.bag.push_back(game.fallingBlock.color);

	ofstream savedBag("bag.txt");
	for (auto shape : game.bag) savedBag << shape;
	savedBag.close();

	ofstream savedLevel("level.txt");
	savedLevel <<game.level;
	savedLevel.close();

	ofstream savedScore("score.txt");
	savedScore << game.score;
	savedScore.close();

	ofstream savedHeldBlock("held block.txt");
	savedHeldBlock << game.heldBlock;
	savedHeldBlock.close();

	ofstream savedLinesCleared("lines cleared.txt");
	savedLinesCleared << game.linesCleared;
	savedLinesCleared.close();

	ofstream savedGameboardState("gameboard state.txt");
	ofstream savedGameboardColor("gameboard color.txt");
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			savedGameboardState << game.gameboard[i][j].state;
			savedGameboardColor << game.gameboard[i][j].color;
		}
		savedGameboardState << endl;
		savedGameboardColor << endl;
	}
	savedGameboardState.close();
	savedGameboardColor.close();
	system("cls");
	cout << "SAVED";
	Sleep(1000);
}

void pauseMenu(bool &quit, GameStatistics game) {
	bool redo;
	do {
		system("cls");
		cout << "\033[10;22H" << "PAUSED";
		cout << "\033[11;19HCONTINUE[1]\033[12;19HSAVE GAME[2]\033[13;19HQUIT[3]";
		redo = false;
		switch (_getch()) {
		case '1':
			return;
		case '2':
			saveGame(game);
			redo = true;
			break;
		case '3':
			quit = true;
			return;
		default:
			redo = true;
			break;
		}
	} while (redo);
	system("cls");
}

void playerInputs(GameStatistics& game, int& fallingDelay, bool& hasHeldBlock, bool& gameover, chrono::steady_clock::time_point& lastTime, bool& quit) {
	int key = 0;
	if (_kbhit()) {
		key = _getch();
		switch (key) {
		case ARROW_UP:
			rotateTetromino(game);
			break;
		case ARROW_LEFT:
			moveTetromino(game.fallingBlock, -1, 0, game.gameboard);
			break;
		case ARROW_DOWN:
			Block tempBlock = game.fallingBlock;
			fallingDelay = (moveTetromino(game.fallingBlock, 0, 1, game.gameboard)) ? 10 : fallingDelay;
			break;
		case ARROW_RIGHT:
			moveTetromino(game.fallingBlock, 1, 0, game.gameboard);
			break;
		case SPACEBAR:
			while (moveTetromino(game.fallingBlock, 0, 1, game.gameboard)) game.score += 2;
			placeTetromino(game, gameover, hasHeldBlock);
			lastTime = chrono::steady_clock::now();
			break;
		case 'w':
		case 'W':
			if (!hasHeldBlock) {
				holdBlock(game);
				spawnNewBlock(game);
				lastTime = chrono::steady_clock::now();
				hasHeldBlock = true;
			}
			break;
		case 'p':
			pauseMenu(quit, game);
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

void clearLines(GameStatistics& game) {
	vector<int> fullLines;
	for (int y = 0; y < HEIGHT; y++) {
		bool isFullLine = true;
		for (int x = 0; x < WIDTH; x++)
			if (game.gameboard[y][x].state == 0) isFullLine = false;

		if (isFullLine) fullLines.push_back(y);
	}

	if (!fullLines.empty()) clearedLinesAnimation(fullLines);

	for (int lineIndex : fullLines)
		for (int y = lineIndex; y > 0; y--)
			for (int x = 0; x < WIDTH; x++)
				game.gameboard[y][x] = game.gameboard[y - 1][x];

	switch (fullLines.size()) {
	case 1:
		game.score += 100 * (game.level + 1);
		break;
	case 2:
		game.score += 300 * (game.level + 1);
		break;
	case 3:
		game.score += 500 * (game.level + 1);
		break;
	case 4:
		game.score += 800 * (game.level + 1);
		break;
	}

	game.linesCleared += fullLines.size();
	if (game.linesCleared > (game.level + 1) * 10) game.level++;
}

void tetris(GameStatistics game, bool gameLoaded, bool& gameover) {
	system("cls");
	cout << ANSI_CODES[HIDE_CURSOR];
	if (!gameLoaded) {
		initializeBoard(game.gameboard);
		fillBag(game.bag);
		game.heldBlock = -1;
		game.score = 0;
		game.linesCleared = 0;
	}

	Block predictedBlock;
	spawnNewBlock(game);

	gameover = false;
	bool quit = false;
	int fallingDelay;
	auto lastTime = chrono::steady_clock::now();
	bool hasHeldBlock = false;

	while (!gameover && !quit) {
		fallingDelay = (game.level > 19) ? fallingDelays[19] : fallingDelays[game.level];
		playerInputs(game, fallingDelay, hasHeldBlock, gameover, lastTime, quit);

		if (chrono::steady_clock::now() - lastTime >= chrono::milliseconds(fallingDelay)) {
			if (!moveTetromino(game.fallingBlock, 0, 1, game.gameboard))
				placeTetromino(game, gameover, hasHeldBlock);

			game.score += (fallingDelay == 10) ? 2 : 1;
			lastTime = chrono::steady_clock::now();
		}

		predictedBlock = predictBlock(game);
		drawBoard(game, predictedBlock, game.bag[game.bag.size() - 1]);
		clearLines(game);
	}
	if (quit) return;

	cout << "GAMEOVER";
	Sleep(3000);
}

void loadGame(GameStatistics& newGame) {
	ifstream gameboardState("gameboard state.txt");
	ifstream gameboardColor("gameboard color.txt");
	int rowIndex = 0;
	string row;
	while (getline(gameboardState, row)) {
		int columnIndex = 0;
		for (char number : row) newGame.gameboard[rowIndex][columnIndex++].state = number - '0';
		rowIndex++;
	}

	rowIndex = 0;
	while (getline(gameboardColor, row)) {
		int columnIndex = 0;
		for (char number : row) newGame.gameboard[rowIndex][columnIndex++].color = number - '0';
		rowIndex++;
	}
	gameboardState.close();
	gameboardColor.close();

	ifstream savedScore("score.txt");
	savedScore >> newGame.score;
	savedScore.close();

	ifstream savedLevel("level.txt");
	savedLevel >> newGame.level;
	savedLevel.close();

	ifstream savedHeldBlock("held block.txt");
	savedHeldBlock >> newGame.heldBlock;
	savedScore.close();

	ifstream savedLinesCleared("lines cleared.txt");
	savedLinesCleared >> newGame.linesCleared;
	savedLinesCleared.close();

	ifstream savedBag("bag.txt");
	string numbers;
	getline(savedBag, numbers);
	for (char number : numbers) {
		newGame.bag.push_back(number - '0');
	}
}

bool startMenu(GameStatistics& newGame, bool& gameLoaded, vector<int>& leaderboard) {
	cout << ANSI_CODES[DEFAULT] << ANSI_CODES[SHOW_CURSOR];
	system("cls");
	cout << "LEADERBOARD\n";
	readLeaderboard(leaderboard);
	cout << "[1]Start New Game\n[2]Load Game\n[3]QUIT";
	bool redo;
	do {
		redo = false;
		switch (_getch()) {
		case '1':
			cout << "\nChoose starting level(0-19): ";
			do cin >> newGame.level; while (!(newGame.level >= 0 && newGame.level <= 19));
			gameLoaded = false;
			return true;
		case '2':
			loadGame(newGame);
			gameLoaded = true;
			return true;
		case '3':
			return false;
		default:
			redo = true;
			break;
		}
	} while (redo);
	return false;
}

int main() {
	srand(time(0));
	bool run = true;
	while (run) {
		GameStatistics newGame;
		vector <int> leaderboard;
		bool gameLoaded, gameover = false;
		
		run = startMenu(newGame, gameLoaded, leaderboard);
		
		if(run) tetris(newGame, gameLoaded, gameover);
		if(gameover) writeHighscore(newGame.score, leaderboard);
	}
	return 0;
}