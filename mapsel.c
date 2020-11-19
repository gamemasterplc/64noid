#include <ultra64.h>
#include "mapsel.h"
#include "render.h"
#include "main.h"
#include "pad.h"
#include "game.h"
#include "map.h"
#include "text.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define CURSOR_DEADZONE 32
#define CURSOR_DELAY 12

static int cursor_pos;
static int cursor_timer;
static int num_lives_temp;
static int map_num_temp;
static bool start_stage;

void MapSelectInit()
{
	RenderSetSize(SCREEN_W, SCREEN_H);
	map_num_temp = 0;
	num_lives_temp = 5;
	cursor_pos = 0;
	cursor_timer = 0;
	start_stage = false;
}

void MapSelectUpdate()
{
	if(pad_data[0].stick_y < -CURSOR_DEADZONE && cursor_timer == 0) {
		cursor_pos--;
		cursor_timer = CURSOR_DELAY;
		if(cursor_pos < 0) {
			cursor_pos = 1;
		}
	}
	if(pad_data[0].stick_y > CURSOR_DEADZONE && cursor_timer == 0) {
		cursor_pos++;
		cursor_timer = CURSOR_DELAY;
		if(cursor_pos > 1) {
			cursor_pos = 0;
		}
	}
	if(pad_data[0].stick_y < -CURSOR_DEADZONE && cursor_timer == 0) {
		cursor_pos--;
		cursor_timer = CURSOR_DELAY;
		if(cursor_pos < 0) {
			cursor_pos = 1;
		}
	}
	if(pad_data[0].stick_x > CURSOR_DEADZONE && cursor_timer == 0) {
		cursor_timer = CURSOR_DELAY;
		switch(cursor_pos) {
			case 0:
				map_num_temp++;
				if(map_num_temp >= num_maps) {
					map_num_temp = 0;
				}
				break;
				
			case 1:
				num_lives_temp++;
				if(num_lives_temp > 99) {
					num_lives_temp = 99;
				}
				break;
		}
	}
	if(pad_data[0].stick_x < -CURSOR_DEADZONE && cursor_timer == 0) {
		cursor_timer = CURSOR_DELAY;
		switch(cursor_pos) {
			case 0:
				map_num_temp--;
				if(map_num_temp < 0) {
					map_num_temp = num_maps-1;
				}
				break;
				
			case 1:
				num_lives_temp--;
				if(num_lives_temp < 1) {
					num_lives_temp = 1;
				}
				break;
		}
	}
	if(cursor_timer > 0) {
		cursor_timer--;
	}
	if(pad_data[0].trigger & START_BUTTON) {
		game_globals.score = 0;
		game_globals.map_num = map_num_temp;
		game_globals.num_lives = num_lives_temp;
		SetNextStage(STAGE_GAME);
	}
}

static void DrawMapSelectText()
{
	char text_buf[64];
	if(cursor_pos == 0) {
		TextSetColor(0, 255, 0, 255);
	} else {
		TextSetColor(255, 255, 255, 255);
	}
	sprintf(text_buf, "Map %d", map_num_temp+1);
	TextDraw(SCREEN_W/2, (SCREEN_H/2)-9, TEXT_ALIGNMENT_CENTER, text_buf);
	if(cursor_pos == 1) {
		TextSetColor(0, 255, 0, 255);
	} else {
		TextSetColor(255, 255, 255, 255);
	}
	sprintf(text_buf, "Lives %d", num_lives_temp);
	TextDraw(SCREEN_W/2, SCREEN_H/2, TEXT_ALIGNMENT_CENTER, text_buf);
}

void MapSelectDraw()
{
	RenderClear(0, 0, 0);
	DrawMapSelectText();
}