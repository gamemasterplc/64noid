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
	"..iliili.."
	"..fedcba.."
	"..fedcba.."
	"..fedcba.."
	"..fedcba.."
	"..fedcba.."
	"..fedcba.."
	"..iliili.."
	"..l....l.."
	"..l....l.."
	".........."
	".........."
};

static char map12_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	"l........l"
	"ll......ll"
	".ll....ll."
	"h.ll..ll.h"
	"hh..ll..hh"
	"chh....chh"
	"cchh..cchh"
	"fcchhcchhf"
	"ffchhcchff"
	".ffhhccff."
	"..ffhcff.."
	"...ffff..."
	"....ff...."
	".........."
	".........."
};

static char map13_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	".ii.ii.ii."
	".ie.id.ic."
	".ii.ii.ii."
	".........."
	".........."
	".ii.ii.ii."
	".ih.il.ib."
	".ii.ii.ii."
	".........."
	".........."
	".ii.ii.ii."
	".ig.if.ia."
	".ii.ii.ii."
	".........."
	".........."
};

static char map14_data[MAP_WIDTH*MAP_HEIGHT] = {
	"l........l"
	"...e.f.d.."
	"..h.c.b.c."
	"...e.f.d.."
	"..h.c.b.c."
	"...e.f.d.."
	"..h.c.b.c."
	"...e.f.d.."
	"..h.c.b.c."
	"...e.f.d.."
	"..h.c.b.c."
	"...e.f.d.."
	"..h.c.b.c."
	"...e.f.d.."
	"..h.c.b.c."
	"l........l"
};

static char map15_data[MAP_WIDTH*MAP_HEIGHT] = {
	".........."
	".f.l..l.f."
	".l......l."
	"..ff......"
	".fiff....."
	"affff....a"
	"lffffcc..l"
	".ffffcc..."
	".ffffccc.."
	"..ffcccc.."
	".aaacccc.."
	"aaaacccc.a"
	"laaaaccc.l"
	".aaaa....."
	".l.c..c.l."
	"...c..c..."
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
	map11_data,
	map12_data,
	map13_data,
	map14_data,
	map15_data
};

int num_maps = sizeof(mapdata_table)/sizeof(char *);