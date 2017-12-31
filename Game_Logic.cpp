#include"Game_Logic.h"

void actor::initialize(int x, int y, bool isPuppet, int vel = RUNNING_SPEED) {
	angle = 0;
	old_angle = 0;
	timer = 0.0;
	rot_vel = ROTATING_SPEED;
	mov_vel = vel;
	moves = 0;
	is_puppet = isPuppet;
	pos.setCoord(x, y);
	old_pos.setCoord(x, y);
	mov_state = STOPPED;
}

void animate_puppet(coords pos, map* gameMap, actor* puppet, directions direction) {
	if (puppet != NULL) {
		pushCrate(gameMap, pos.x, pos.y, direction);
		puppet->old_pos = pos;
		puppet->angle = directionToAngle(direction);
		puppet->pos = pos;
		puppet->mov_state = MOVING;
		puppet->timer = 0;
	}
}

int actor::process_input(directions direction, map* gameMap, actor* puppet) {
	int	res;
	old_pos = pos;
	switch (direction) {
	case UP:
		res = checkCollision(gameMap, pos.x, pos.y - 1, true, UP);
		if (res > 0) {
			pos.y -= 1;
			angle = 0;
			if (res == 2) {
				animate_puppet(this->pos, gameMap, puppet, UP); this->mov_vel = PUSHING_SPEED;
			}
			return 1;
		}
		break;
	case DOWN:
		res = checkCollision(gameMap, pos.x, pos.y + 1, true, DOWN);
		if (res > 0) {
			pos.y += 1;
			angle = 180;
			if (res == 2) {
				animate_puppet(this->pos, gameMap, puppet, DOWN); this->mov_vel = PUSHING_SPEED;
			}
			return 1;
		}
		break;
	case LEFT:
		res = checkCollision(gameMap, pos.x - 1, pos.y, true, LEFT);
		if (res > 0) {
			pos.x -= 1;
			angle = 270;
			if (res == 2) {
				animate_puppet(this->pos, gameMap, puppet, LEFT); this->mov_vel = PUSHING_SPEED;
			}
			return 1;
		}
		break;
	case RIGHT:
		res = checkCollision(gameMap, pos.x + 1, pos.y, true, RIGHT);
		if (res > 0) {
			pos.x += 1;
			angle = 90;
			if (res == 2) {
				animate_puppet(this->pos, gameMap, puppet, RIGHT); this->mov_vel = PUSHING_SPEED;
			}
			return 1;
		}
		break;
	}
	return 0;
}

int actor::move(directions direction, map* gameMap, actor* puppet) {
	if (this->mov_state == STOPPED) {
		if (this->angle != directionToAngle(direction)) {
			old_angle = angle;
			angle = directionToAngle(direction);
			mov_state = ROTATING;
			timer = 0;
		}else {
			if (process_input(direction, gameMap, puppet)) {
				this->mov_state = MOVING;
				timer = 0;
				return 1;
			}
		}	
	}	
	return 0;
}

int actor::update(double delta, map* gameMap, actor* puppet) {
	if (this->mov_state == ROTATING) {
		if (this->timer*rot_vel <= 1) {
			this->timer += delta;
		}else {
			this->mov_state = STOPPED;
			if (process_input(angleToDirection(this->angle), gameMap, puppet)) {
				this->mov_state = MOVING;
			}
			this->timer = 0;
			this->old_angle = this->angle;
		}
	}
	if (this->mov_state == MOVING) {
		if (this->timer*mov_vel <= 1) {
			this->timer += delta;
		}else {
			this->mov_state = STOPPED;
			this->timer = 0;
			this->old_pos = this->pos;
			this->moves++;
			if(puppet != NULL)this->mov_vel = RUNNING_SPEED;
			if (gameMap->isSolved()) return 1;
		}
	}
	return 0;
}

void actor::draw(display* gameDisplay, SDL_Rect tile) {
	SDL_Rect tile_tmp = tile;
	SDL_Rect actor_frame;
	actor_frame.x = actor_frame.y = 0;
	actor_frame.h = actor_frame.w = 64;
	double curr_angle = this->angle;
	if (this->mov_state == STOPPED && is_puppet)
		return;
	if (this->mov_state == ROTATING) {
		curr_angle = getRotation(this->old_angle, this->angle);
		curr_angle = rot_vel*curr_angle*this->timer + this->old_angle;
	}
	if (this->mov_state == MOVING) {
		tile.y += this->old_pos.y*tile.w; // +((is_puppet && !(int(angle) % 90)) ? 1 : 0);
		tile.x += this->old_pos.x*tile.w; // +((is_puppet && !(int(angle) % 90)) ? 1 : 0);
		getInterpolation(&tile, this->timer*mov_vel*tile.w, this->angle);
		if (!is_puppet) {
			float frame = timer*mov_vel*4;
			actor_frame.x = 64 * (int(frame)%4);
		}
	}
	else {
		tile.y += this->pos.y*tile.w;
		tile.x += this->pos.x*tile.w;
	}
	if (is_puppet){
		tile_tmp.y += this->pos.y*tile.w;
		tile_tmp.x += this->pos.x*tile.w;
		getInterpolation(&tile_tmp, 1*tile.w, this->angle);
		SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[FLOOR_TEXTURE], NULL, &tile_tmp);
	}
	SDL_RenderCopyEx(gameDisplay->renderer, gameDisplay->gameTextures[is_puppet? CRATE_TEXTURE:PLAYER_TEXTURE], &actor_frame, &tile, curr_angle, NULL, SDL_FLIP_NONE);

}


void map::draw(display* gameDisplay, SDL_Rect tile) {
	SDL_Rect tmp_tile = tile;
	for (int i = 0; i < dimension.height; i++) {
		for (int j = 0; j < dimension.width; j++) {
			tile.y = tmp_tile.y + i*tile.w;
			tile.x = tmp_tile.x + j*tile.w;
			if (this->entity[i][j].type == WALL) {
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[WALL_TEXTURE], NULL, &tile);
			}
			else if (this->entity[i][j].type == CRATE) {
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[FLOOR_TEXTURE], NULL, &tile);
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[CRATE_TEXTURE], NULL, &tile);
			}
			else if (this->entity[i][j].type == FLOOR) {
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[FLOOR_TEXTURE], NULL, &tile);
			}
			if (this->entity[i][j].is_goal) {
				SDL_Rect goal_offset;
				goal_offset.h = goal_offset.w = 64;
				goal_offset.y = goal_offset.x = 0;
				if(this->entity[i][j].type == CRATE)
					goal_offset.x = 64;
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[GOAL_TEXTURE], &goal_offset, &tile);
			}
		}
	}
}


bool map::isSolved() {
	for (int i = 0; i < dimension.height; i++) {
		for (int j = 0; j < dimension.width; j++) {
			if (entity[i][j].is_goal == true && entity[i][j].type != CRATE)
				return false;
		}
	}
	return true;
}


map* loadMap(const char* fName, actor* player) {
	FILE* pMap = fopen(fName, "r");
	if (pMap == NULL)
		return NULL;
	map* loadedMap = new map;
	char val;
	fscanf(pMap, "%d\n%d\n", &(loadedMap->dimension.width), &(loadedMap->dimension.height));
	loadedMap->entity = new entities*[loadedMap->dimension.height];
	for (int i = 0; i < loadedMap->dimension.height; i++) {
		loadedMap->entity[i] = new entities[loadedMap->dimension.width];
	}
	for (int i = 0; i < loadedMap->dimension.height; i++) {
		for (int j = 0; j < loadedMap->dimension.width; j++) {
			val = fgetc(pMap);
			loadedMap->entity[i][j].is_goal = false;
			if (val == '\n') {
				j--;
				continue;
			}
			if (val == 'w') {
				loadedMap->entity[i][j].type = WALL;
			}else if(val == 'c'){
				loadedMap->entity[i][j].type = CRATE;
			}else if (val == 'f') {
				loadedMap->entity[i][j].type = FLOOR;
			}else if(val == 'g'){
				loadedMap->entity[i][j].type = FLOOR;
				loadedMap->entity[i][j].is_goal = true;
			}else if(val == 'p'){
				loadedMap->entity[i][j].type = FLOOR;
				if(player != NULL)
					player->initialize(j, i, false);
			} else{
				loadedMap->entity[i][j].type = EMPTY;
			}
		}
	}
	fclose(pMap);
	return loadedMap;
}


map_list::map_list(const char* path) {
	reload(path);
}
map_list::~map_list() {
	cleanUp();
}

void map_list::cleanUp() {
	for (int i = 0; i < amount; i++)
		delete[] arr[i];
	delete[] arr;
}


void map_list::reload(const char* path) {
	FILE* pMaps = fopen(path, "r");
	int count = 0;
	char val[30];
	while (fscanf(pMaps, "%s\n", val) != -1) {
		count++;
	}
	this->amount = count;
	rewind(pMaps);
	arr = new char*[count];
	for (int i = 0; i < count; i++) {
		this->arr[i] = new char[30];
	}
	for (int i = 0; i < count; i++) {
		fscanf(pMaps, "%s\n", val);
		memcpy(arr[i], val, 30);
	}
}


SDL_Rect calculateTileDimension(map* gameMap) {
	SDL_Rect result_tile;
	float sizeByHeight = (SCREEN_HEIGHT - LEGEND_HEIGHT) / gameMap->dimension.height;
	float sizeByWidth = SCREEN_WIDTH / gameMap->dimension.width;
	result_tile.w = sizeByHeight < sizeByWidth ? sizeByHeight : sizeByWidth;
	result_tile.h = result_tile.w;
	result_tile.x = (SCREEN_WIDTH - result_tile.w*gameMap->dimension.width) / 2;
	result_tile.y = (SCREEN_HEIGHT - result_tile.h*gameMap->dimension.height) / 2;
	return result_tile;
}


int isInBounds(map* gameMap, int x, int y) {
	if (x > gameMap->dimension.width || x < 0)
		return 0;
	if (y > gameMap->dimension.height || y < 0)
		return 0;
	return 1;
}


int checkCollision(map* gameMap, int x, int y, bool canPush, directions direction) {
	if (isInBounds(gameMap, x, y)) {
		if (gameMap->entity[y][x].type == FLOOR || gameMap->entity[y][x].type == GOAL)
			return 1;
		if (gameMap->entity[y][x].type == WALL || gameMap->entity[y][x].type == EMPTY)
			return 0;
		if (gameMap->entity[y][x].type == CRATE && canPush) {
			switch (direction) {
				case UP: y--; break;
				case DOWN: y++; break;
				case LEFT: x--; break;
				case RIGHT: x++; break;
			}
			if (checkCollision(gameMap, x, y, false, direction))
				return 2;
		}
	}
	return 0;
}


void pushCrate(map* gameMap, int x, int y, directions direction) {
	gameMap->entity[y][x].type = FLOOR;
	switch (direction) {
		case UP: y--; break;
		case DOWN: y++; break;
		case LEFT: x--; break;
		case RIGHT: x++; break;
	}
	gameMap->entity[y][x].type = CRATE;
}


double directionToAngle(directions direction) {
	double angle = 0;
	switch(direction){
		case UP: angle = 0; break;
		case DOWN: angle = 180; break;
		case LEFT: angle = 270; break;
		case RIGHT: angle = 90; break;
	}
	return angle;
}


double getRotation(double old_angle, double new_angle) {
	double res_angle = old_angle - new_angle;
	if (new_angle == 0 && old_angle > 180)
		res_angle = -360 + old_angle;
	if (new_angle > 180 && old_angle == 0)
		res_angle = 360 - new_angle;
	res_angle = res_angle == 180 ? res_angle : -res_angle;
	return res_angle;
}


void getInterpolation(SDL_Rect* tile, double delta, int direction) {
	switch (direction) {
		case 0:
			tile->y -= delta;
			break;
		case 90:
			tile->x += delta;
			break;
		case 180:
			tile->y += delta;
			break;
		case 270:
			tile->x -= delta;
	}
}


directions angleToDirection(int angle) {
	switch (angle) {
		case 0: return UP;
		case 90: return RIGHT;
		case 180: return DOWN;
		case 270: return LEFT;
	}
	return UP;
}


game_states gameLoop(const char* lvlName, display &gameDisplay) {
	int t1, t2, quit, frames;
	double delta, worldTime, fpsTimer, fps;
	SDL_Event event;
	actor Player; actor activeCrate;
	activeCrate.initialize(0, 0, true, PUSHING_SPEED);
	map* gameMap = loadMap(lvlName, &Player);
	if (gameMap == NULL) return MAIN_MENU;
	text_display messages(gameDisplay.renderer);


	char text[128];
	int czarny = SDL_MapRGB(messages.surface->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(messages.surface->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(messages.surface->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(messages.surface->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	while (true) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		worldTime += delta;


		SDL_FillRect(messages.surface, NULL, czarny);
		fpsTimer += delta;
		if (fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};
		DrawRectangle(messages.surface, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		sprintf(text, "Moves: %3d Time elapsed: %.1lf s  %.0lf FPS", Player.moves, worldTime, fps);
		DrawString(messages.surface, messages.surface->w / 2 - strlen(text) * 8 / 2, 10, text, gameDisplay.charset);
		SDL_UpdateTexture(messages.texture, NULL, messages.surface->pixels, messages.surface->pitch);
		SDL_RenderCopy(gameDisplay.renderer, messages.texture, NULL, NULL);


		SDL_Rect tile = calculateTileDimension(gameMap);

		gameMap->draw(&gameDisplay, tile);
		Player.draw(&gameDisplay, tile);
		activeCrate.draw(&gameDisplay, tile);

		if (Player.update(delta, gameMap, &activeCrate)) {
			gameMap->draw(&gameDisplay, tile);
			Player.draw(&gameDisplay, tile);
			SDL_RenderPresent(gameDisplay.renderer);
			break;
		}
		activeCrate.update(delta, gameMap, NULL);
		SDL_RenderPresent(gameDisplay.renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
			    if (event.key.keysym.sym == SDLK_UP) Player.move(UP, gameMap, &activeCrate);
				else if (event.key.keysym.sym == SDLK_DOWN) Player.move(DOWN, gameMap, &activeCrate);
				else if (event.key.keysym.sym == SDLK_LEFT) Player.move(LEFT, gameMap, &activeCrate);
				else if (event.key.keysym.sym == SDLK_RIGHT) Player.move(RIGHT, gameMap, &activeCrate);
				else if (event.key.keysym.sym == SDLK_n) { 
					delete gameMap;
					return RESET;
				}
				else if (event.key.keysym.sym == SDLK_ESCAPE) {
					delete gameMap;
					return MAIN_MENU;
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				delete gameMap;
				return QUIT;
				break;
			};
		};
		frames++;
	};
	char player_name[256];
	getTextInput(gameDisplay, "Level completed! Enter your name: ", player_name, MAX_PL_NAME_LENGTH);
	removeSpaces(player_name);
	if (strlen(player_name))
		saveScore(worldTime, Player.moves, player_name, lvlName);
	printf("%s", player_name);
	delete gameMap;
	return CONTINUE_GAME;
}


game_states menuLoop(display &gameDisplay) {
	SDL_Event event;
	text_display messages(gameDisplay.renderer);

	int top_margin = SCREEN_HEIGHT / 2 - NUM_OF_OPTIONS*(OPTION_HEIGHT + 50) / 2;
	int spacing = SCREEN_HEIGHT/NUM_OF_OPTIONS*0.2;
	cursor menuCursor(OPTION_HEIGHT, messages.surface->w / 2 - 100, top_margin, spacing);

	while (1) {
		menuCursor.shape.y = menuCursor.y_val();
		SDL_FillRect(messages.surface, NULL, SDL_MapRGB(messages.surface->format, 0x00, 0x00, 0x00));
		drawMenu(gameDisplay, messages.surface, top_margin, spacing);
		SDL_FillRect(messages.surface, &(menuCursor.shape), SDL_MapRGB(messages.surface->format, 0xFF, 0xFF, 0xFF));

		SDL_UpdateTexture(messages.texture, NULL, messages.surface->pixels, messages.surface->pitch);
		SDL_RenderCopy(gameDisplay.renderer, messages.texture, NULL, NULL);
		SDL_RenderPresent(gameDisplay.renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) return QUIT;
				else if (event.key.keysym.sym == SDLK_RETURN) return menuCursor.pos_val();
				else if (event.key.keysym.sym == SDLK_UP) menuCursor.change_pos(-1, NUM_OF_OPTIONS, true);
				else if (event.key.keysym.sym == SDLK_DOWN) menuCursor.change_pos(1, NUM_OF_OPTIONS, true);
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				return QUIT;
				break;
			};
		};
	}
}


game_states selectLoop(display &gameDisplay, map_list* mapList, int* mapNumber) {
	SDL_Event event;
	text_display messages(gameDisplay.renderer);

	int top_margin = 24;
	int spacing = 12;
	int begin_index = 0;
	int max_on_screen = (SCREEN_HEIGHT-top_margin)/(spacing);
	cursor menuCursor(OPTION_HEIGHT, messages.surface->w / 2 - 100, top_margin, spacing);

	while (1) {
		menuCursor.shape.y = menuCursor.y_val();
		SDL_FillRect(messages.surface, NULL, SDL_MapRGB(messages.surface->format, 0x00, 0x00, 0x00));
		drawSelect(gameDisplay, messages.surface, mapList, spacing, top_margin, begin_index, max_on_screen);
		SDL_FillRect(messages.surface, &(menuCursor.shape), SDL_MapRGB(messages.surface->format, 0xFF, 0xFF, 0xFF));

		SDL_UpdateTexture(messages.texture, NULL, messages.surface->pixels, messages.surface->pitch);
		SDL_RenderCopy(gameDisplay.renderer, messages.texture, NULL, NULL);
		SDL_RenderPresent(gameDisplay.renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) return MAIN_MENU;
				else if (event.key.keysym.sym == SDLK_RETURN) {
					*mapNumber = menuCursor.pos + begin_index;
					return GAME;
				}
				else if (event.key.keysym.sym == SDLK_UP) menuCursor.change_pos(-1, mapList->amount, false);
				else if (event.key.keysym.sym == SDLK_DOWN) menuCursor.change_pos(1, mapList->amount, false);
				else if (event.key.keysym.sym == SDLK_a) {
					char map_name[256];
					getTextInput(gameDisplay, "Enter map name: ", map_name, MAX_MAP_NAME_LENGTH);
					removeSpaces(map_name);
					if (strlen(map_name)) {
						addMap(map_name, MAP_LIST_DIR);
						mapList->cleanUp();
						mapList->reload(MAP_LIST_DIR);
					}
					printf("%s", map_name);
				}
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				return QUIT;
				break;
			};
		};
		if (menuCursor.pos >= max_on_screen) {
			if(begin_index<mapList->amount-max_on_screen)begin_index += 1;
			menuCursor.pos--;
		}
		if (menuCursor.pos < 0) {
			if(begin_index>0)begin_index -= 1;
			menuCursor.pos++;
		}
	}
}


game_states scoreLoop(display &gameDisplay, map_list* mapList){ 
	SDL_Event event;
	text_display messages(gameDisplay.renderer);
	int top_margin = 24;
	int spacing = 12;
	int max_on_screen = (SCREEN_HEIGHT - top_margin) / (spacing);
	int map_index = 0;
	score_board results;
	results.load_scores(mapList->arr[map_index]);
	while (1) {
		SDL_FillRect(messages.surface, NULL, SDL_MapRGB(messages.surface->format, 0x00, 0x00, 0x00));
		drawScore(gameDisplay, messages.surface, mapList, max_on_screen, top_margin, map_index, &results);
		SDL_UpdateTexture(messages.texture, NULL, messages.surface->pixels, messages.surface->pitch);
		SDL_RenderCopy(gameDisplay.renderer, messages.texture, NULL, NULL);
		SDL_RenderPresent(gameDisplay.renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) return MAIN_MENU;
				else if (event.key.keysym.sym == SDLK_UP) printf("dol");
				else if (event.key.keysym.sym == SDLK_DOWN) printf("gora");
				else if (event.key.keysym.sym == SDLK_LEFT) {
					map_index--;
					if (map_index < 0) map_index = mapList->amount - 1;
					results.cleanUp();
					results.load_scores(mapList->arr[map_index]);
				}
				else if (event.key.keysym.sym == SDLK_RIGHT) {
					map_index++;
					if (map_index > mapList->amount - 1) map_index = 0;
					results.cleanUp();
					results.load_scores(mapList->arr[map_index]);
				}
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				return QUIT;
				break;
			};
		};
	}
}


void drawScore(display &gameDisplay, SDL_Surface *screen, const map_list* mapList, int max, int top_margin, int map_index, score_board* res) {
	char text[256] = "\0";
	sprintf(text, "\032 %s \033", mapList->arr[map_index]);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 2, text, gameDisplay.charset);
	if (res->size) {
		sprintf(text, "By time: ");
		DrawString(screen, screen->w / 2 - 30*8 - 4*8, top_margin, text, gameDisplay.charset);
		sprintf(text, "By Moves: ");
		DrawString(screen, screen->w / 2 + 60, top_margin, text, gameDisplay.charset);
		for (int i = 0; i < res->size; i++) {
			sprintf(text, "%s %.2fs", res->array_time[i].player_name, res->array_time[i].time);
			DrawString(screen, screen->w / 2 - 30*8 - 4 * 8, 2*top_margin + i*12, text, gameDisplay.charset);
			sprintf(text, "%s %d", res->array_moves[i].player_name, res->array_moves[i].moves);
			DrawString(screen, screen->w / 2 + 60, 2*top_margin + i*12, text, gameDisplay.charset);
		}
	}
}


void drawSelect(display &gameDisplay, SDL_Surface *screen, const map_list* mapList, int spacing, int topMargin, int beginIndex, int max) {
	char text[256] = "Press a - to add map";
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 2, text, gameDisplay.charset);
	int endIndex = beginIndex + max > mapList->amount ? mapList->amount : beginIndex + max;
	for (int i = beginIndex; i < endIndex; i++) {
		sscanf(mapList->arr[i], "%s", text);
		text[strlen(text) - 4] = '\0';
		DrawString(screen, screen->w / 2 - strlen(mapList->arr[i]) * 8 / 2,  topMargin+(i-beginIndex)*spacing, text, gameDisplay.charset);
	}
}


void drawMenu(display &gameDisplay, SDL_Surface* screen, int top_margin, int spacing) {
	const char* menu_options[NUM_OF_OPTIONS] = { "New Game", "Select Level", "High Scores", "Quit" };
	for (int i = 0; i < NUM_OF_OPTIONS; i++) {
		DrawString(screen, screen->w / 2 - strlen(menu_options[i]) * 8 / 2, top_margin+i*spacing, menu_options[i], gameDisplay.charset);
	}
}


cursor::cursor(int size, int origin_x, int origin_y, int jump) {
	this->shape.w = this->shape.h = size;
	this->shape.x = origin_x;
	this->shape.y = origin_y;
	this->origin_y = origin_y;
	this->jump = jump;
	this->pos = 0;
}


void cursor::change_pos(int dir, int num_of_options, bool loop) {
	if (this->pos == 0 && dir < 0 && loop)
		this->pos = num_of_options - 1;
	else
		this->pos = (this->pos + dir) % num_of_options;
}


int cursor::y_val() {
	return this->origin_y + this->pos*this->jump;
}


game_states cursor::pos_val() {
	switch (this->pos) {
	case 0:
		return GAME;
	case 1:
		return SELECT;
	case 2:
		return SCORES;
	case 3:
		return QUIT;
	}
	return GAME;
}


map::~map() {
	for (int i = 0; i < dimension.height; i++)
		delete[] entity[i];
	delete[] entity;

}


void removeSpaces(char* str) {
	for (int i = 0; i < strlen(str); i++)
		if (str[i] == ' ')
			str[i] = '_';
}


void addMap(char* map_name, const char* dir) {
	FILE* fp;
	char map_file[256];
	sprintf(map_file, "\n%s.lvl", map_name);
	fp = fopen(dir, "a+");
	if (fp != NULL) {
		fputs(map_file, fp);
		fclose(fp);
	}
}

void saveScore(float time, int moves, char* player_name, const char* lvlName) {
	FILE* fp;
	char score_file[256];
	char* short_name = getFileFromPath(lvlName);
	sprintf(score_file, "%s%s.score", SCORE_LIST_DIR, short_name);
	fp = fopen(score_file, "a+");
	if (fp != NULL) {
		fprintf(fp, "%s %d %f\n", player_name, moves, time);
	}
	fclose(fp);
	delete short_name;
}

char* getFileFromPath(const char* lvlName) {
	char * lvl_local = new char[MAX_MAP_NAME_LENGTH+4];
	memcpy(lvl_local, &(lvlName[strlen(MAP_DIR)]), strlen(lvlName)-strlen(MAP_DIR));
	lvl_local[strlen(lvlName) - strlen(MAP_DIR)] = '\0';
	return lvl_local;
}


int cmp_by_time(const void* a, const void *b) {
	score casted_a = *(score*)a;
	score casted_b = *(score*)b;

	if (casted_a.time > casted_b.time) return 1;
	if (casted_a.time < casted_b.time) return -1;
	return 0;
}

int cmp_by_moves(const void* a, const void *b) {
	score casted_a = *(score*)a;
	score casted_b = *(score*)b;

	if (casted_a.moves > casted_b.moves) return 1;
	if (casted_a.moves < casted_b.moves) return -1;
	return 0;
}


score_board::~score_board() {
	cleanUp();
}


void score_board::cleanUp() {
	if (this->size) {
		delete[] array_time;
		delete[] array_moves;
		this->size = 0;
	}
}


void score_board::load_scores(const char* mapName) {
	this->size = 0;
	char path[256];
	sprintf(path, "%s%s.score", SCORE_LIST_DIR, mapName);
	FILE* fp;
	fp = fopen(path, "r");
	if (fp != NULL) {
		int count = 0;
		score val;
		while (fscanf(fp, "%s %d %f\n", &(val.player_name), &(val.moves), &(val.time)) != -1) {
			count++;
		}
		
		if (count) {
			this->size = count;
			this->array_moves = new score[count];
			this->array_time = new score[count];
			rewind(fp);
			count = 0;
			while (fscanf(fp, "%s %d %f\n", &(val.player_name), &(val.moves), &(val.time)) != -1) {
				memcpy(&(this->array_moves[count]), &val, sizeof val);
				memcpy(&(this->array_time[count]), &val, sizeof val);
				count++;
			}
			for (int i = 0; i < this->size; i++) {
				printf("%s %f| ", array_time[i].player_name, array_time[i].time);
			}
			printf("\n");
			SDL_qsort(array_moves, count, sizeof(score), cmp_by_moves);
			SDL_qsort(array_time, count, sizeof(score), cmp_by_time);
			for (int i = 0; i < this->size; i++) {
				printf("%s %f| ", array_time[i].player_name, array_time[i].time);
			}
		}
		fclose(fp);
	}
}