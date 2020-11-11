#include <PR/ultratypes.h>
#include <PR/gu.h>
#include <math.h>
#include "render.h"
#include "pad.h"
#include "map.h"
#include "sprite.h"
#include "text.h"
#include "bool.h"

#define MAX_BALLS 3
#define MAX_POWERUPS 5
#define MAX_BULLETS 32
#define BALL_VELOCITY 2.5
#define PADDLE_VELOCITY 5.0
#define POWERUP_FALL_SPEED 0.833
#define LASER_DURATION 300
#define PADDLE_RESIZE_DURATION 300
#define BIG_BALL_DURATION 300
#define BALL_SPEED_CHANGE_DURATION 300
#define PADDLE_STICKY_LENGTH 300

#define HIT_SIDE_TOP 0
#define HIT_SIDE_BOTTOM 1
#define HIT_SIDE_LEFT 2
#define HIT_SIDE_RIGHT 3

#define POWERUP_LASER 0
#define POWERUP_ENLARGE 1
#define POWERUP_BALL_ENLARGE 2
#define POWERUP_SHRINK 3
#define POWERUP_SLOW 4
#define POWERUP_FAST 5
#define POWERUP_EXTRA_LIFE 6
#define POWERUP_TRIPLE 7
#define POWERUP_CATCH 8
#define POWERUP_MAX 9

#define BALL_SIZE_NORMAL 0
#define BALL_SIZE_BIG 1
#define BALL_SIZE_COUNT 2

#define PADDLE_TYPE_NORMAL 0
#define PADDLE_TYPE_LONG 1
#define PADDLE_TYPE_SHORT 2
#define PADDLE_TYPE_LASER 3
#define PADDLE_TYPE_COUNT 4

typedef struct paddle {
	float x;
	float y;
	float w;
	float h;
	int sticky_timer;
	int laser_timer;
	int size_reset_timer;
	SpriteInfo sprite;
} Paddle;

typedef struct ball {
	bool exists;
	float x;
	float y;
	float radius;
	float vel_x;
	float vel_y;
	Paddle *catcher;
	float catch_pos;
	SpriteInfo sprite;
} Ball;

typedef struct powerup {
	bool exists;
	int type;
	float x;
	float y;
	SpriteInfo sprite;
} PowerUp;

typedef struct bullet {
	bool exists;
	float x;
	float y;
	SpriteInfo sprite;
} Bullet;

static Ball balls[MAX_BALLS];
static PowerUp powerups[MAX_POWERUPS];
static Bullet bullets[MAX_BULLETS];
static Paddle paddle;
static float ball_vel_ratio;
static int reset_vel_timer;
static int curr_ball_size;
static int shrink_timer;
static int num_balls;
static int num_lives;
static SpriteData *game_sprites;
static SpriteInfo *border_sprite;
static SpriteInfo *field_bg_sprite;

static char *powerup_images[POWERUP_MAX] = {
	"powerup_laser",
	"powerup_enlarge",
	"powerup_ball_enlarge",
	"powerup_shrink",
	"powerup_slow",
	"powerup_fast",
	"powerup_extra_life",
	"powerup_triple",
	"powerup_catch"
};

static int paddle_width[PADDLE_TYPE_COUNT] = { 40, 48, 32, 40 };
static int ball_radius[BALL_SIZE_COUNT] = { 4, 8 };

static void InitBalls()
{
	int i;
	for(i=0; i<MAX_BALLS; i++) {
		balls[i].exists = false;
		SpriteInit(&balls[i].sprite, game_sprites);
	}
	num_balls = 0;
}

static void InitPaddle()
{
	paddle.x = (MAP_WIDTH*MAP_BRICK_W/2);
	paddle.y = 200;
	paddle.w = 40;
	paddle.h = 8;
	paddle.size_reset_timer = paddle.laser_timer = paddle.sticky_timer = 0;
	SpriteInit(&paddle.sprite, game_sprites);
	SpriteSetImage(&paddle.sprite, "paddle");
	SpriteSetPos(&paddle.sprite, paddle.x+MAP_X_OFS, paddle.y+MAP_Y_OFS);
}

static void InitPowerups()
{
	int i;
	for(i=0; i<MAX_POWERUPS; i++) {
		powerups[i].exists = false;
		SpriteInit(&powerups[i].sprite, game_sprites);
	}
}

static void InitBullets()
{
	int i;
	for(i=0; i<MAX_BULLETS; i++) {
		bullets[i].exists = false;
		SpriteInit(&bullets[i].sprite, game_sprites);
		SpriteSetImage(&bullets[i].sprite, "bullet");
	}
}

static void CreateFirstBall()
{
	int i, j;
	int range;
	char *ball_images[BALL_SIZE_COUNT] = { "ball", "big_ball" };
	for(i=0; i<MAX_BALLS; i++) {
		if(!balls[i].exists) {
			balls[i].exists = true;
			break;
		}
	}
	if(i == MAX_BALLS) {
		return;
	}
	balls[i].radius = ball_radius[curr_ball_size];
	balls[i].y = paddle.y-(paddle.h/2)-balls[i].radius;
	balls[i].vel_x = 0;
	balls[i].vel_y = 0;
	balls[i].catcher = &paddle;
	range = paddle.w-(balls[i].radius*2);
	balls[i].catch_pos = (rand()%range)-((float)range/2);
	balls[i].x = paddle.x+balls[i].catch_pos;
	SpriteSetImage(&balls[i].sprite, ball_images[curr_ball_size]);
	SpriteSetPos(&balls[i].sprite, balls[i].x+MAP_X_OFS, balls[i].y+MAP_Y_OFS);
	num_balls++;
}

static void CreateBall()
{
	int i, j;
	float angle;
	char *ball_images[BALL_SIZE_COUNT] = { "ball", "big_ball" };
	for(i=0; i<MAX_BALLS; i++) {
		if(!balls[i].exists) {
			balls[i].exists = true;
			break;
		}
	}
	if(i == MAX_BALLS) {
		return;
	}
	balls[i].radius = ball_radius[curr_ball_size];
	balls[i].y = paddle.y-(paddle.h/2)-balls[i].radius;
	angle = M_DTOR*((rand()%90)+30);
	balls[i].vel_x = BALL_VELOCITY*cosf(angle);
	balls[i].vel_y = -BALL_VELOCITY*sinf(angle);
	balls[i].catcher = NULL;
	balls[i].catch_pos = 0;
	balls[i].x = paddle.x+balls[i].catch_pos;
	SpriteSetImage(&balls[i].sprite, ball_images[curr_ball_size]);
	SpriteSetPos(&balls[i].sprite, balls[i].x+MAP_X_OFS, balls[i].y+MAP_Y_OFS);
	num_balls++;
}

void StageGameInit()
{
	RenderSetSize(320, 240);
	game_sprites = SpriteLoadFile("gamesprites.spr");
	ball_vel_ratio = 1.0f;
	curr_ball_size = BALL_SIZE_NORMAL;
	reset_vel_timer = shrink_timer = 0;
	num_lives = 4;
	InitBalls();
	InitPaddle();
	InitPowerups();
	InitBullets();
	CreateFirstBall();
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

static void CreatePowerup(float x, float y)
{
	int i;
	for(i=0; i<MAX_POWERUPS; i++) {
		if(!powerups[i].exists) {
			powerups[i].exists = true;
			break;
		}
	}
	if(i == MAX_POWERUPS) {
		return;
	}
	powerups[i].type = rand() % POWERUP_MAX;
	powerups[i].x = x;
	powerups[i].y = y;
	SpriteSetImage(&powerups[i].sprite, powerup_images[powerups[i].type]);
	SpriteSetPos(&powerups[i].sprite, x+MAP_X_OFS, y+MAP_Y_OFS);
}

static bool TestBrickCollision(Ball *ball, int side)
{
	MapBrick *brick;
	float powerup_x, powerup_y;
	switch(side) {
		case HIT_SIDE_TOP:
			powerup_x = ((int)(ball->x/MAP_BRICK_W)*MAP_BRICK_W)+(MAP_BRICK_W/2);
			powerup_y = ((int)((ball->y-ball->radius)/MAP_BRICK_H)*MAP_BRICK_H)+(MAP_BRICK_H/2);
			brick = MapGetBrick(ball->x/MAP_BRICK_W, (ball->y-ball->radius)/MAP_BRICK_H);
			break;
			
		case HIT_SIDE_BOTTOM:
			powerup_x = ((int)(ball->x/MAP_BRICK_W)*MAP_BRICK_W)+(MAP_BRICK_W/2);
			powerup_y = ((int)((ball->y+ball->radius)/MAP_BRICK_H)*MAP_BRICK_H)+(MAP_BRICK_H/2);
			brick = MapGetBrick(ball->x/MAP_BRICK_W, (ball->y+ball->radius)/MAP_BRICK_H);
			break;
			
		case HIT_SIDE_LEFT:
			powerup_x = ((int)((ball->x-ball->radius)/MAP_BRICK_W)*MAP_BRICK_W)+(MAP_BRICK_W/2);
			powerup_y = ((int)(ball->y/MAP_BRICK_H)*MAP_BRICK_H)+(MAP_BRICK_H/2);
			brick = MapGetBrick((ball->x-ball->radius)/MAP_BRICK_W, ball->y/MAP_BRICK_H);
			break;
			
		case HIT_SIDE_RIGHT:
			powerup_x = ((int)((ball->x+ball->radius)/MAP_BRICK_W)*MAP_BRICK_W)+(MAP_BRICK_W/2);
			powerup_y = ((int)(ball->y/MAP_BRICK_H)*MAP_BRICK_H)+(MAP_BRICK_H/2);
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
		if(rand() % 5 == 0 && brick->type != BRICK_GOLD) {
			if(brick->type != BRICK_ROCK3 && brick->type != BRICK_ROCK2 && brick->type != BRICK_ROCK1) {
				CreatePowerup(powerup_x, powerup_y);
			}
		}
		MapDestroyBrick(brick);
		return true;
	}
	return false;
}

static void UpdateBalls()
{
	int i;
	bool reset_field = false;
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists) {
			if(balls[i].catcher) {
				balls[i].x = paddle.x+balls[i].catch_pos;
				balls[i].y = paddle.y-(paddle.h/2)-balls[i].radius;
			} else {
				balls[i].x += balls[i].vel_x*ball_vel_ratio;
				balls[i].y += balls[i].vel_y*ball_vel_ratio;
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
					if(num_balls == 0 && num_lives != 0) {
						reset_field = true;
						num_lives--;
					}
				}
				if(TestPaddleCollision(&balls[i])) {
					if(paddle.sticky_timer == 0) {
						float rel_x = (balls[i].x-paddle.x);
						float angle = ((-90*(rel_x/paddle.w))+90)*M_DTOR;
						balls[i].vel_x = BALL_VELOCITY*cosf(angle);
						balls[i].vel_y = -BALL_VELOCITY*sinf(angle);
					} else {
						float catch_pos = balls[i].x-paddle.x;
						if(fabsf(catch_pos) <= (paddle.w/2)-balls[i].radius) {
							balls[i].catcher = &paddle;
							balls[i].catch_pos = catch_pos;
						}
					}
				} else {
					TestBrickCollision(&balls[i], HIT_SIDE_TOP);
					TestBrickCollision(&balls[i], HIT_SIDE_BOTTOM);
					TestBrickCollision(&balls[i], HIT_SIDE_LEFT);
					TestBrickCollision(&balls[i], HIT_SIDE_RIGHT);
				}
			}
			SpriteSetPos(&balls[i].sprite, balls[i].x+MAP_X_OFS, balls[i].y+MAP_Y_OFS);
		}
	}
	if(reset_field) {
		CreateFirstBall();
	}
}

static void ReleaseBalls()
{
	int i;
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists && balls[i].catcher) {
			float angle = ((-90*(balls[i].catch_pos/paddle.w))+90)*M_DTOR;
			balls[i].catcher = NULL;
			balls[i].vel_x = BALL_VELOCITY*cosf(angle);
			balls[i].vel_y = -BALL_VELOCITY*sinf(angle);
		}
	}
}

static void SetBallSize(int size)
{
	int i;
	char *ball_images[BALL_SIZE_COUNT] = { "ball", "big_ball" };
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists) {
			balls[i].radius = ball_radius[size];
			SpriteSetImage(&balls[i].sprite, ball_images[size]);
		}
	}
	curr_ball_size = size;
}

static void CreateBullet(float x, float y)
{
	int i;
	for(i=0; i<MAX_BULLETS; i++) {
		 if(!bullets[i].exists) {
			 bullets[i].exists = true;
			 break;
		 }
	}
	if(i == MAX_BULLETS) {
		return;
	}
	bullets[i].x = x;
	bullets[i].y = y;
	SpriteSetPos(&bullets[i].sprite, x+MAP_X_OFS, y+MAP_Y_OFS);
}

static void UpdatePaddle()
{
	if(pad_data[0].stick_x < -10 || pad_data[0].stick_x >= 10) {
		if(pad_data[0].stick_x < -64 || pad_data[0].stick_x >= 64) {
			if(pad_data[0].stick_x < 0) {
				paddle.x -= PADDLE_VELOCITY;
			} else {
				paddle.x += PADDLE_VELOCITY;
			}
		} else {
			paddle.x += (PADDLE_VELOCITY*(pad_data[0].stick_x/64.0f));
		}
		
		if(paddle.x < (paddle.w/2)) {
			paddle.x = paddle.w/2;
		}
		if(paddle.x >= (MAP_WIDTH*MAP_BRICK_W)-(paddle.w/2)) {
			paddle.x = (MAP_WIDTH*MAP_BRICK_W)-(paddle.w/2);
		}
	}
	if(pad_data[0].trigger & A_BUTTON) {
		if(paddle.laser_timer == 0) {
			ReleaseBalls();
		} else {
			CreateBullet(paddle.x-16, paddle.y-4);
			CreateBullet(paddle.x+16, paddle.y-4);
		}
	}
	SpriteSetPos(&paddle.sprite, paddle.x+MAP_X_OFS, paddle.y+MAP_Y_OFS);
}

static void SetPaddleType(int length)
{
	char *images[PADDLE_TYPE_COUNT] = {"paddle", "paddle_long", "paddle_short", "paddle_laser"};
	paddle.w = paddle_width[length];
	SpriteSetImage(&paddle.sprite, images[length]);
}

static void ReleaseOffEdgeBalls()
{
	int i;
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists && balls[i].catcher) {
			if(fabsf(balls[i].catch_pos) > (paddle.w/2)-balls[i].radius) {
				int side = 1;
				float angle;
				if(balls[i].catch_pos < 0) {
					side = -1;
				}
				angle = ((-90*side)+90)*M_DTOR;
				balls[i].catcher = NULL;
				balls[i].vel_x = BALL_VELOCITY*cosf(angle);
				balls[i].vel_y = -BALL_VELOCITY*sinf(angle);
			}
		}
	}
}

static void ActivatePowerup(int type)
{
	switch(type) {
		case POWERUP_LASER:
			if(paddle.sticky_timer) {
				ReleaseBalls();
				paddle.sticky_timer = 0;
			}
			paddle.laser_timer = LASER_DURATION;
			SetPaddleType(PADDLE_TYPE_LASER);
			break;
			
		case POWERUP_ENLARGE:
			paddle.size_reset_timer = PADDLE_RESIZE_DURATION;
			SetPaddleType(PADDLE_TYPE_LONG);
			ReleaseOffEdgeBalls();
			break;
			
		case POWERUP_BALL_ENLARGE:
			shrink_timer = BIG_BALL_DURATION;
			SetBallSize(BALL_SIZE_BIG);
			ReleaseOffEdgeBalls();
			break;
			
		case POWERUP_SHRINK:
			paddle.size_reset_timer = PADDLE_RESIZE_DURATION;
			SetPaddleType(PADDLE_TYPE_SHORT);
			ReleaseOffEdgeBalls();
			break;
			
		case POWERUP_SLOW:
			reset_vel_timer = BALL_SPEED_CHANGE_DURATION;
			ball_vel_ratio = 0.666f;
			break;
			
		case POWERUP_FAST:
			reset_vel_timer = BALL_SPEED_CHANGE_DURATION;
			ball_vel_ratio = 1.5f;
			break;
			
		case POWERUP_EXTRA_LIFE:
			num_lives++;
			break;
			
		case POWERUP_TRIPLE:
			CreateBall();
			CreateBall();
			break;
			
		case POWERUP_CATCH:
			if(paddle.laser_timer) {
				paddle.laser_timer = 0;
			}
			paddle.sticky_timer = PADDLE_STICKY_LENGTH;
			break;
			
		default:
			break;
	}
}

static void UpdatePowerups()
{
	int i;
	for(i=0; i<MAX_POWERUPS; i++) {
		if(powerups[i].exists) {
			powerups[i].y += POWERUP_FALL_SPEED;
			if(powerups[i].y > 240+(MAP_BRICK_H/2)) {
				powerups[i].exists = false;
			} else {
				if((paddle.y+(paddle.h/2)) >= powerups[i].y-(MAP_BRICK_H/2) && (paddle.y-(paddle.h/2)) < powerups[i].y+(MAP_BRICK_H/2)) {
					if((paddle.x+(paddle.w/2)) >= powerups[i].x-(MAP_BRICK_W/2) && (paddle.x-(paddle.w/2)) < powerups[i].x+(MAP_BRICK_W/2)) {
						ActivatePowerup(powerups[i].type);
						powerups[i].exists = false;
						continue;
					}
				}
				SpriteSetPos(&powerups[i].sprite, powerups[i].x+MAP_X_OFS, powerups[i].y+MAP_Y_OFS);
			}
		}
	}
}

static void UpdateTimers()
{
	if(paddle.sticky_timer) {
		paddle.sticky_timer--;
		if(paddle.sticky_timer == 0) {
			ReleaseBalls();
		}
	}
	if(paddle.size_reset_timer) {
		paddle.size_reset_timer--;
		if(paddle.size_reset_timer == 0) {
			SetPaddleType(PADDLE_TYPE_NORMAL);
			ReleaseOffEdgeBalls();
		}
	}
	if(paddle.laser_timer) {
		paddle.laser_timer--;
		if(paddle.laser_timer == 0) {
			SetPaddleType(PADDLE_TYPE_NORMAL);
		}
	}
	if(reset_vel_timer) {
		reset_vel_timer--;
		if(reset_vel_timer == 0) {
			ball_vel_ratio = 1.0;
		}
	}
	if(shrink_timer) {
		shrink_timer--;
		if(shrink_timer == 0) {
			SetBallSize(BALL_SIZE_NORMAL);
			ReleaseOffEdgeBalls();
		}
	}
}

static void UpdateBullets()
{
	int i;
	for(i=0; i<MAX_BULLETS; i++) {
		if(bullets[i].exists) {
			MapBrick *brick;
			bullets[i].y -= 2.0f;
			if(bullets[i].y < 4) {
				bullets[i].exists = false;
				continue;
			}
			brick = MapGetBrick(bullets[i].x/MAP_BRICK_W, (bullets[i].y-4)/MAP_BRICK_H);
			if(brick && brick->type != BRICK_EMPTY) {
				MapDestroyBrick(brick);
				bullets[i].exists = false;
				continue;
			}
			SpriteSetPos(&bullets[i].sprite, bullets[i].x+MAP_X_OFS, bullets[i].y+MAP_Y_OFS);
		}
	}
}

void StageGameUpdate()
{
	if(num_lives > 0 && MapGetNumBricks() > 0) {
		UpdatePaddle();
		UpdateBalls();
		UpdatePowerups();
		UpdateBullets();
		UpdateTimers();
	}
}

static void DrawBalls()
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

static void DrawPowerups()
{
	int i;
	for(i=0; i<MAX_POWERUPS; i++) {
		if(powerups[i].exists) {
			SpriteDraw(&powerups[i].sprite);
		}
	}
}

static void DrawBullets()
{
	int i;
	for(i=0; i<MAX_BULLETS; i++) {
		if(bullets[i].exists) {
			SpriteDraw(&bullets[i].sprite);
		}
	}
}

static void DrawHUD()
{
	char text_buf[64];
	sprintf(text_buf, "Lives %d", num_lives);
	TextDraw(228, 24, TEXT_ALIGNMENT_LEFT, text_buf);
	sprintf(text_buf, "Bricks %d", MapGetNumBricks());
	TextDraw(228, 33, TEXT_ALIGNMENT_LEFT, text_buf);
}

void StageGameDraw()
{
    RenderStartFrame();
	RenderClear(0, 0, 0);
	if(num_lives > 0 && MapGetNumBricks() > 0) {
		SpriteDraw(border_sprite);
		SpriteDraw(field_bg_sprite);
		MapDraw();
		DrawPaddle();
		DrawPowerups();
		DrawBalls();
		DrawBullets();
		DrawHUD();
	} else {
		if(num_lives > 0) {
			TextDraw(160, 120, TEXT_ALIGNMENT_CENTER, "You Win");
		} else {
			TextDraw(160, 120, TEXT_ALIGNMENT_CENTER, "You Lose");
		}
	}
	
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