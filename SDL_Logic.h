#ifndef SDL_Logic
#define SDL_Logic

#include<math.h>
#include<stdio.h>
#include<string.h>

#define _USE_MATH_DEFINES

extern "C" {
#include"./sdl-2.0.7/include/SDL.h"
#include"./sdl-2.0.7/include/SDL_main.h"
}

// main
#ifdef __cplusplus
extern "C"
#endif

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

struct display {
	SDL_Renderer *renderer;
	SDL_Window *window;
	SDL_Texture* gameTextures[5];
	SDL_Surface* charset;


	// inicjalizacja window and renderera jak i za³adowanie potrzebnych tekstur
	int initialize();
	void cleanUp();

};


// wczytanie pojedyñczej powierzchni.
SDL_Surface* loadSurface(const char* path);

// wczytanie pojedyñczej tekstury.
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path);


// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset);

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y);

// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color);

// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color);

// rysowanie prostok¹ta o d³ugoœci boków l i k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor);



#endif 