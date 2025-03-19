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
#include <locale>

using namespace std;

//Structs och konstanter för att göra koden mer läsvänlig

const string ANSI_CODES[] = {
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
	"\033[47m",
	"\033[30m",
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
	WHITE,
	BLACK_TEXT,
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
	ENTER = 13
};

struct Board {
	int state = 0;
	int color = DEFAULT;
};

struct Position {
	int x = 0;
	int y = 0;
};

struct Block {
	Position positions[4] = {};
	int color = DEFAULT;
};

struct GameStatistics {
	Block fallingBlock = {};
	Board gameboard[HEIGHT][WIDTH] = {};
	vector <int> bag = {};
	int score = 0;
	int level = 0;
	int linesCleared = 0;
	int heldBlock = -1;
};

struct Leaderboard {
	int score = 0;
	string name = "";
};

const string SQUARE = "  ";

//Funktion för att läsa leaderboard från txt fil
void readLeaderboard(vector <Leaderboard>& leaderboard) {
	//Tömmer listan på leaderboard scores om den redan har lästs en gång
	leaderboard.clear();
	//Öppnar leaderboard txt filerna
	ifstream leaderboardScores("highscores.txt");
	ifstream leaderboardNames("names.txt");
	//Kollar om de gick att öppna
	if (!leaderboardScores || !leaderboardNames) {
		cout << "Could not read the file";
		return;
	}
	//Hämtar varje rad i filen och lägger värdet på raden i leaderboard vectorn
	string scoreStr = "";
	while (getline(leaderboardScores, scoreStr)) leaderboard.push_back({ stoi(scoreStr), ""});
	//Stänger filen
	leaderboardScores.close();

	string fileName = "";
	int i = 0;
	while (getline(leaderboardNames, fileName)) leaderboard[i++].name = fileName;
	//Stänger filen
	leaderboardNames.close();

	//Skriver ut topplistan
	for (int i = 0; i < leaderboard.size(); i++) {
		cout << i + 1 << ". " << leaderboard[i].score << " by " << leaderboard[i].name << '\n';
	}
}

//Funktion för att skriva highscore till leaderboard
void writeHighscore(int gameScore, vector<Leaderboard> leaderboard) {
	//Öppnar leaderboard filen
	ofstream leaderboardScore("highscores.txt");
	ofstream leaderboardName("names.txt");
	//Kollar så att den lyckades öppna
	if (!leaderboardScore || !leaderboardName) {
		cout << "Could not write to the file\n";
		return;
	}
	//Vector för de olika nya scoresen i leaderboard
	vector <Leaderboard> newLeaderboard = {};
	bool hasEntered = false;
	//Loopar genom varje score som lästes innan spelet
	for (int i = 0; i < leaderboard.size(); i++) {
		//Kollar om spelarens score är bättre än något av listans och att score inte redan skrivit. I så fall skrivs score före
		if (gameScore > leaderboard[i].score && !hasEntered) {
			cout << ANSI_CODES[SHOW_CURSOR] << "\033[25;10HENTER YOUR NAME: ";
			string userName = "";
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			getline(cin, userName);
			newLeaderboard.push_back({ gameScore , userName});
			hasEntered = true;
			//Går tillbaka ett index i scores för att lägga in den nerflyttade scoren också. Annars hoppas den över
			i--;
		}	
		else newLeaderboard.push_back({ leaderboard[i].score, leaderboard[i].name});
	}
	//Om storleken för den nya leaderboaren är mer än 10 så tar den bort sista elementet
	if (newLeaderboard.size() > 10) newLeaderboard.pop_back();
	//Skriver varje score på leaderboard till txt filen
	for (Leaderboard placement : newLeaderboard) {
		leaderboardScore << placement.score << '\n';
		leaderboardName << placement.name << '\n';
	}

	//Stänger filerna
	leaderboardScore.close();
	leaderboardName.close();
}

//Ritar vald rad av en tetromino. För hold och next block
void drawTetromino(int block, int row) {
	//Array för de 2 raderna i blocken
	string output[2] = {};
	//Kollar vilket block det är
	switch (block) {
		//Output får båda raderna tilldelade
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
	//Skriver bara ut den raden som ska skrivas ut
	cout << output[row] << ANSI_CODES[DEFAULT];
}

//Ritar upp spelplanen
void drawBoard(const GameStatistics& game, const Block& predictedBlock, int nextBlock) {
	//Återställer marköre och färg
	cout << ANSI_CODES[RESET_CURSOR] << ANSI_CODES[DEFAULT];
	//Skriver ut leveln
	for (int i = 0; i < 10; i++) cout << SQUARE;
	cout << "LEVEL: " << game.level << '\n';
	//Skiver ut övre ramen med grå bakgrundsfärg
	for (int i = 0; i < 6; i++) cout << SQUARE;
	cout << ANSI_CODES[GRAY];
	for (int i = 0; i < 12; i++) cout << SQUARE;
	cout << '\n';

	//Loopar genom brädets höjd
	for (int i = 0; i < HEIGHT; i++) {
		//Skriver ut en tom ruta för whitespace
		cout << ANSI_CODES[DEFAULT] << SQUARE;
		//Om höjden är 0 skrivs HOLD ut innan själva brädet
		if (i == 0) cout << "  HOLD  ";
		//Om 2 skrivs en rad av holdade blocket ut och 3 andra
		else if (i == 2) drawTetromino(game.heldBlock, 0);
		else if (i == 3) drawTetromino(game.heldBlock, 1);
		//Annars skriver den ut blankrader
		else drawTetromino(-1, 0);
		//Skriver ut en tom ruta för whitespace
		cout << ANSI_CODES[DEFAULT] << SQUARE;

		//Skriver ut ram
		cout << ANSI_CODES[GRAY] << SQUARE << ANSI_CODES[DEFAULT];
		//Loopar genom spelplans raden
		for (int j = 0; j < WIDTH; j++) {
			bool hasPrinted = false;
			//Om falling block är på denna positionen så skrivs blockets färg ut
			for (auto position : game.fallingBlock.positions) {
				if (position.x == j && position.y == i) {
					cout << ANSI_CODES[game.fallingBlock.color] << SQUARE << ANSI_CODES[DEFAULT];
					hasPrinted = true;
					break;
				}
			}
			//Om predicted block är på denna positionen och fallingblock inte skrevs ut skrivs en grå ruta ut
			for (auto position : predictedBlock.positions) {
				if (position.x == j && position.y == i && !hasPrinted) {
					cout << ANSI_CODES[DARK_GRAY] << SQUARE << ANSI_CODES[DEFAULT];
					hasPrinted = true;
					break;
					ANSI_CODES[predictedBlock.color];
				}
			}
			if (hasPrinted) continue; //Om fallingBlock eller predictedblock är printat hoppar den över resten i loopen

			//Annars skriver den ut brädets färg
			cout << ANSI_CODES[game.gameboard[i][j].color] << SQUARE;
		}
		//Skriv ut ram + blankrad
		cout << ANSI_CODES[GRAY] << SQUARE << ANSI_CODES[DEFAULT] << SQUARE;
		//Om höjden är 0 skrivs NEXT ut efter brädet
		if (i == 0) cout << "  NEXT  ";
		//Om 2 skrivs en rad av nästa blocket ut och 3 andra
		else if (i == 2) drawTetromino(nextBlock, 0);
		else if (i == 3) drawTetromino(nextBlock, 1);
		//Byt rad
		cout << '\n';
	}
	//Skriver ut bottenraden och score under
	for (int i = 0; i < 6; i++) cout << SQUARE;
	cout << ANSI_CODES[GRAY];
	for (int i = 0; i < 12; i++) cout << SQUARE;
	cout << ANSI_CODES[DEFAULT] << '\n';
	for (int i = 0; i < 10; i++) cout << SQUARE;
	cout << "SCORE: " << game.score << '\n';
}

//Kollar om ett block ligger i ett annat eller utanför planen
bool isCollision(const Block& tetromino, const Board gameboard[HEIGHT][WIDTH]) {
	//Loopar genom varje position i blocket
	for (auto position : tetromino.positions)
		//Om positionen är på brädet
		if (position.x >= 0 && position.x < WIDTH && position.y >= 0 && position.y < HEIGHT) {
			//Kollar om det ligger en tetromino på den positionen
			if (gameboard[position.y][position.x].state == 1)
				return true; //I så fall returner true
		}
		//Annars om positionen är utanför brädet höger/vänster/ner så är det också kollision(uppåt får den vara)
		else if (position.x < 0 || position.x >= WIDTH || position.y >= HEIGHT) {
			return true;
		}
	return false;
}

//Flyttar en tetromino åt olika håll
bool moveTetromino(Block& tetromino, int delta_x, int delta_y, Board gameboard[HEIGHT][WIDTH]) {
	//Variabel för den nya positonen
	Block newPosition = {};
	//Loopar genom varje position
	for (int i = 0; i < 4; i++) {
		//newPosition får gamla positionen + delta_x/y
		newPosition.positions[i].x = tetromino.positions[i].x + delta_x;
		newPosition.positions[i].y = tetromino.positions[i].y + delta_y;
	}
	//Om det blir kollision flyttar den inte
	if (isCollision(newPosition, gameboard)) return false;

	//Annars får tetrominon de nya positionerna
	for (int i = 0; i < 4; i++) tetromino.positions[i] = newPosition.positions[i];

	return true;
}

//Roterar tetromino medurs + flyttar den om den ligger på ogiltig plats
void rotateTetromino(GameStatistics& game) {
	//Gul/O tetromino kan inte rotera
	if (game.fallingBlock.color == YELLOW) return;
	
	//Variabel för hur den nya blocket kommer ligga
	Block newBlock = {};

	//Sätter position 0 i blocket som referens
	Position reference = game.fallingBlock.positions[0];

	//Loopar genom varje bit i blocket
	for (int i = 0; i < 4; i++) {
		//y koordinater i referens till referensen blir negativa y koordinat i referens efteråt
		//x koordinater i referens till referensen blir samma y koordinat i referens efteråt
		newBlock.positions[i].x = reference.x - (game.fallingBlock.positions[i].y - reference.y);
		newBlock.positions[i].y = reference.y + (game.fallingBlock.positions[i].x - reference.x);
	}

	//Om det roterade blocket hamnat i ett annat block så flyttas det till en giltig plats om det är tillräckligt nära
	int delta_y = 0;
	while (isCollision(newBlock, game.gameboard) && delta_y > -5) {
		//Array som bestämmer vilken ordning delta_x ska testas
		int moveOrder[5] = { 0, 1, -1, 2, -2 };
		//Loopar alla delta_x och försöker flytta
		for (int i = 0; i < 5; i++) {
			//Om det går att flytta så breakar den ut ur for loopen
			if (moveTetromino(newBlock, moveOrder[i], delta_y, game.gameboard)) 
				break;
		}
		//Nästa loop så öka delta y med 1 och samma delta x testas
		delta_y--;
	}
	//Om en giltig posiiton hittades tillräckligt nära tilldelas fallingBlock newBlocks positioner
	if (delta_y != -5)
		for (int i = 0; i < 4; i++) game.fallingBlock.positions[i] = newBlock.positions[i];
}

//Fyller bag med de 7 formerna i random ordning
void fillBag(vector<int>& bag) {
	//Loopar tills bagen har alla former
	while (bag.size() != 7) {
		int randomShape = rand() % 7; //Väljer ett random tal 0-6 som representerar en tetromino
		bool alreadyInBag = false; 
		for (auto shape : bag) //Kollar om den formen redan ligger i bagen
			if (randomShape == shape) alreadyInBag = true;

		if (!alreadyInBag) bag.push_back(randomShape); //Om den inte finns så läggs den i bagen
	}
}

//Ger fallingBlock startposition och random form/färg
void spawnNewBlock(GameStatistics& game) {
	//Hittar sista värdet i bag och sparar det
	int nextShape = game.bag.back();
	game.bag.pop_back();//Tar bort sista elementet
	//Kollar vilken form som är nästa och anger start position och färg till fallingBlock. 
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

	//Om bagen är tom så fylls den på igen
	if (game.bag.empty()) fillBag(game.bag);
}

//Byter ut det fallande blocket mot det som hålls och tvär om
void holdBlock(GameStatistics& game) {
	//Om det hålls ett block så läggs det blocket längst fram i kön/bagen
	if (game.heldBlock != -1) game.bag.push_back(game.heldBlock);
	//Lägger fallingblock i hold
	game.heldBlock = game.fallingBlock.color;
}

//Lägger det fallande blocket på brädet
void placeTetromino(GameStatistics& game, bool& gameover, bool& hasHeldBlock) {
	//Loopar genom alla positioner för det fallande blocker
	for (int i = 0; i < 4; i++) {
		//Kollar så att man inte accessar array out of range
		if(game.fallingBlock.positions[i].y >= 0 && game.fallingBlock.positions[i].y < HEIGHT && game.fallingBlock.positions[i].x >= 0 && game.fallingBlock.positions[i].x < WIDTH)
			//Sätter gamestate på den platsen till 1 och färgen till fallande blocket färg
			game.gameboard[game.fallingBlock.positions[i].y][game.fallingBlock.positions[i].x] = { 1, game.fallingBlock.color };
	}

	//Spawnar nytt block
	spawnNewBlock(game);
	//Kollar om det är kollision direkt, ist gameover
	if (isCollision(game.fallingBlock, game.gameboard)) gameover = true;
	//Tillåter spelaren att hålla block igen
	hasHeldBlock = false;
}

//Förutser var det fallande blocket kommer hamna och skickar tillbaka det
Block predictBlock(GameStatistics game) {
	//Flyttar block neråt tills det inte går mer
	while (moveTetromino(game.fallingBlock, 0, 1, game.gameboard));
	//Skickar tillbaka blocket
	return game.fallingBlock;
}

//Sparar olika värden i txt filer för att kunna fortsätta på samma ställe nästa gång
void saveGame(const GameStatistics& game) {
//Öppnar text filerna för de sparade värdena
//Läser och anger värdena till game
//Stänger filen
	ofstream savedBag("bag.txt");
	for (auto shape : game.bag) savedBag << shape;
	savedBag.close();

	ofstream savedLevel("level.txt");
	savedLevel << game.level;
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
}

//Anger startvärden för ett helt nytt tetris
void initializeGame(GameStatistics& game) {
	game.bag.clear(); //Tömmer bag
	fillBag(game.bag); //Fyller den på nytt
	//Sätter hela gameboard till state 0 och färg default
	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++)
			game.gameboard[i][j] = { 0,DEFAULT };
	game.heldBlock = -1; //inget held block
	game.level = game.linesCleared = game.score = 0; //Level, lines cleared och score = 0
}

void countDown() {
	//Skriver ut 3-2-1 i mitten av brädet med 500ms delay
	for (int i = 3; i > 0; i--) {
		cout << "\033[12;25H" << i;
		Sleep(500);
	}
	//Tömmer input buffer så att användare inte kan göra inputs under nedräkning
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}

int menuChoice(int optionsCount, int posX, int posY, vector<string> text) {
	int playerChoice = 1;
	bool redo = true;
	do {
		//Skriver ut valen användaren har
		for (int i = 0; i < optionsCount; i++) { //Loopar så många gånger som det finns options
			if (playerChoice == i + 1) cout << ANSI_CODES[WHITE] << ANSI_CODES[BLACK_TEXT]; //Skriver vit bakgrund svart text om det är detta index som är spelarens val
			else cout << ANSI_CODES[DEFAULT]; //Annars är det vanlig färg
			cout << "\033[" << posY + i << ";" << posX << "H"; //Flyttar musmarkören till rätt position
			cout << text[i]; //Skriver ut i:te alternativet i menyn
		}
		//Kollar om användaren byter val eller väljer det markerade valet
		redo = true;
		switch (_getch()) {
		case ARROW_UP:
			if (playerChoice > 1) playerChoice--;
			else playerChoice = optionsCount;
			break;
		case ARROW_DOWN:
			if (playerChoice < optionsCount) playerChoice++;
			else playerChoice = 1;
			break;
		case ENTER:
			redo = false;
			break;
		}
	} while (redo);
	//Återställer färg
	cout << ANSI_CODES[DEFAULT];
	return playerChoice; //Returnerar spelarens val
}

//Meny när man pausar med olika menyval
void pauseMenu(bool& quit, bool& gameover, GameStatistics game, chrono::steady_clock::time_point& lastTime) {
	bool redo = false;
	//Loopar tills användaren lämnar menyn
	do {
		//Visar att spelet är pausat mitt på skärmen
		cout << "\033[10;22H" << "PAUSED";

		redo = false;
		//vector för de olika menyvalen
		vector<string> menuText = { "CONTINUE", "SAVE AND QUIT", "QUIT" };
		switch (menuChoice(3, 19, 11, menuText)) {
		case 1: 
			//Skriver över meny texten
			cout << "\033[10;22H      \033[11;19H           \033[12;16H                \033[13;21H       ";
			//Räknar ner från 3 innan det startar igen
			countDown();
			lastTime = chrono::steady_clock::now();
			return;//Gå tillbaka till spelet
		case 2: 
			game.bag.push_back(game.fallingBlock.color);
			saveGame(game); //Sparar spelet
			cout << "\033[8;22HSAVING";
			Sleep(1000);
			quit = true;
			return;
		case 3: 
			gameover = true; //Avslutar spelet
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
	//Kollar om ett knapp blivit nertryckt
	if (_kbhit()) {
		//Kollar vilken knapp det är
		switch (_getch()) {
		case ARROW_UP: //Om pil upp rotera medurs
			rotateTetromino(game);
			break;
		case ARROW_LEFT: //Om vänster flytta fallande block vänster
			moveTetromino(game.fallingBlock, -1, 0, game.gameboard);
			break;
		case ARROW_DOWN: { //Om ner minska fallDelay till 10 ms
			//Kollar om den träffar marken nästa steg. 
			// I så fall ökas inte hastigheten så att man kan flytta/rotera blocket efter att det kommit till botten
			Block tempBlock = game.fallingBlock;
			fallingDelay = (moveTetromino(tempBlock, 0, 1, game.gameboard)) ? 10 : fallingDelay;
			break;
		}
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
			pauseMenu(quit, gameover, game, lastTime);
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
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}

//Tar bort rader som är fulla
void clearLines(GameStatistics& game) {
	//vector som sparar index för de rader som är fulla
	vector<int> fullLines = {};
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
		game.score += 40 * (game.level + 1);
		break;
	case 2:
		game.score += 100 * (game.level + 1);
		break;
	case 3:
		game.score += 300 * (game.level + 1);
		break;
	case 4:
		game.score += 1200 * (game.level + 1);
		break;
	}
	//Ökar totala lines cleared
	game.linesCleared += (int)fullLines.size();
	//Om linesCleared är mer än 10x leveln + 1, så ökar leveln
	if (game.linesCleared > (game.level + 1) * 10) game.level++;
}

//Spelfunktionen
void tetris(GameStatistics& game, bool gameLoaded, bool& gameover) {
	//Rensar skärmen och gömmer musmarkören
	system("cls");
	cout << ANSI_CODES[HIDE_CURSOR];

	//Standard variabler varje spel
	
	bool quit = false;
	int fallingDelay = fallingDelays[0];
	bool hasHeldBlock = false;

	spawnNewBlock(game);
	Block predictedBlock = predictBlock(game);
	drawBoard(game, predictedBlock, game.bag.back());
	//Räknar ner från 3 innan det startar
	countDown();
	auto lastTime = chrono::steady_clock::now();
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
			//Gamescore ökar med 1 om det föll naturligt eller 2 om spelaren soft droppade det manuellt
			game.score += (fallingDelay == 10) ? 2 : 1;
			//Sätter att nu var senaste gången blocket föll
			lastTime = chrono::steady_clock::now();
		}
		//Skapar ett förutspått shadow block som visar var det fallande blocket hamnar om man skickar ner det
		predictedBlock = predictBlock(game);
		//Ritar upp brädet
		drawBoard(game, predictedBlock, game.bag.back());
		//Tar bort lines om det finns några fulla
		clearLines(game);
	}
	//När spelet är över kollar den om det är över pga gameover eller quit
	if (quit) return; //Om quit kommer man till meny direkt
	cout << "\033[12;21H\033[31mGAMEOVER" << ANSI_CODES[DEFAULT]; // Annars skrivs gameover ut i 3 sekunder
	Sleep(3000);
}

//Ändrar spelvärden till det sparade spelets värden
void loadGame(GameStatistics& newGame) {
	//Öppnar txt filerna för gameboard state och color för läsning
	ifstream gameboardState("gameboard state.txt");
	ifstream gameboardColor("gameboard color.txt");
	int rowIndex = 0;
	string row = "";
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
	savedHeldBlock.close();

	ifstream savedLinesCleared("lines cleared.txt");
	savedLinesCleared >> newGame.linesCleared;
	savedLinesCleared.close();

	//läser raden och tar varje siffra i raden och lägger i bagen
	ifstream savedBag("bag.txt");
	string numbers = "";
	getline(savedBag, numbers);

	for (char number : numbers) newGame.bag.push_back(number - '0');

	savedBag.close();
	newGame.bag.back();
}

//Startmeny med leaderboard och olika val
bool startMenu(GameStatistics& newGame, bool& gameLoaded, vector<Leaderboard>& leaderboard) {
	//Återställer text färger och visar musmarkör
	cout << ANSI_CODES[DEFAULT] << ANSI_CODES[HIDE_CURSOR];
	
	bool redo = false;
	//Så länge användaren skriver fel input loopas det
	do {
		system("cls");
		//Visar leaderboarden
		cout << "HIGHSCORES\n\n";
		readLeaderboard(leaderboard);
		redo = false;
		vector<string> menuText = { "START NEW GAME" , "LOAD GAME" , "QUIT" };
		switch (menuChoice(3, 0, 14, menuText)) {
		case 1: //Start new game
			cout << "\n\nThe new game will overwrite the current saved game\nAre you sure you want to continue?";
			menuText = { "YES" , "NO"};
			switch (menuChoice(2, 0, 20, menuText)) {
			case 1:
				break;
			default:
				redo = true;
				break;
			}
			if (redo) break;

			//Ber användaren välja startlevel
			cout << ANSI_CODES[SHOW_CURSOR];
			cout << "\nChoose starting level(0-19): ";
			//Loopar tills användare valj giltig level att starta på
			do {
				cin >> newGame.level;
				cin.ignore();
				if (!(newGame.level >= 0 && newGame.level <= 19)) cout << "Try again!\n";

			} while (!(newGame.level >= 0 && newGame.level <= 19));
			//Sätter gameloaded till false eftersom det är nytt spel
			gameLoaded = false;
			return true;
		case 2: //LOAD GAME
			//Initialiserar startvärden till de sparade spelet värden
			loadGame(newGame);
			//Sätter att game blivit loadat
			gameLoaded = true;
			return true;
		case 3: //QUIT
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
	srand((int)time(0));
	//setlocale(LC_ALL, "sv_SE.UTF-8");
	bool run = true;
	//Main loop som gör att man kommer tillbaka till meny när spelet är över
	while (run) {
		GameStatistics newGame = {};
		initializeGame(newGame);
		vector <Leaderboard> leaderboard = {};
		bool gameLoaded = false, gameover = false;
		
		//Går till startmenyn för att välja hur man vill starta spelet
		run = startMenu(newGame, gameLoaded, leaderboard);
		
		//Om användaren inte valt quit startar spelet med startvärden om användaren valt
		if(run) tetris(newGame, gameLoaded, gameover);
		//Om det blivit gameover ska scoret läggas in på leaderboard om det är bättre än 10an
		if (gameover) {
			if (newGame.score > leaderboard.back().score)
				writeHighscore(newGame.score, leaderboard);
			//Spara ett tomt spel så att den gamla sparfilen inte ligger kvar
			initializeGame(newGame);
			saveGame(newGame);
		}
	}
	return 0;
}