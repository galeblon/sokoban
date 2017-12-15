#ifndef Game_Logic
#define Game_Logic

#include<stdio.h>
#include"SDL_Logic.h"


#define PUSHING_SPEED 3
#define RUNNING_SPEED 12
#define ROTATING_SPEED 12

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
};

struct map {
	entities** entity;
	dimensions dimension;

	void draw(display* gameDisplay, SDL_Rect tile);
};

struct actor {
	bool is_puppet;
	double angle, old_angle;
	int rot_vel, mov_vel;
	double timer;
	coords pos; coords old_pos;
	mov_states mov_state;

	void initialize(int x, int y, bool isPuppet, int vel);
	int move(directions direction, map* gameMap, actor* crate);
	void update(double delta, map* gameMap, actor* crate); // todo
	void draw(display* gameDisplay, SDL_Rect tile);
	int process_input(directions direction, map* gameMap, actor* crate);

};


// zczytuje mape z pliku tekstowego i inicjalizuje wartosci poczatkowe gracza
map* loadMap(const char* fName, actor* player);

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
void getInterpolation(SDL_Rect* tile, int delta, int direction);

// powolanie animacji skrzynki do zycia
void animate_puppet(coords pos, map* gameMap, actor* puppet, directions direction);

#endif 