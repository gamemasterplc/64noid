#include <PR/ultratypes.h>
#include "render.h"
#include "controller.h"

#define MAP_WIDTH 13
#define MAP_HEIGHT 29
#define MAP_X_OFS 56

static char map_data[MAP_WIDTH*MAP_HEIGHT] = {
	"............."
	"h............"
	"hg..........."
	"hgf.........."
	"hgfe........."
	"hgfed........"
	"hgfedc......."
	"hgfedcb......"
	"hgfedcba....."
	"hgfedcbah...."
	"hgfedcbahg..."
	"hgfedcbahgf.."
	"hgfedcbahgfe."
	"iiiiiiiiiiiii"
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
	"............."
};

u8 block_cols[][3] = {
	{ 38, 217, 95 },
	{ 27, 188, 155 },
	{ 40, 136, 229 },
	{ 155, 88, 181 },
	{ 226, 93, 181 },
	{ 237, 65, 52 },
	{ 231, 126, 35 },
	{ 241, 196, 15 },
	{ 190, 195, 199 },
};

void StageGameInit()
{
	RenderSetSize(320, 240);
}

void StageGameUpdate()
{
	
}

static void DrawMap()
{
	int i, j;
	for(i=0; i<MAP_HEIGHT; i++) {
		for(j=0; j<MAP_WIDTH; j++) {
			char value = map_data[(i*MAP_WIDTH)+j];
			if(value != '.') {
				RenderPutRect((j*16)+MAP_X_OFS, i*8, 16, 8,  block_cols[value-'a'][0], block_cols[value-'a'][1],
					block_cols[value-'a'][2], 255);
			}
		}
	}
}

void StageGameDraw()
{
    RenderStartFrame();
	RenderClear(0, 0, 0);
	DrawMap();
	RenderEndFrame();
}

void StageGameDestroy()
{
	
}