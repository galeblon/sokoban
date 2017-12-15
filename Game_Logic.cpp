#include"Game_Logic.h"


void actor::initialize(int x, int y, bool isPuppet, int vel = RUNNING_SPEED) {
	angle = 0;
	old_angle = 0;
	timer = 0.0;
	rot_vel = ROTATING_SPEED;
	mov_vel = vel;
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

int actor::process_input(directions direction, map* gameMap, actor* crate) {
	int	res;
	old_pos = pos;
	switch (direction) {
	case UP:
		res = checkCollision(gameMap, pos.x, pos.y - 1, true, UP);
		if (res > 0) {
			pos.y -= 1;
			angle = 0;
			if (res == 2) {
				animate_puppet(this->pos, gameMap, crate, UP); this->mov_vel = PUSHING_SPEED;
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
				animate_puppet(this->pos, gameMap, crate, DOWN); this->mov_vel = PUSHING_SPEED;
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
				animate_puppet(this->pos, gameMap, crate, LEFT); this->mov_vel = PUSHING_SPEED;
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
				animate_puppet(this->pos, gameMap, crate, RIGHT); this->mov_vel = PUSHING_SPEED;
			}
			return 1;
		}
		break;
	}
}

int actor::move(directions direction, map* gameMap, actor* crate) {
	if (this->mov_state == STOPPED) {
		if (this->angle != directionToAngle(direction)) {
			old_angle = angle;
			angle = directionToAngle(direction);
			mov_state = ROTATING;
			timer = 0;
		}else {
			if (process_input(direction, gameMap, crate)) {
				this->mov_state = MOVING;
				timer = 0;
				return 1;
			}
		}	
	}	
	return 0;
}

void actor::update(double delta, map* gameMap, actor* crate) {
	if (this->mov_state == ROTATING) {
		if (this->timer*rot_vel <= 1) {
			this->timer += delta;
		}else {
			this->mov_state = STOPPED;
			if (process_input(angleToDirection(this->angle), gameMap, crate)) {
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
			if(crate != NULL)this->mov_vel = RUNNING_SPEED;
		}
	}
}

void actor::draw(display* gameDisplay, SDL_Rect tile) {
	SDL_Rect tile_tmp = tile;
	double curr_angle = this->angle;
	if (this->mov_state == STOPPED && is_puppet)
		return;
	if (this->mov_state == ROTATING) {
		curr_angle = getRotation(this->old_angle, this->angle);
		curr_angle = rot_vel*curr_angle*this->timer + this->old_angle;
	}
	if (this->mov_state == MOVING) {
		tile.y += this->old_pos.y*tile.w;
		tile.x += this->old_pos.x*tile.w;
		getInterpolation(&tile, this->timer*mov_vel*tile.w, this->angle);
	}
	else {
		tile.y += this->pos.y*tile.w;
		tile.x += this->pos.x*tile.w;
	}
	if (is_puppet){
		tile_tmp.y += this->pos.y*tile.w;
		tile_tmp.x += this->pos.x*tile.w;
		getInterpolation(&tile_tmp, 1*tile.w, this->angle);
		SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[0], NULL, &tile_tmp);
	}
	SDL_RenderCopyEx(gameDisplay->renderer, gameDisplay->gameTextures[is_puppet? 2:4], NULL, &tile, curr_angle, NULL, SDL_FLIP_NONE);

}


void map::draw(display* gameDisplay, SDL_Rect tile) {
	SDL_Rect tmp_tile = tile;
	for (int i = 0; i < dimension.height; i++) {
		for (int j = 0; j < dimension.width; j++) {
			tile.y = tmp_tile.y + i*tile.w;
			tile.x = tmp_tile.x + j*tile.w;
			if (this->entity[i][j].type == WALL) {
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[1], NULL, &tile);
			}
			else if (this->entity[i][j].type == CRATE) {
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[2], NULL, &tile);
			}
			else if (this->entity[i][j].type == FLOOR) {
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[0], NULL, &tile);
			}
			else if (this->entity[i][j].type == GOAL) {
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[0], NULL, &tile);
				SDL_RenderCopy(gameDisplay->renderer, gameDisplay->gameTextures[3], NULL, &tile);
			}
		}
	}
}


map* loadMap(const char* fName, actor* player) {
	FILE* pMap = fopen(fName, "r");
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
				loadedMap->entity[i][j].type = GOAL;
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


SDL_Rect calculateTileDimension(map* gameMap) {
	SDL_Rect result_tile;
	float sizeByHeight = SCREEN_HEIGHT / gameMap->dimension.height;
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


void getInterpolation(SDL_Rect* tile, int delta, int direction) {
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
}