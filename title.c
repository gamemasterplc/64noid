#include <ultra64.h>
#include "mapsel.h"
#include "render.h"
#include "main.h"
#include "pad.h"
#include "game.h"
#include "map.h"
#include "text.h"
#include "bool.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define CURSOR_DEADZONE 32
#define CURSOR_DELAY 12

static SpriteData *title_data;
static SpriteInfo *logo_sprite;

void TitleInit()
{
	RenderSetSize(SCREEN_W, SCREEN_H);
	title_data = SpriteLoadFile("titlesprites.spr");
	logo_sprite = SpriteCreate(title_data);
	SpriteSetImage(logo_sprite, "logo");
	SpriteSetPos(logo_sprite, (SCREEN_W/2), 24);
}

void TitleUpdate()
{
	if(pad_data[0].trigger & START_BUTTON) {
		game_globals.score = 0;
		game_globals.map_num = 0;
		game_globals.num_lives = 5;
		game_globals.edit_mode = false;
		SetNextStage(STAGE_NEXTMAP);
	}
	if((pad_data[0].button & (L_TRIG|R_TRIG)) == (L_TRIG|R_TRIG)) {
		SetNextStage(STAGE_MAPEDITOR);
	}
}

void TitleDraw()
{
	RenderClear(0, 0, 0);
	SpriteDraw(logo_sprite);
	TextSetColor(255, 255, 255, 255);
	TextDraw(SCREEN_W/2, SCREEN_H-33, TEXT_ALIGNMENT_CENTER, "Press Start");
}

void TitleDestroy()
{
	SpriteFreeData(title_data);
	SpriteDelete(logo_sprite);
}