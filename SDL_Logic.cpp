#include"SDL_Logic.h"
#include"Game_Logic.h"



int display::initialize() {
	this->charset = loadSurface("./cs8x8.bmp");
	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &(this->window), &(this->renderer))) {
		logError(SDL_GetError(), LOG_FILE);
		return 1;
	}
	const char* names[5] = { "floor.bmp", "wall.bmp", "crate.bmp", "goal.bmp", "player.bmp"};
	for (int i = 0; i < sizeof(this->gameTextures) / sizeof(SDL_Texture*); i++) {
		if ((this->gameTextures[i] = loadTexture(this->renderer, names[i])) == NULL)
			return 1;
	}
	return 0;
}


display::~display() {
	SDL_FreeSurface(this->charset);
	for (int i = 0; i < sizeof(this->gameTextures) / sizeof(SDL_Texture*); i++) {
		SDL_DestroyTexture(this->gameTextures[i]);
	}
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
}


SDL_Surface* loadSurface(const char* path) {
	SDL_Surface* surface = SDL_LoadBMP(path);
	if (surface == NULL) {
		logError(SDL_GetError(), LOG_FILE);
		return surface;
	};
	SDL_SetColorKey(surface, true, 0x0000);
	return surface;
}


SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
	SDL_Texture* texture = NULL;
	char buff[256];
	sprintf(buff, "./textures/%s", path);
	SDL_Surface* surface = SDL_LoadBMP(buff);
	if (surface == NULL) {
		logError(SDL_GetError(), LOG_FILE);
		return texture;
	}
	if (SDL_SetColorKey(surface, true, SDL_MapRGB(surface->format, 255, 0, 0)) != 0) {
		logError(SDL_GetError(), LOG_FILE);
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


text_display::text_display(SDL_Renderer* renderer) {
	surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);
}


text_display::~text_display(){
	SDL_FreeSurface(this->surface);
	SDL_DestroyTexture(this->texture);
}


void getTextInput(display &gameDisplay, const char* query, char* res, int max_length) {
	text_display messages(gameDisplay.renderer);
	SDL_Rect textbox;
	textbox.h = 44;
	textbox.w = SCREEN_WIDTH;
	textbox.x = 0;
	textbox.y = SCREEN_HEIGHT / 2;
	SDL_Rect src_box;
	src_box.w = SCREEN_WIDTH;
	src_box.h = 44;
	src_box.x = src_box.y = 0;

	int border_color = SDL_MapRGB(messages.surface->format, 0xFF, 0xFF, 0xFF);
	int content_color = SDL_MapRGB(messages.surface->format, 0x00, 0x00, 0x00);
	char text[128];

	SDL_Event event;
	res[0] = '\0';
	SDL_StartTextInput();
	while (1) {
		DrawRectangle(messages.surface, 0, 0, SCREEN_WIDTH, 44, border_color, content_color);
		sprintf(text, "%s %s ", query, res);
		DrawString(messages.surface, messages.surface->w / 2 - strlen(text) * 8 / 2, 20, text, gameDisplay.charset);
		SDL_UpdateTexture(messages.texture, NULL, messages.surface->pixels, messages.surface->pitch);
		SDL_RenderCopy(gameDisplay.renderer, messages.texture, &src_box, &textbox);
		SDL_RenderPresent(gameDisplay.renderer);
		bool quit = false;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_BACKSPACE) {
					if (strlen(res))
						res[strlen(res) - 1] = '\0';
				}
				else if (event.key.keysym.sym == SDLK_RETURN)
					quit = true;
				else if (event.key.keysym.sym == SDLK_ESCAPE) {
					res[0] = '\0';
					return;
				}
				break;
			case SDL_TEXTINPUT:
				if (strlen(res) < max_length)
					strcat(res, event.text.text);
				break;
			case SDL_QUIT:
				res[0] ='\0';
				return;
				break;
			};
		}
		if (quit) break;
	}
	SDL_StopTextInput();
}


void logError(const char* val, const char* path) {
	FILE* fp = fopen(path, "a+");
	if (fp) {
		fputs(val, fp);
		fputs("\n", fp);
		fclose(fp);
	}
}
