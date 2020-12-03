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
	"cclcccclcc"
	"cclcccclcc"
	"cccccccccc"
	"cccccccccc"
	"c.cccccc.c"
	"c.cccccc.c"
	".cc....cc."
	".cc....cc."
	"...c..c..."
};

static char map6_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"....bb...."
	"....bb...."
	"...bbbb..."
	"...bbbb..."
	"..bbbbbb.."
	"..bbbbbb.."
	".bbliblib."
	".bbliblib."
	".bbiibiib."
	".bbiibiib."
	"..b.bb.b.."
	".b......b."
	".b......b."
	"..b....b.."
	"..b....b.."
};

static char map7_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"....ff...."
	"...ffff..."
	"..ffggff.."
	"..fggggf.."
	".ffgghgff."
	".ffgghgff."
	"ffghcchgff"
	"ffgh..hgff"
	"ffg....gff"
	"ff......ff"
	"ff......ff"
	"ff......ff"
	"ff......ff"
	"iii.ii.iii"
	".........."
};

static char map8_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"....b....."
	"....b....."
	"....b....."
	"....bbbbbb"
	".bbbb...b."
	"h...h..h.."
	"h...h.h..."
	"h...h..h.."
	"h...h...h."
	"d...d....d"
	".dddd....d"
	"....d....d"
	".....d..d."
	"......bb.."
	".........."
};

static char map9_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"aaaaaaaaaa"
	".ffffffff."
	"..aaaaaa.."
	"...ffff..."
	"h...ff...h"
	"hc......ch"
	"hch.ll.hch"
	"hchc..chch"
	"hch.aa.hch"
	"hc.ffff.ch"
	"h.aaaaaa.h"
	".ffffffff."
	"aaaaaaaaaa"
	".ffffffff."
	"..aaaaaa.."
};

static char map10_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	".iiiiiiii."
	".iiiiiiii."
	".i......i."
	".icc..cci."
	".ic....ci."
	".i..ee..i."
	".i.elle.i."
	".iellllei."
	".i.elle.i."
	".i..ee..i."
	".ic....ci."
	".icc..cci."
	".i......i."
	".iiiiiiii."
	".iiiiiiii."
};

static char map11_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	".........."
	"..l....l.."
	"..l....l.."
	"..ililll.."
	"..fedcba.."
	"..fedcba.."
	"..fedcba.."
	"..fedcba.."
	"..fedcba.."
	"..fedcba.."
	"..ililll.."
	"..l....l.."
	"..l....l.."
	".........."
	".........."
};

char *mapdata_table[] = {
	map1_data,
	map2_data,
	map3_data,
	map4_data,
	map5_data,
	map6_data,
	map7_data,
	map8_data,
	map9_data,
	map10_data,
	map11_data
};

int num_maps = sizeof(mapdata_table)/sizeof(char *);