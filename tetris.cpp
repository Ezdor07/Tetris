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

//Structs och konstanter för att göra koden mer läsvänlig

const string ANSI_CODES[]{
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

//Funktion för att läsa leaderboard från txt fil
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

//Funktion för att skriva highscore till leaderboard
void writeHighscore(int gameScore, vector<int> scores) {
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

//Ritar vald rad av en tetromino. För hold och next block
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

//Ritar upp spelplanen
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
			cout << (game.gameboard[i][j].state == 0 ? ANSI_CODES[DEFAULT] : ANSI_CODES[game.gameboard[i][j].color]) << SQUARE;
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

//Tömmer spelbrädet
void initializeBoard(Board gameboard[HEIGHT][WIDTH]) {
	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++)
			gameboard[i][j] = { 0,0 };
}

//Kollar om ett block ligger i ett annat eller utanför planen
bool isCollision(const Block& tetromino, const Board gameboard[HEIGHT][WIDTH]) {
	for (auto position : tetromino.positions)
		if (position.x >= 0 && position.x <= WIDTH - 1 && position.y >= 0 && position.y <= HEIGHT - 1) {
			if (gameboard[position.y][position.x].state == 1)
				return true;
		}
		else if (position.x < 0 || position.x >= WIDTH || position.y >= HEIGHT) {
			return true;
		}
	return false;
}

//Flyttar en tetromino åt olika håll
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

//Roterar tetromino medurs + flyttar den om den ligger på ogiltig plats
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

//Fyller bag med de 7 formerna i random ordning
void fillBag(vector<int>& bag) {
	while (bag.size() != 7) {
		int randomShape = rand() % 7;
		bool alreadyInBag = false;
		for (auto shape : bag)
			if (randomShape == shape) alreadyInBag = true;

		if (!alreadyInBag) bag.push_back(randomShape);
	}
}

//Ger fallingBlock startposition och random form/färg
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

//Byter ut det fallande blocket mot det som hålls och tvär om
void holdBlock(GameStatistics& game) {
	if (game.heldBlock != -1) game.bag.push_back(game.heldBlock);
	game.heldBlock = game.fallingBlock.color;
}

//Lägger det fallande blocket på brädet
void placeTetromino(GameStatistics& game, bool& gameover, bool& hasHeldBlock) {
	for (int i = 0; i < 4; i++)
		game.gameboard[game.fallingBlock.positions[i].y][game.fallingBlock.positions[i].x] = { 1, game.fallingBlock.color };

	spawnNewBlock(game);
	if (isCollision(game.fallingBlock, game.gameboard)) gameover = true;
	hasHeldBlock = false;
}

//Förutser var det fallande blocket kommer hamna och skickar tillbaka det
Block predictBlock(GameStatistics game) {
	while (moveTetromino(game.fallingBlock, 0, 1, game.gameboard));
	return game.fallingBlock;
}

//Sparar olika värden i txt filer för att kunna fortsätta på samma ställe nästa gång
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
	cout << "\033[8;22HSAVED";
	Sleep(1000);
	cout << "\033[8;22H     ";
}

//Meny när man pausar med olika menyval
void pauseMenu(bool &quit, GameStatistics game) {
	bool redo;
	//Loopar tills användaren lämnar menyn
	do {
		//Skriver ut menyvalen
		cout << "\033[10;22H" << "PAUSED";
		cout << "\033[11;19H[1]CONTINUE\033[12;19H[2]SAVE GAME\033[13;21H[3]QUIT";
		redo = false;
		switch (_getch()) {
		case '1': //Gå tillbaka till spelet
			return;
		case '2': //Sparar spelet och loopar igen
			saveGame(game);
			redo = true;
			break;
		case '3': //Avslutar spelet
			quit = true;
			return;
		default: //Loopar igen
			redo = true;
			break;
		}
	} while (redo);
	//Rensar skärmen
	system("cls");
}

//Hanterar spelarens inputs
void playerInputs(GameStatistics& game, int& fallingDelay, bool& hasHeldBlock, bool& gameover, chrono::steady_clock::time_point& lastTime, bool& quit) {
	//int key = 0;
	//Kollar om ett knapp blivit nertryckt
	if (_kbhit()) {
		//key = _getch();
		//Kollar vilken knapp det är
		switch (_getch()) {
		case ARROW_UP: //Om pil upp rotera medurs
			rotateTetromino(game);
			break;
		case ARROW_LEFT: //Om vänster flytta fallande block vänster
			moveTetromino(game.fallingBlock, -1, 0, game.gameboard);
			break;
		case ARROW_DOWN: //Om ner minska fallDelay till 10 ms
			//Kollar om den träffar marken nästa steg. 
			// I så fall ökas inte hastigheten så att man kan flytta/rotera blocket efter att det kommit till botten
			Block tempBlock = game.fallingBlock;
			fallingDelay = (moveTetromino(game.fallingBlock, 0, 1, game.gameboard)) ? 10 : fallingDelay;
			break;
		case ARROW_RIGHT: //Om pil höger flytta fallande block höger
			moveTetromino(game.fallingBlock, 1, 0, game.gameboard);
			break;
		case SPACEBAR: //Om mellanslag skickas blocket ner hela vägen direkt
			//Flytta så länge som det är möjligt och öka score med 2 för varje steg den flyttas
			while (moveTetromino(game.fallingBlock, 0, 1, game.gameboard)) game.score += 2;
			//Lägg blocket på planen
			placeTetromino(game, gameover, hasHeldBlock);
			//Ändra lastTime till nu så att det nya blocket fallet efter fallingDelay igen
			lastTime = chrono::steady_clock::now();
			break;
		case 'w':
		case 'W': //Om w/W Håll kvar blocket till senare
			//Kolla så att ett block inte redan hållits efter senaste placeringen på brädet
			if (!hasHeldBlock) {
				//byt plats på det sedan tidigare hållda med det fallande blocket
				holdBlock(game);
				spawnNewBlock(game);
				//Ändra lastTime till nu så att det nya blocket fallet efter fallingDelay igen
				lastTime = chrono::steady_clock::now();
				//Sätter att block har hållts denna "omgången"
				hasHeldBlock = true;
			}
			break;
		case 'p': //Om p så pausas spelet
			pauseMenu(quit, game);
		}
	}
}

//Kort animation när en rad är rensad
void clearedLinesAnimation(vector<int> fullLines) {
	//Blinkar 2 ggr
	for (int i = 0; i < 2; i++) {
		//loopar genom varje rad
		for (int lineIndex : fullLines) {
			//Skriver ut en ljusblå remsa över raden
			cout << "\033[48;5;123m" << "\033[" << lineIndex + 3 << ";" << 15 << "H";
			for (int i = 0; i < 10; i++) cout << SQUARE;
		}
		//Vänta 75 ms och ta bort den ljusblå remsan
		Sleep(75);
		for (int lineIndex : fullLines) {
			cout << ANSI_CODES[DEFAULT] << "\033[" << lineIndex + 3 << ";" << 15 << "H";
			for (int i = 0; i < 10; i++) cout << SQUARE;
		}
		//Vänta 75 ms innan den repeterar
		Sleep(75);
	}
}

//Tar bort rader som är fulla
void clearLines(GameStatistics& game) {
	//vector som sparar index för de rader som är fulla
	vector<int> fullLines;
	//Loopar genom brädet
	for (int y = 0; y < HEIGHT; y++) {
		bool isFullLine = true;
		for (int x = 0; x < WIDTH; x++)
			//Om någon ruta i en rad är 0(tom) så är raden inte full
			if (game.gameboard[y][x].state == 0) isFullLine = false;

		//Om raden är full sparas indexet i vectorn fullLines
		if (isFullLine) fullLines.push_back(y);
	}

	//Om fullLines inte är tom görs det en animation för att visa vilka rader som togs bort
	if (!fullLines.empty()) clearedLinesAnimation(fullLines);

	//Loopar genom alla rader som är fulla
	for (int lineIndex : fullLines)
		//Loopar från full rad till toppen
		for (int y = lineIndex; y > 0; y--)
			//Flyttar raden ovenför varje rad neråt
			for (int x = 0; x < WIDTH; x++)
				game.gameboard[y][x] = game.gameboard[y - 1][x];

	//Kollar hur många rader som togs bort och ökar score beroende på hur många och vilket level det är
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
	//Ökar totala lines cleared
	game.linesCleared += fullLines.size();
	//Om linesCleared är mer än 10x leveln + 1, så ökar leveln
	if (game.linesCleared > (game.level + 1) * 10) game.level++;
}

//Spelfunktionen
void tetris(GameStatistics& game, bool gameLoaded, bool& gameover) {
	//Rensar skärmen och gömmer musmarkören
	system("cls");
	cout << ANSI_CODES[HIDE_CURSOR];
	//Om ett spel inte redan blivit loadat ska startvärden sättar.
	if (!gameLoaded) {
		//Sätter brädet state och färg till 0
		initializeBoard(game.gameboard);
		//Fyller bag med random former
		fillBag(game.bag);
		//Sätter att ingen block hålls
		game.heldBlock = -1;
		//Score och linescleared till 0;
		game.score = 0;
		game.linesCleared = 0;
	}

	//Standard variabler oavsett om spel är loadat eller inte
	Block predictedBlock;
	bool quit = false;
	int fallingDelay;
	bool hasHeldBlock = false;
	auto lastTime = chrono::steady_clock::now();

	//Skapar ett nytt block som börjar uppifrån
	spawnNewBlock(game);
	//Spelloop som körs så länge som det inte är gameover eller spelaren har valt quit från menyn
	while (!gameover && !quit) {
		//FallingDelay sätts beroende på level från en lista. Alla levlar mer än 19 har samma hastighet
		fallingDelay = (game.level > 19) ? fallingDelays[19] : fallingDelays[game.level];
		//Kollar inputs från spelaren för att flytta blocket bland annat
		playerInputs(game, fallingDelay, hasHeldBlock, gameover, lastTime, quit);

		//Om det gått "fallingDelay" tid sedan senaste gången blocket föll
		if (chrono::steady_clock::now() - lastTime >= chrono::milliseconds(fallingDelay)) {
			//Flytta blocket neråt
			if (!moveTetromino(game.fallingBlock, 0, 1, game.gameboard))
				//Om det inte går att flytta ska blocket läggas på brädet
				placeTetromino(game, gameover, hasHeldBlock);
			//Gamescore ökar med 1 om det föll naturligt eller 2 om spelaren fast droppade det manuellt
			game.score += (fallingDelay == 10) ? 2 : 1;
			//Sätter att senast blocket föll var nu
			lastTime = chrono::steady_clock::now();
		}
		//Skapar ett förutspått shadow block som visar var det fallande blocket hamnar om man skickar ner det
		predictedBlock = predictBlock(game);
		//Ritar upp brädet
		drawBoard(game, predictedBlock, game.bag[game.bag.size() - 1]);
		//Tar bort lines om det finns några fulla
		clearLines(game);
	}
	//När spelet är över kollar den om det är över pga gameover eller quit
	if (quit) return; //Om quit kommer man till meny direkt
	cout << "GAMEOVER"; // Annars skrivs gameover ut i 3 sekunder
	Sleep(3000);
}

//Ändrar spelvärden till det sparade spelets värden
void loadGame(GameStatistics& newGame) {
	//Öppnar txt filerna för gameboard state och color för läsning
	ifstream gameboardState("gameboard state.txt");
	ifstream gameboardColor("gameboard color.txt");
	int rowIndex = 0;
	string row;
	//För varje rad i filen hämtas varje karaktär i raden som översätts till siffra och tilldelas gameboard
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
	//Stänger filerna
	gameboardState.close();
	gameboardColor.close();

	//Öppnar sparaden filerna för score, level, heldBlock, linesCleared och tilldelar värdena. Stänger filerna
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

	//läser raden och tar varje siffra i raden och lägger i bagen
	ifstream savedBag("bag.txt");
	string numbers;
	getline(savedBag, numbers);
	for (char number : numbers) {
		newGame.bag.push_back(number - '0');
	}
}

//Startmeny med leaderboard och olika val
bool startMenu(GameStatistics& newGame, bool& gameLoaded, vector<int>& leaderboard) {
	//Återställer text färger och visar musmarkör
	cout << ANSI_CODES[DEFAULT] << ANSI_CODES[SHOW_CURSOR];
	system("cls");
	//Visar leaderboarden
	cout << "LEADERBOARD\n";
	readLeaderboard(leaderboard);
	//Skriver ut valen användaren har
	cout << "[1]Start New Game\n[2]Load Game\n[3]QUIT";
	bool redo;
	//Så länge användaren skriver fel input loopas det
	do {
		redo = false;
		switch (_getch()) {
		case '1': //Start new game
			//Ber användaren välja startlevel
			cout << "\nChoose starting level(0-19): ";
			//Loopar tills användare valj giltig level att starta på
			do {
				cin >> newGame.level;
				if (!(newGame.level >= 0 && newGame.level <= 19)) cout << "Try again!\n";
			} while (!(newGame.level >= 0 && newGame.level <= 19));
			//Sätter gameloaded till false eftersom det är nytt spel
			gameLoaded = false;
			return true;
		case '2':
			//Initialiserar startvärden till de sparade spelet värden
			loadGame(newGame);
			//Sätter att game blivit loadat
			gameLoaded = true;
			return true;
		case '3': //QUIT
			//Returnerar så run blir false
			return false;
		default:
			//Loopar igen
			redo = true;
			break;
		}
	} while (redo);
	return false;
}

int main() {
	srand(time(0));
	bool run = true;
	//Main loop som gör att man kommer tillbaka till meny när spelet är över
	while (run) {
		GameStatistics newGame;
		vector <int> leaderboard;
		bool gameLoaded, gameover = false;
		
		//Går till startmenyn för att välja hur man vill starta spelet
		run = startMenu(newGame, gameLoaded, leaderboard);
		
		//Om användaren inte valt quit startar spelet med startvärden om användaren valt
		if(run) tetris(newGame, gameLoaded, gameover);
		//Om det blivit gameover(inte valt quit själv) ska scoret läggas in på leaderboard om det är bättre än 10an
		if(gameover && newGame.score > leaderboard[9]) writeHighscore(newGame.score, leaderboard);
	}
	return 0;
}