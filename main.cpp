#include"SDL_Logic.h"
#include"Game_Logic.h"

int main(int argc, char **argv) {
	int quit;
	SDL_Event event;
	display gameDisplay;
	game_states gameState = MAIN_MENU;

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
	SDL_SetWindowTitle(gameDisplay.window, "Sokoban");

	map_list mapList("./maps/map.list");
	int map_number = 0;
	char map_path[50];
	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);
	while (gameState != QUIT) {
		sprintf(map_path, "./maps/%s", mapList.arr[map_number]);
		switch (gameState) {
		case MAIN_MENU: gameState = menuLoop(gameDisplay); break;
		case GAME: gameState = gameLoop(map_path, gameDisplay); if(gameState == MAIN_MENU) map_number = 0; break;
		case RESET: gameState = gameLoop(map_path, gameDisplay); break;
		case SELECT: gameState = selectLoop(gameDisplay, &mapList, &map_number); break;
		case CONTINUE_GAME: 
			if (map_number < mapList.amount-1) {
				map_number++;
				gameState = GAME;
			}
			else {
				map_number = 0;
				gameState = MAIN_MENU;
			}
			break;
		}
	}

	// zwolnienie powierzchni / freeing all surfaces
	SDL_Quit();
	return 0;
	};
