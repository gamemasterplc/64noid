#include <ultra64.h>
#include "nextmap.h"
#include "render.h"
#include "sprite.h"
#include "main.h"
#include "save.h"
#include "text.h"

#define SCREEN_W 320
#define SCREEN_H 240

static SpriteData *gameover_data;
static SpriteInfo *gameover_sprite;

void GameOverInit()
{
	RenderSetSize(SCREEN_W, SCREEN_H);
	gameover_data = SpriteLoadFile("gameover.spr");
	gameover_sprite = SpriteCreate(gameover_data);
	SpriteSetImage(gameover_sprite, "gameover");
	SpriteSetPos(gameover_sprite, SCREEN_W/2, 24);
}

void GameOverUpdate()
{
	
}

void GameOverDraw()
{
	RenderClear(0, 0, 0);
	SpriteDraw(gameover_sprite);
	TextSetColor(255, 255, 255, 255);
	TextDraw(SCREEN_W/2, SCREEN_H-33, TEXT_ALIGNMENT_CENTER, "Press A to return to the title screen.");
}

void GameOverDestroy()
{
	SpriteFreeData(gameover_data);
	SpriteDelete(gameover_sprite);
}