#include <ultra64.h>
#include "nextmap.h"
#include "render.h"
#include "main.h"
#include "game.h"
#include "save.h"
#include "text.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define NEXT_MAP_WAIT_TIME 120

static int timer;

void NextMapInit()
{
	RenderSetSize(SCREEN_W, SCREEN_H);
	timer = NEXT_MAP_WAIT_TIME;
}

void NextMapUpdate()
{
	if(--timer == 0) {
		SetNextStage(STAGE_GAME);
	}
}

static void DrawNextMapText()
{
	char text_buf[64];
	TextSetColor(255, 255, 255, 255);
	sprintf(text_buf, "Level %d", game_globals.map_num+1);
	TextDraw(SCREEN_W/2, (SCREEN_H/2)-5, TEXT_ALIGNMENT_CENTER, text_buf);
	TextDraw(36, 24, TEXT_ALIGNMENT_CENTER, "Lives");
	sprintf(text_buf, "%d", game_globals.num_lives);
	TextDraw(36, 33, TEXT_ALIGNMENT_CENTER, text_buf);
	TextDraw(SCREEN_W/2, 24, TEXT_ALIGNMENT_CENTER, "High Score");
	sprintf(text_buf, "%d", save_data->high_score);
	TextDraw(SCREEN_W/2, 33, TEXT_ALIGNMENT_CENTER, text_buf);
}

void NextMapDraw()
{
	RenderClear(0, 0, 0);
	DrawNextMapText();
}