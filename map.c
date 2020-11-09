#include <stdlib.h>
#include <malloc.h>
#include "map.h"
#include "bool.h"

extern char *mapdata_table[];

static MapBrick *brick_map;
static SpriteData *brick_spr_data;
static int num_bricks;

static char *brick_image_names[BRICK_GOLD-BRICK_START+1] = {
	"brick_white",
	"brick_orange",
	"brick_cyan",
	"brick_green",
	"brick_red",
	"brick_blue",
	"brick_magenta",
	"brick_yellow",
	"brick_rock3",
	"brick_rock2",
	"brick_rock1",
	"brick_gold"
};

static void LoadMapBricks(char *src)
{
	int i;
	for(i=0; i<MAP_WIDTH*MAP_HEIGHT; i++) {
		int x = ((i%MAP_WIDTH)*MAP_BRICK_W);
		int y = ((i/MAP_WIDTH)*MAP_BRICK_H);
		brick_map[i].type = src[i];
		brick_map[i].x = x;
		brick_map[i].y = y;
		if(brick_map[i].type != BRICK_EMPTY) {
			SpriteInit(&brick_map[i].sprite, brick_spr_data);
			SpriteSetImage(&brick_map[i].sprite, brick_image_names[brick_map[i].type-BRICK_START]);
			SpriteSetPos(&brick_map[i].sprite, x+MAP_X_OFS+(MAP_BRICK_W/2), y+MAP_Y_OFS+(MAP_BRICK_H/2));
		}
	}
}

static int GetDefaultNumBricks()
{
	int count = 0;
	int i;
	for(i=0; i<MAP_WIDTH*MAP_HEIGHT; i++) {
		if(brick_map[i].type != BRICK_EMPTY && brick_map[i].type != BRICK_GOLD) {
			count++;
		}
	}
	return count;
}

void MapLoad(int map_id)
{
	brick_spr_data = SpriteLoadFile("bricksprites.spr");
	brick_map = malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(MapBrick));
	LoadMapBricks(mapdata_table[map_id]);
	num_bricks = GetDefaultNumBricks();
}

MapBrick *MapGetBrick(int x, int y)
{
	if(x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT) {
		return &brick_map[(y*MAP_WIDTH)+x];
	}
	return NULL;
}

void MapDestroyBrick(MapBrick *brick)
{
	switch(brick->type) {
		case BRICK_GOLD:
			return;
			
		case BRICK_ROCK3:
			brick->type = BRICK_ROCK2;
			break;
			
		case BRICK_ROCK2:
			brick->type = BRICK_ROCK1;
			break;
			
		default:
			brick->type = BRICK_EMPTY;
			num_bricks--;
			break;
	}
	if(brick->type != BRICK_EMPTY) {
		SpriteSetImage(&brick->sprite, brick_image_names[brick->type-BRICK_START]);
	}
}

void MapUnload()
{
	SpriteFreeData(brick_spr_data);
	brick_spr_data = NULL;
	free(brick_map);
}

int MapGetNumBricks()
{
	return num_bricks;
}

void MapDraw()
{
	int i;
	for(i=0; i<MAP_WIDTH*MAP_HEIGHT; i++) {
		if(brick_map[i].type != BRICK_EMPTY) {
			SpriteDraw(&brick_map[i].sprite);
		}
	}
}