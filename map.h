#ifndef MAP_H
#define MAP_H

#include "sprite.h"

#define MAP_WIDTH 10
#define MAP_HEIGHT 17
#define MAP_X_OFS 16
#define MAP_Y_OFS 16
#define MAP_BRICK_W 20
#define MAP_BRICK_H 10
#define BRICK_EMPTY '.'
#define BRICK_START 'a'
#define BRICK_ROCK3 'i'
#define BRICK_ROCK2 'j'
#define BRICK_ROCK1 'k'
#define BRICK_GOLD 'l'

typedef struct map_brick {
	char type;
	int x;
	int y;
	SpriteInfo sprite;
} MapBrick;

void MapLoad(int map_id);
MapBrick *MapGetBrick(int x, int y);
void MapDestroyBrick(MapBrick *brick);
void MapUnload();
int MapGetNumBricks();
void MapDraw();


#endif