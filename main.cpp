#include"SDL_Logic.h"
#include"Game_Logic.h"

int main(int argc, char **argv) {
	int t1, t2, quit, frames, movCount;
	double delta, worldTime, fpsTimer, fps;
	SDL_Event event;
	SDL_Surface *screen;
	SDL_Texture *scrtex;
	display gameDisplay;

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return 1;
		}

	if (gameDisplay.initialize() != 0) {
		SDL_Quit();
	}

	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(gameDisplay.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(gameDisplay.renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(gameDisplay.window, "Szablon do zdania drugiego 2017");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(gameDisplay.renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);



	actor Player; actor activeCrate;
	activeCrate.initialize(0, 0, true, PUSHING_SPEED);
	map* gameMap = loadMap("./maps/1.lvl", &Player);

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	movCount = 0;
	while(!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		worldTime += delta;


		SDL_FillRect(screen, NULL, czarny);
		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		sprintf(text, "Moves: %3d Time elapsed: %.1lf s  %.0lf FPS", movCount, worldTime, fps);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, gameDisplay.charset);
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer);
		SDL_RenderCopy(gameDisplay.renderer, scrtex, NULL, NULL);


		SDL_Rect tile = calculateTileDimension(gameMap);
	
		gameMap->draw(&gameDisplay, tile);
		Player.draw(&gameDisplay, tile);
		activeCrate.draw(&gameDisplay, tile);

		activeCrate.update(delta, gameMap, NULL);
		Player.update(delta, gameMap, &activeCrate);

		SDL_RenderPresent(gameDisplay.renderer);

		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if(event.key.keysym.sym == SDLK_UP) movCount+=Player.move(UP, gameMap, &activeCrate);
					else if(event.key.keysym.sym == SDLK_DOWN)movCount+=Player.move(DOWN, gameMap, &activeCrate);
					else if (event.key.keysym.sym == SDLK_LEFT)movCount+=Player.move(LEFT, gameMap, &activeCrate);
					else if (event.key.keysym.sym == SDLK_RIGHT)movCount+=Player.move(RIGHT, gameMap, &activeCrate);
					break;
				case SDL_KEYUP:
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		};

	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	gameDisplay.cleanUp();

	SDL_Quit();
	return 0;
	};
