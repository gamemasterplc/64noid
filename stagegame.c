#include <PR/ultratypes.h>
#include "render.h"
#include "controller.h"
#include "bool.h"

#define MAP_WIDTH 13
#define MAP_HEIGHT 29
#define MAP_X_OFS 56
#define MAP_BLOCK_W 16
#define MAP_BLOCK_H 8
#define BALL_W 8
#define BALL_H 8
#define PADDLE_W 32
#define PADDLE_H 8

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

static float ball_x, ball_y;
static float paddle_x, paddle_y;
static float ball_vel_x, ball_vel_y;

static void ResetField()
{
	ball_x = MAP_WIDTH*MAP_BLOCK_W/2;
	ball_y = (MAP_HEIGHT*MAP_BLOCK_H)-64;
	ball_vel_x = 0.5f;
	ball_vel_y = 1.0f;
	paddle_x = MAP_WIDTH*MAP_BLOCK_W/2;
}

void StageGameInit()
{
	RenderSetSize(320, 240);
	ResetField();
	paddle_y = (MAP_HEIGHT*MAP_BLOCK_H)-8;
}
 
static bool TestMapCollision(int x, int y)
{
	int map_x = x/MAP_BLOCK_W;
	int map_y = y/MAP_BLOCK_H;
	if(map_x >= 0 && map_x < MAP_WIDTH && map_y >= 0 && map_y < MAP_HEIGHT) {
		if(map_data[(map_y*MAP_WIDTH)+map_x] != '.') {
			map_data[(map_y*MAP_WIDTH)+map_x] = '.';
			return true;
		}
	}
	return false;
}

static void UpdateBall()
{
	int left, top, right, bottom;
	ball_x += ball_vel_x;
	ball_y += ball_vel_y;
	if(ball_x >= MAP_WIDTH*MAP_BLOCK_W-(BALL_W/2)) {
		ball_vel_x = -ball_vel_x;
	}
	if(ball_x < (BALL_W/2)) {
		ball_vel_x = -ball_vel_x;
	}
	if(ball_y < (BALL_H/2)) {
		ball_vel_y = -ball_vel_y;
	}
	if(ball_y >= paddle_y && ball_x >= (paddle_x-(PADDLE_W/2)) && ball_x < (paddle_x+(PADDLE_W/2)) && ball_vel_y > 0) {
		ball_vel_x = ((ball_x-paddle_x)/(PADDLE_W/2))*1.5f;
		ball_vel_y = -ball_vel_y;
	}
	if(ball_y >= MAP_HEIGHT*MAP_BLOCK_H) {
		ResetField();
	}
	left = ball_x-(BALL_W/2);
	right = ball_x+(BALL_W/2);
	top = ball_y-(BALL_H/2);
	bottom = ball_y+(BALL_H/2);
	if(TestMapCollision(left, top) || TestMapCollision(right, top) || TestMapCollision(left, bottom) || TestMapCollision(right, bottom)) {
		ball_vel_y = -ball_vel_y;
	}
}

void StageGameUpdate()
{
	paddle_x += (cont_data[0].stick_x/10);
	if(paddle_x >= MAP_WIDTH*MAP_BLOCK_W-(PADDLE_W/2)) {
		paddle_x = MAP_WIDTH*MAP_BLOCK_W-(PADDLE_W/2);
	}
	if(paddle_x < (PADDLE_W/2)) {
		paddle_x = (PADDLE_W/2);
	}
	UpdateBall();
}

static void DrawMap()
{
	int i, j;
	for(i=0; i<MAP_HEIGHT; i++) {
		for(j=0; j<MAP_WIDTH; j++) {
			char value = map_data[(i*MAP_WIDTH)+j];
			if(value != '.') {
				RenderPutRect((j*MAP_BLOCK_W)+MAP_X_OFS, (i*MAP_BLOCK_H), MAP_BLOCK_W, MAP_BLOCK_H, block_cols[value-'a'][0], block_cols[value-'a'][1],
					block_cols[value-'a'][2], 255);
			}
		}
	}
}

static void DrawBall()
{
	RenderPutRect(ball_x+MAP_X_OFS-(BALL_W/2), ball_y-(BALL_H/2), BALL_W, BALL_H, 255, 255, 255, 255);
}

static void DrawPaddle()
{
	RenderPutRect(paddle_x+MAP_X_OFS-(PADDLE_W/2), paddle_y, PADDLE_W, PADDLE_H, 255, 255, 255, 255);
}

void StageGameDraw()
{
    RenderStartFrame();
	RenderClear(0, 0, 0);
	DrawMap();
	DrawBall();
	DrawPaddle();
	RenderEndFrame();
}

void StageGameDestroy()
{
	
}