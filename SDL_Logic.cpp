#include"SDL_Logic.h"


int display::initialize() {
	this->charset = loadSurface("./cs8x8.bmp");
	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &(this->window), &(this->renderer))) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		return 1;
	}
	const char* names[5] = { "floor.bmp", "wall.bmp", "crate.bmp", "goal.bmp", "player.bmp"};
	for (int i = 0; i < sizeof(this->gameTextures) / sizeof(SDL_Texture*); i++) {
		if ((this->gameTextures[i] = loadTexture(this->renderer, names[i])) == NULL)
			return 1;
	}
	return 0;
}


void display::cleanUp() {
	SDL_FreeSurface(this->charset);
	for (int i = 0; i < sizeof(this->gameTextures) / sizeof(SDL_Texture*); i++) {
		SDL_DestroyTexture(this->gameTextures[i]);
	}
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
}

SDL_Surface* loadSurface(const char* path) {
	SDL_Surface* charset = SDL_LoadBMP(path);
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		return charset;
	};
	SDL_SetColorKey(charset, true, 0x0000);
	return charset;
}


SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
	SDL_Texture* texture = NULL;
	char buff[256];
	sprintf(buff, "./textures/%s", path);
	SDL_Surface* surface = SDL_LoadBMP(buff);
	if (surface == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load surface: %s", SDL_GetError());
		return texture;
	}
	if (SDL_SetColorKey(surface, true, SDL_MapRGB(surface->format, 255, 0, 0)) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set the color key: %s", SDL_GetError());
		return texture;
	}
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	return texture;
}



void DrawString(SDL_Surface *screen, int x, int y, const char *text,
	SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};



void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};



void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
};



void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};



void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};
