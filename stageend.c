#include <ultra64.h>
#include "stageend.h"
#include "render.h"
#include "pad.h"
#include "sprite.h"
#include "game.h"
#include "map.h"
#include "main.h"
#include "save.h"
#include "text.h"

#define SCREEN_W 320
#define SCREEN_H 240

static SpriteData *endstage_data;
static SpriteInfo *endtext_sprite;

void StageEndInit()
{
	RenderSetSize(SCREEN_W, SCREEN_H);
	endstage_data = SpriteLoadFile("endstagesprites.spr");
	endtext_sprite = SpriteCreate(endstage_data);
	if(game_globals.num_lives == 0) {
		SpriteSetImage(endtext_sprite, "gameover");
	} else {
		SpriteSetImage(endtext_sprite, "youwin");
	}
	SpriteSetPos(endtext_sprite, SCREEN_W/2, 24);
	if(game_globals.update_high_score) {
		SaveWrite();
	}
}

void StageEndUpdate()
{
	if(pad_data[0].trigger & A_BUTTON) {
		SetNextStage(STAGE_TITLE);
	}
}

void StageEndDraw()
{
	RenderClear(0, 0, 0);
	SpriteDraw(endtext_sprite);
	TextSetColor(255, 255, 255, 255);
	TextDraw(SCREEN_W/2, SCREEN_H-33, TEXT_ALIGNMENT_CENTER, "Press A to return to the title screen.");
}

void StageEndDestroy()
{
	SpriteFreeData(endstage_data);
	SpriteDelete(endtext_sprite);
}