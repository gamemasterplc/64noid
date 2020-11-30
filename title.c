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

struct menu_option {
	char *name;
	void (*activate_func)();
};

static int cursor_pos;
static int cursor_delay;

void TitleInit()
{
	RenderSetSize(SCREEN_W, SCREEN_H);
	title_data = SpriteLoadFile("titlesprites.spr");
	logo_sprite = SpriteCreate(title_data);
	SpriteSetImage(logo_sprite, "logo");
	SpriteSetPos(logo_sprite, (SCREEN_W/2), 24);
	cursor_delay = 0;
}

static void StartGame()
{
	game_globals.score = 0;
	game_globals.map_num = 0;
	game_globals.num_lives = 5;
	game_globals.edit_mode = false;
	game_globals.update_high_score = false;
	SetNextStage(STAGE_NEXTMAP);
}

static void StartLevelEditor()
{
	game_globals.score = 0;
	game_globals.num_lives = 5;
	game_globals.edit_mode = true;
	game_globals.save_map = false;
	game_globals.update_high_score = false;
	SetNextStage(STAGE_MAPEDITOR);
}

static struct menu_option menu_options[] = {
	{ "Game", StartGame },
	{ "Level Editor", StartLevelEditor },
};

static int num_menu_options = sizeof(menu_options)/sizeof(struct menu_option);
static int options_y = SCREEN_H-24-(9*(sizeof(menu_options)/sizeof(struct menu_option)));

void TitleUpdate()
{
	if(cursor_delay == 0) {
		if(pad_data[0].stick_y >= CURSOR_DEADZONE) {
			cursor_pos--;
			if(cursor_pos < 0) {
				cursor_pos = num_menu_options-1;
			}
			cursor_delay = CURSOR_DELAY;
		}
		if(pad_data[0].stick_y <= -CURSOR_DEADZONE) {
			cursor_pos++;
			if(cursor_pos >= num_menu_options) {
				cursor_pos = 0;
			}
			cursor_delay = CURSOR_DELAY;
		}
	} else {
		cursor_delay--;
	}
	if(pad_data[0].trigger & (A_BUTTON|START_BUTTON)) {
		menu_options[cursor_pos].activate_func();
	}
	if((pad_data[0].button & (L_TRIG|R_TRIG)) == (L_TRIG|R_TRIG)) {
		game_globals.score = 0;
		game_globals.map_num = 0;
		game_globals.num_lives = 5;
		game_globals.edit_mode = false;
		game_globals.update_high_score = false;
		SetNextStage(STAGE_MAPSELECT);
	}
}

static void DrawMenuOptions()
{
	int i;
	for(i=0; i<num_menu_options; i++) {
		if(i == cursor_pos) {
			TextSetColor(0, 255, 0, 255);
		} else {
			TextSetColor(255, 255, 255, 255);
		}
		TextDraw(SCREEN_W/2, options_y+(i*9), TEXT_ALIGNMENT_CENTER, menu_options[i].name);
	}
}

void TitleDraw()
{
	RenderClear(0, 0, 0);
	SpriteDraw(logo_sprite);
	DrawMenuOptions();
}

void TitleDestroy()
{
	SpriteFreeData(title_data);
	SpriteDelete(logo_sprite);
}