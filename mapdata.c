#include "map.h"

static char map1_data[MAP_WIDTH*MAP_HEIGHT] = {
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
	"hgfedcbahg"
	".........."
	".........."
	".........."
	".........."
	".........."
	".........."
};

char *mapdata_table[] = {
	map1_data,
};