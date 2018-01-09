#ifndef Game_Logic
#define Game_Logic

#include<stdio.h>
#include"SDL_Logic.h"


#define NUM_OF_OPTIONS 4
#define OPTION_HEIGHT 8

#define PUSHING_SPEED 3
#define RUNNING_SPEED 6
#define ROTATING_SPEED 12

#define LEGEND_HEIGHT 100

#define MAX_PL_NAME_LENGTH 30
#define MAX_MAP_NAME_LENGTH 30

#define MAP_DIR "./maps/"
#define MAP_LIST_DIR "./maps/map.list"
#define SCORE_LIST_DIR "./scores/"


enum types {
	WALL,
	CRATE,
	PLAYER,
	FLOOR,
	GOAL,
	EMPTY
};


enum mov_states {
	STOPPED,
	MOVING,
	ROTATING,
	PUSHING
};


enum game_states {
	GAME,
	MAIN_MENU,
	RESET,
	SELECT,
	CONTINUE_GAME,
	SCORES,
	QUIT
};


enum directions {
	UP,
	DOWN,
	LEFT,
	RIGHT
};


struct coords {
	int x;
	int y;

	void setCoord(int a, int b) { x = a; y = b; };
};


struct dimensions {
	int width;
	int height;
};


struct entities {
	types type;
	bool is_goal;
};


struct map {
	entities** entity;
	dimensions dimension;

	bool isSolved();
	void draw(display* gameDisplay, SDL_Rect tile);
	~map();
};


struct map_list {
	int amount;
	char ** arr;

	void cleanUp();
	void reload(const char* path);
	map_list(const char* path);
	~map_list();
};


struct actor {
	bool is_puppet;
	double angle, old_angle;
	int rot_vel, mov_vel, moves;
	double timer;
	coords pos; coords old_pos;
	mov_states mov_state;

	void initialize(int x, int y, bool isPuppet, int vel);
	int move(directions direction, map* gameMap, actor* puppet);
	int update(double delta, map* gameMap, actor* puppet);
	void draw(display* gameDisplay, SDL_Rect tile);
	int process_input(directions direction, map* gameMap, actor* crate);

};


struct cursor {
	SDL_Rect shape;
	int pos;
	int jump;
	int origin_y;

	cursor(int size, int origin_x, int origin_y, int jump);
	void change_pos(int dir, int num_of_options, bool loop);
	int y_val();
	game_states pos_val();
};


struct score {
	char player_name[MAX_PL_NAME_LENGTH];
	int moves;
	float time;
};


struct score_board {
	score * array_time;
	score * array_moves;
	int size;

	void load_scores(const char* mapName);
	void cleanUp();

	~score_board();
};

// zczytuje mape z pliku tekstowego i inicjalizuje wartosci poczatkowe gracza
map* loadMap(const char* fName, actor* player);

// sprawdza czy wczytana mapa jest poprawna i zwraca true jesli tak jest
bool valid_map(map* mapToCheck);

// oblicza wymiary pojedynczego kafelka i marginesy
SDL_Rect calculateTileDimension(map* gameMap);

// sprawdza czy podane wspolrzedne mieszcza sie w tablicy kafelek
// zwraca 1 jezeli sie mieszcza, 0 w przeciwnym wypadku
int isInBounds(map* gameMap, int x, int y);


// sprawdza czy podane pole jest dostepne
// zwraca 0 jezeli nie jest
// zwraca 1 jezeli jest
// zwraca 2 jezeli powoduje popchniecie skrzyni
int checkCollision(map* gameMap, int x, int y, bool canPush, directions direction);

// przesuwa skrzynie w danym polozenie o jedno pole we wskazanym kierunku
void pushCrate(map* gameMap, int x, int y, directions direction);

// zamienia typ enum na kat w formie double
double directionToAngle(directions direction);

// zamienia kat na typ enum
directions angleToDirection(int angle);

// zwraca mniejszy kat z odpowiednim kierunkiem obrotu
double getRotation(double old_angle, double new_angle);

// zwraca odpowiednie przesuniecie w czasie w trakcie poruszania
void getInterpolation(SDL_Rect* tile, double delta, int direction);

// powolanie animacji skrzynki do zycia
void animate_puppet(coords pos, map* gameMap, actor* puppet, directions direction);


// petle w ktorej odbya sie glowna gra
game_states gameLoop(const char* lvlName, display &gameDisplay);

// petla w ktorej odbywa sie menu glowne
game_states menuLoop(display &gameDisplay);

// petla w w ktorej odbywa sie wybor poziomu
game_states selectLoop(display &gameDisplay, map_list* mapList, int* mapNumber);

// petla w ktorej odbywa sie wyswietlanie najlepszych wynikow
game_states scoreLoop(display &gameDisplay, map_list* mapList);

// podfunkcja menuLoop odpowiedzialna za wyswietlenie odpowienich pozycji w menu
void drawMenu(display &gameDisplay, SDL_Surface* screen, int top_margin, int spacing);

// podfunkcja selectLoop odpowiedzialna za wyswietlenie listy map
void drawSelect(display &gameDisplay, SDL_Surface *screen, const map_list* mapList, int spacing, int topMargin, int beginIndex, int max);

// podfunkcja scoreLoop odpowiedzialna za wyswietlenie wynikow dla danej mapy
void drawScore(display &gameDisplay, SDL_Surface *screen, const map_list* mapList, int max, int top_margin, int map_index, score_board* res, int begin_index);

// zamienia spacje na _ w lancuchu znakow;
void removeSpaces(char* str);

// dodaje mape do pliku z lista map
void addMap(char* map_name, const char* dir);

// zapisuje wynik uzytkownika na danej mapie do pliku
void saveScore(float time, int moves, char* player_name, const char* lvlName);

// wy³uskuje nazwe mapy ze scie¿ki
char* getFileFromPath(const char* lvlName);

// funkcje potrzebne do wywolania funkcji qsort
int cmp_by_time(const void* a, const void *b);
int cmp_by_moves(const void*a, const void *b);

#endif 