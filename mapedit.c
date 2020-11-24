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
#define UI_BASE_POS_X (MAP_X_OFS+(MAP_WIDTH*MAP_BRICK_W)+12)
#define UI_CENTER_POS_X (UI_BASE_POS_X+((SCREEN_W-UI_BASE_POS_X)/2)-12)
#define UI_RIGHT_POS_X (SCREEN_W-24)
#define CURSOR_DEADZONE 32
#define INPUT_REPEAT_DELAY 12

static SpriteData *mapedit_sprite_data;
static SpriteInfo *sprite_cursor;
static SpriteInfo *sprite_brick;
static SpriteInfo *sprite_l_button;
static SpriteInfo *sprite_r_button;
static SpriteInfo *sprite_border;
static SpriteInfo *sprite_c_left;
static SpriteInfo *sprite_c_right;
static int cursor_x, cursor_y;
static int cursor_delay;
static int brick_select_delay;
static int brick_type;

static char *brick_images[] = {
	"brick_white",
	"brick_orange",
	"brick_cyan",
	"brick_green",
	"brick_red",
	"brick_blue",
	"brick_magenta",
	"brick_yellow",
	"brick_rock3",
	"brick_gold"
};

static char brick_types[] = { '.', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'l' };

static void UpdateCursorSpritePos()
{
	SpriteSetPos(sprite_cursor, (cursor_x*MAP_BRICK_W)+MAP_X_OFS+(MAP_BRICK_W/2), (cursor_y*MAP_BRICK_H)+MAP_Y_OFS+(MAP_BRICK_H/2));
}

static void CursorInit()
{
	sprite_cursor = SpriteCreate(mapedit_sprite_data);
	SpriteSetAnim(sprite_cursor, "cursor_pulsate");
	cursor_x = MAP_WIDTH/2;
	cursor_y = MAP_HEIGHT/2;
	UpdateCursorSpritePos();
}

static void SetBrickType(int type)
{
	brick_type = type;
	if(type) {
		SpriteSetImage(sprite_brick, brick_images[type-1]);
	}
}

static void BrickInit()
{
	sprite_brick = SpriteCreate(mapedit_sprite_data);
	sprite_l_button = SpriteCreate(mapedit_sprite_data);
	sprite_r_button = SpriteCreate(mapedit_sprite_data);
	SpriteSetPos(sprite_brick, UI_CENTER_POS_X, 64);
	SetBrickType(2);
	SpriteSetPos(sprite_l_button, UI_BASE_POS_X+8, 64);
	SpriteSetImage(sprite_l_button, "button_l");
	SpriteSetPos(sprite_r_button, UI_RIGHT_POS_X-8, 64);
	SpriteSetImage(sprite_r_button, "button_r");
}

static void LevelButtonInit()
{
	sprite_c_left = SpriteCreate(mapedit_sprite_data);
	sprite_c_right = SpriteCreate(mapedit_sprite_data);
	SpriteSetPos(sprite_c_left, UI_BASE_POS_X+4, 28);
	SpriteSetImage(sprite_c_left, "button_c_left");
	SpriteSetPos(sprite_c_right, UI_RIGHT_POS_X-4, 28);
	SpriteSetImage(sprite_c_right, "button_c_right");
}

static void BorderInit()
{
	sprite_border = SpriteCreate(mapedit_sprite_data);
	SpriteSetPos(sprite_border, MAP_X_OFS-8, MAP_Y_OFS-8);
	SpriteSetImage(sprite_border, "border");
}

void MapEditorInit()
{
	RenderSetSize(SCREEN_W, SCREEN_H);
	MapLoadSave(game_globals.map_num);
	mapedit_sprite_data = SpriteLoadFile("mapeditsprites.spr");
	BrickInit();
	CursorInit();
	LevelButtonInit();
	BorderInit();
}

void MapEditorUpdate()
{
	
}

static DrawMapNumber()
{
	char text_buf[64];
	sprintf(text_buf, "Level %d", game_globals.map_num+1);
	TextDraw(UI_CENTER_POS_X, 24, TEXT_ALIGNMENT_CENTER, text_buf);
}

void MapEditorDraw()
{
	RenderClear(0, 0, 0);
	MapDraw();
	SpriteDraw(sprite_border);
	SpriteDraw(sprite_cursor);
	SpriteDraw(sprite_brick);
	SpriteDraw(sprite_l_button);
	SpriteDraw(sprite_r_button);
	DrawMapNumber();
	SpriteDraw(sprite_c_left);
	SpriteDraw(sprite_c_right);
}

void MapEditorDestroy()
{
	MapUnload();
	SpriteFreeData(mapedit_sprite_data);
	SpriteDelete(sprite_border);
	SpriteDelete(sprite_cursor);
	SpriteDelete(sprite_brick);
	SpriteDelete(sprite_l_button);
	SpriteDelete(sprite_r_button);
	SpriteDelete(sprite_c_left);
	SpriteDelete(sprite_c_right);
}