#include "map.h"

static char map1_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"....gg...."
	"...ffff..."
	"..eeeeee.."
	"..dddddd.."
	"...cccc..."
	"....bb...."
	".........."
	".........."
	".........."
	".........."
	".........."
	".........."
	".........."
	".........."
	".........."
};

static char map2_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	".aaaaaaaa."
	".bbbbbbbb."
	".cccccccc."
	".dddddddd."
	".eeeeeeee."
	".ffffffff."
	".gggggggg."
	".hhhhhhhh."
	".........."
	".........."
	".........."
	".........."
	".........."
	".........."
	".........."
};

char *mapdata_table[] = {
	map1_data,
	map2_data,
};

int num_maps = sizeof(mapdata_table)/sizeof(char *);