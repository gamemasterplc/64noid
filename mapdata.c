#include "map.h"

static char map1_data[MAP_WIDTH*MAP_HEIGHT] = {
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

static char map2_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"h........."
	"hg........"
	"hgf......."
	"hgfe......"
	"hgfed....."
	"hgfedc...."
	"hgfedcb..."
	"hgfedcba.."
	"hgfedcbah."
	"iiiiiiiiii"
	".........."
	".........."
	".........."
	".........."
	".........."
};

static char map3_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"aeaeaeaeae"
	".........."
	".........."
	"..iiiiiiii"
	".........."
	".........."
	"iiiiiiii.."
	".........."
	".........."
	"..iiiiiiii"
	".........."
	".........."
	"iiiiiiii.."
	".........."
	".........."
};

char *mapdata_table[] = {
	map1_data,
	map2_data,
	map3_data,
};

int num_maps = sizeof(mapdata_table)/sizeof(char *);