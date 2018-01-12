#include"SDL_Logic.h"
#include"Game_Logic.h"



int main(int argc, char **argv) {
	display gameDisplay;
	game_states gameState = MAIN_MENU;

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		logError(SDL_GetError(), LOG_FILE);
		return 1;
		}

	if (gameDisplay.initialize() != 0) {
		logError(SDL_GetError(), LOG_FILE);
		SDL_Quit();
		return 1;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(gameDisplay.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(gameDisplay.renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(gameDisplay.window, "Sokoban");

	map_list mapList(MAP_LIST_DIR);
	int map_number = 0;
	char map_path[50];
	SDL_ShowCursor(SDL_DISABLE);
	while (gameState != QUIT) {
		sprintf(map_path, "./maps/%s", mapList.arr[map_number]);
		switch (gameState) {
		case MAIN_MENU: gameState = menuLoop(gameDisplay); break;
		case GAME: gameState = gameLoop(map_path, gameDisplay); if(gameState == MAIN_MENU) map_number = 0; break;
		case RESET: gameState = gameLoop(map_path, gameDisplay); if(gameState == MAIN_MENU) map_number = 0; break;
		case SELECT: gameState = selectLoop(gameDisplay, &mapList, &map_number); break;
		case SCORES: gameState = scoreLoop(gameDisplay, &mapList); break;
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

	SDL_Quit();
	return 0;
	};
