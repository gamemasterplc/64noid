#include <PR/ultratypes.h>
#include <PR/gu.h>
#include "render.h"
#include "pad.h"
#include "map.h"
#include "sprite.h"
#include "text.h"
#include "bool.h"

#define MAX_BALLS 3
#define MAX_POWERUP_ITEMS 5
#define BALL_VELOCITY 2.5
#define PADDLE_VELOCITY 3.5

#define HIT_SIDE_TOP 0
#define HIT_SIDE_BOTTOM 1
#define HIT_SIDE_LEFT 2
#define HIT_SIDE_RIGHT 3

#define POWERUP_LASER 0
#define POWERUP_GROW_PADDLE 1
#define POWERUP_SHRINK_PADDLE 2
#define POWERUP_STICK 3
#define POWERUP_SLOW_BALL 4
#define POWERUP_TRIPLE_BALL 4
#define POWERUP_EXTRA_LIFE 5
#define POWERUP_BIG_BALL 6

typedef struct paddle {
	float x;
	float y;
	float w;
	float h;
	SpriteInfo sprite;
} Paddle;

typedef struct ball {
	bool exists;
	float x;
	float y;
	float radius;
	float vel_x;
	float vel_y;
	Paddle *glued_to_paddle;
	float glued_pos;
	SpriteInfo sprite;
} Ball;

typedef struct powerup_item {
	bool exists;
	int type;
	float x;
	float y;
	SpriteInfo sprite;
} PowerUpItem;

static Ball balls[MAX_BALLS];
static PowerUpItem powerup_item[MAX_POWERUP_ITEMS];
static Paddle paddle;
static int num_balls;
static SpriteData *game_sprites;
static SpriteInfo *border_sprite;
static SpriteInfo *field_bg_sprite;

static void InitBalls()
{
	int i;
	for(i=0; i<MAX_BALLS; i++) {
		balls[i].exists = false;
		SpriteInit(&balls[i].sprite, game_sprites);
		SpriteSetImage(&balls[i].sprite, "ball_blue");
	}
	num_balls = 0;
}

static void InitPaddle()
{
	int i;
	paddle.x = (MAP_WIDTH*MAP_BRICK_W/2);
	paddle.y = 200;
	paddle.w = 40;
	paddle.h = 8;
	SpriteInit(&paddle.sprite, game_sprites);
	SpriteSetImage(&paddle.sprite, "paddle");
	SpriteSetPos(&paddle.sprite, paddle.x+MAP_X_OFS, paddle.y+MAP_Y_OFS);
}

static void CreateBall()
{
	int i, j;
	float angle;
	for(i=0; i<MAX_BALLS; i++) {
		if(!balls[i].exists) {
			balls[i].exists = true;
			break;
		}
	}
	if(i == MAX_BALLS) {
		return;
	}
	balls[i].radius = 4;
	balls[i].y = paddle.y-(paddle.h/2)-balls[i].radius;
	angle = M_DTOR*((rand()%90)+30);
	balls[i].vel_x = BALL_VELOCITY*cosf(angle);
	balls[i].vel_y = -BALL_VELOCITY*sinf(angle);
	balls[i].glued_to_paddle = NULL;
	balls[i].glued_pos = 0;
	balls[i].x = paddle.x+balls[i].glued_pos;
	SpriteSetPos(&balls[i].sprite, balls[i].x+MAP_X_OFS, balls[i].y+MAP_Y_OFS);
	num_balls++;
}

void StageGameInit()
{
	RenderSetSize(320, 240);
	game_sprites = SpriteLoadFile("gamesprites.spr");
	InitBalls();
	InitPaddle();
	CreateBall();
	MapLoad(0);
	border_sprite = SpriteCreate(game_sprites);
	SpriteSetImage(border_sprite, "border");
	SpriteSetPos(border_sprite, MAP_X_OFS-8, MAP_Y_OFS-8);
	field_bg_sprite = SpriteCreate(game_sprites);
	SpriteSetImage(field_bg_sprite, "field_bg");
	SpriteSetPos(field_bg_sprite, MAP_X_OFS, MAP_Y_OFS);
}

static bool TestPaddleCollision(Ball *ball)
{
	if((ball->y+ball->radius > paddle.y-(paddle.h/2)) &&
		(ball->y-ball->radius < paddle.y+(paddle.h/2)) &&
		(ball->x+ball->radius > paddle.x-(paddle.w/2)) &&
		(ball->x-ball->radius < paddle.x+(paddle.w/2))) {
			return true;
		}
	return false;
}

static bool TestBrickCollision(Ball *ball, int side)
{
	MapBrick *brick;
	switch(side) {
		case HIT_SIDE_TOP:
			brick = MapGetBrick(ball->x/MAP_BRICK_W, (ball->y-ball->radius)/MAP_BRICK_H);
			break;
			
		case HIT_SIDE_BOTTOM:
			brick = MapGetBrick(ball->x/MAP_BRICK_W, (ball->y+ball->radius)/MAP_BRICK_H);
			break;
			
		case HIT_SIDE_LEFT:
			brick = MapGetBrick((ball->x-ball->radius)/MAP_BRICK_W, ball->y/MAP_BRICK_H);
			break;
			
		case HIT_SIDE_RIGHT:
			brick = MapGetBrick((ball->x+ball->radius)/MAP_BRICK_W, ball->y/MAP_BRICK_H);
			break;
			
		default:
			brick = NULL;
			break;
	}
	if(brick && brick->type != BRICK_EMPTY) {
		switch(side) {
			case HIT_SIDE_TOP:
				if(ball->vel_y < 0) {
					ball->vel_y = -ball->vel_y;
				}
				break;
				
			case HIT_SIDE_BOTTOM:
				if(ball->vel_y > 0) {
					ball->vel_y = -ball->vel_y;
				}
				break;
				
				
			case HIT_SIDE_LEFT:
				if(ball->vel_x < 0) {
					ball->vel_x = -ball->vel_x;
				}
				break;
				
			case HIT_SIDE_RIGHT:
				if(ball->vel_x > 0) {
					ball->vel_x = -ball->vel_x;
				}
				break;
				
			default:
				break;
		}
		MapDestroyBrick(brick);
		return true;
	}
	return false;
}

static void UpdateBalls()
{
	int i;
	bool create_ball = false;
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists) {
			if(balls[i].glued_to_paddle) {
				balls[i].x = paddle.x+balls[i].glued_pos;
				balls[i].y = paddle.y-(paddle.h/2)-balls[i].radius;
			} else {
				balls[i].x += balls[i].vel_x;
				balls[i].y += balls[i].vel_y;
				if(balls[i].x < balls[i].radius) {
					balls[i].vel_x = -balls[i].vel_x;
					balls[i].x = balls[i].radius;
				}
				if(balls[i].x > (MAP_WIDTH*MAP_BRICK_W)-balls[i].radius) {
					balls[i].vel_x = -balls[i].vel_x;
					balls[i].x = (MAP_WIDTH*MAP_BRICK_W)-balls[i].radius;
				}
				if(balls[i].y < balls[i].radius) {
					balls[i].vel_y = -balls[i].vel_y;
					balls[i].y = balls[i].radius;
				}
				if(balls[i].y > 240+balls[i].radius) {
					balls[i].exists = false;
					num_balls--;
					if(num_balls == 0) {
						create_ball = true;
					}
				}
				if(TestPaddleCollision(&balls[i])) {
					float rel_x = (balls[i].x-(paddle.x));
					float angle = ((-90*(rel_x/paddle.w))+90)*M_DTOR;
					balls[i].vel_x = BALL_VELOCITY*cosf(angle);
					balls[i].vel_y = -BALL_VELOCITY*sinf(angle);
				} else {
					int left, top, right, bottom;
					left = (balls[i].x-balls[i].radius)/MAP_BRICK_W;
					right = (balls[i].x+balls[i].radius)/MAP_BRICK_W;
					top = (balls[i].y-balls[i].radius)/MAP_BRICK_H;
					bottom = (balls[i].y+balls[i].radius)/MAP_BRICK_H;
					if(TestBrickCollision(&balls[i], HIT_SIDE_TOP) || TestBrickCollision(&balls[i], HIT_SIDE_BOTTOM) ||
						TestBrickCollision(&balls[i], HIT_SIDE_LEFT) || TestBrickCollision(&balls[i], HIT_SIDE_RIGHT)) {
							
						}
				}
			}
			SpriteSetPos(&balls[i].sprite, balls[i].x+MAP_X_OFS, balls[i].y+MAP_Y_OFS);
		}
	}
	if(create_ball) {
		CreateBall();
	}
}

static void UpdatePaddle()
{
	if(pad_data[0].stick_x < -32 || pad_data[0].button & L_JPAD) {
		paddle.x -= PADDLE_VELOCITY;
		if(paddle.x < (paddle.w/2)) {
			paddle.x = paddle.w/2;
		}
	}
	if(pad_data[0].stick_x >= 32 || pad_data[0].button & R_JPAD) {
		paddle.x += PADDLE_VELOCITY;
		if(paddle.x >= (MAP_WIDTH*MAP_BRICK_W)-(paddle.w/2)) {
			paddle.x = (MAP_WIDTH*MAP_BRICK_W)-(paddle.w/2);
		}
	}
	SpriteSetPos(&paddle.sprite, paddle.x+MAP_X_OFS, paddle.y+MAP_Y_OFS);
}

static void DoBallRelease()
{
	int i;
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists && pad_data[0].trigger & A_BUTTON) {
			balls[i].glued_to_paddle = NULL;
		}
	}
}

void StageGameUpdate()
{
	UpdatePaddle();
	UpdateBalls();
}

static void DrawBall()
{
	int i;
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists) {
			SpriteDraw(&balls[i].sprite);
		}
	}
}

static void DrawPaddle()
{
	SpriteDraw(&paddle.sprite);
}

static void DrawHUD()
{
	char text_buf[64];
	sprintf(text_buf, "Bricks:%d", MapGetNumBricks());
	TextDraw(228, 64, TEXT_ALIGNMENT_LEFT, text_buf);
}

void StageGameDraw()
{
    RenderStartFrame();
	RenderClear(0, 0, 0);
	SpriteDraw(border_sprite);
	SpriteDraw(field_bg_sprite);
	MapDraw();
	DrawPaddle();
	DrawBall();
	DrawHUD();
	RenderEndFrame();
}

void StageGameDestroy()
{
	MapUnload();
	SpriteDelete(border_sprite);
	SpriteDelete(field_bg_sprite);
	free(game_sprites);
	game_sprites = NULL;
}