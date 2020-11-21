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

static char map4_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"defgh.bcde"
	"efgh.bcdef"
	"fghab.defg"
	"ghab.defgh"
	"habcd.fgha"
	"abcd.fghab"
	"bcdef.habc"
	"cdef.habcd"
	"defgh.bcde"
	"efgh.bcdef"
	"fghab.defg"
	"ghab.defgh"
	"habcd.fgha"
	"abcd.fghab"
	"bcdef.habc"
};

static char map5_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"..f....f.."
	"..f....f.."
	"...f..f..."
	"...f..f..."
	"..cccccc.."
	"..cccccc.."
	"cchcccchcc"
	"cchcccchcc"
	"cccccccccc"
	"cccccccccc"
	"c.cccccc.c"
	"c.cccccc.c"
	".cc....cc."
	".cc....cc."
	"...c..c..."
};

char *mapdata_table[] = {
	map1_data,
	map2_data,
	map3_data,
	map4_data,
	map5_data,
};

int num_maps = sizeof(mapdata_table)/sizeof(char *);