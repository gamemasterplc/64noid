#include <PR/ultratypes.h>
#include <PR/gu.h>
#include <math.h>
#include "game.h"
#include "render.h"
#include "pad.h"
#include "map.h"
#include "sprite.h"
#include "text.h"
#include "main.h"
#include "bool.h"
#include "save.h"

#define SCREEN_W 320
#define SCREEN_H 240

#define MAX_BALLS 3
#define MAX_POWERUPS 1
#define MAX_BULLETS 32
#define BALL_VELOCITY 2.5
#define PADDLE_VELOCITY 5.0
#define PADDLE_ANGLE_RANGE 60
#define BULLET_X_OFS 16
#define POWERUP_FALL_SPEED 0.833
#define POWERUP_APPEAR_RATE 10
#define STICK_DEADZONE 10
#define PAUSE_STICK_DEADZONE 32
#define PAUSE_STICK_DELAY 10
#define STICK_X_RANGE 64
#define PAUSE_FADE_DURATION 8
#define UI_POS_X (MAP_X_OFS+(MAP_WIDTH*MAP_BRICK_W)+12)

#define HIT_SIDE_TOP 0
#define HIT_SIDE_BOTTOM 1
#define HIT_SIDE_LEFT 2
#define HIT_SIDE_RIGHT 3

#define POWERUP_LASER 0
#define POWERUP_ENLARGE 1
#define POWERUP_BALL_ENLARGE 2
#define POWERUP_SHRINK 3
#define POWERUP_EXTRA_LIFE 4
#define POWERUP_TRIPLE 5
#define POWERUP_CATCH 6
#define POWERUP_BALL_SHRINK 7
#define POWERUP_NEXT_LEVEL 8
#define POWERUP_COUNT 9

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
	int type;
	bool sticky;
	bool laser;
	SpriteInfo sprite;
} Paddle;

typedef struct ball {
	bool exists;
	float x;
	float y;
	float radius;
	float vel_x;
	float vel_y;
	float vel_ratio;
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

typedef struct pause_option {
	char *name;
	void (*activate_func)();
} PauseOption;

GameGlobals game_globals;

static Ball balls[MAX_BALLS];
static PowerUp powerups[MAX_POWERUPS];
static Bullet bullets[MAX_BULLETS];
static Paddle paddle;
static int num_balls;
static bool paused;
static int pause_stick_timer;
static int pause_pos;
static SpriteData *game_sprites;
static SpriteInfo *border_sprite;

static char *powerup_images[POWERUP_COUNT] = {
	"powerup_laser",
	"powerup_enlarge",
	"powerup_ball_enlarge",
	"powerup_shrink",
	"powerup_extra_life",
	"powerup_triple",
	"powerup_catch",
	"powerup_ball_shrink",
	"powerup_next_level"
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
	//Set Paddle to be Normal Paddle at the Middle Bottom of the Screen
	paddle.x = (MAP_WIDTH*MAP_BRICK_W/2);
	paddle.y = 200;
	paddle.w = 40;
	paddle.h = 8;
	paddle.sticky = false;
	paddle.laser = false;
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

static int FindFreeBall()
{
	int i;
	for(i=0; i<MAX_BALLS; i++) {
		if(!balls[i].exists) {
			return i;
		}
	}
	return -1;
}

static int FindFirstUsedBall()
{
	int i;
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists) {
			return i;
		}
	}
	return -1;
}

static void InitBall(Ball *ball, bool attached)
{
	float angle;
	char *ball_images[BALL_SIZE_COUNT] = { "ball", "big_ball" };
	ball->exists = true;
	ball->radius = ball_radius[BALL_SIZE_NORMAL];
	ball->y = paddle.y-(paddle.h/2)-ball->radius;
	angle = M_DTOR*((rand()%PADDLE_ANGLE_RANGE)+(90-(PADDLE_ANGLE_RANGE/2)));
	ball->vel_x = BALL_VELOCITY*cosf(angle);
	ball->vel_y = -BALL_VELOCITY*sinf(angle);
	ball->vel_ratio = 1.0f;
	if(attached) {
		ball->catcher = &paddle;
		ball->catch_pos = 0;
	} else {
		ball->catcher = NULL;
		ball->catch_pos = 0;
	}
	ball->x = paddle.x+ball->catch_pos;
	SpriteSetImage(&ball->sprite, ball_images[BALL_SIZE_NORMAL]);
	SpriteSetPos(&ball->sprite, ball->x+MAP_X_OFS, ball->y+MAP_Y_OFS);
}

static void CreateBall()
{
	int ball_idx = FindFreeBall();
	int exist_ball_idx = FindFirstUsedBall();
	if(ball_idx == -1) {
		return;
	}
	InitBall(&balls[ball_idx], false);
	if(exist_ball_idx != -1) {
		balls[ball_idx].x = balls[exist_ball_idx].x;
		balls[ball_idx].y = balls[exist_ball_idx].y;
		if(balls[exist_ball_idx].vel_y > 0) {
			balls[ball_idx].vel_y = -balls[ball_idx].vel_y;
		}
	}
	num_balls++;
}

static void CreateFirstBall()
{
	int ball_idx = FindFreeBall();
	if(ball_idx == -1) {
		return;
	}
	InitBall(&balls[ball_idx], true);
	num_balls++;
}

void StageGameInit()
{
	RenderSetSize(SCREEN_W, SCREEN_H);
	game_sprites = SpriteLoadFile("gamesprites.spr");
	InitBalls();
	InitPaddle();
	InitPowerups();
	InitBullets();
	CreateFirstBall();
	if(game_globals.edit_mode) {
		MapLoadSave(game_globals.map_num);
	} else {
		MapLoad(game_globals.map_num);
	}
	border_sprite = SpriteCreate(game_sprites);
	SpriteSetImage(border_sprite, "border");
	SpriteSetPos(border_sprite, MAP_X_OFS-8, MAP_Y_OFS-8);
	paused = false;
	pause_pos = 0;
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

static bool CheckBallSizeExist(int size)
{
	int i;
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists && balls[i].radius == ball_radius[size]) {
			return true;
		}
	}
	return false;
}

static int GetPowerupType()
{
	int base_type = rand() % POWERUP_COUNT;
	if(base_type == POWERUP_LASER && (paddle.laser || game_globals.edit_mode)) {
		return -1;
	}
	if(base_type == POWERUP_ENLARGE && paddle.type == PADDLE_TYPE_LONG) {
		return -1;
	}
	if(base_type == POWERUP_SHRINK && paddle.type == PADDLE_TYPE_SHORT) {
		return -1;
	}
	if(base_type == POWERUP_BALL_ENLARGE && !CheckBallSizeExist(BALL_SIZE_NORMAL)) {
		return -1;
	}
	if(base_type == POWERUP_EXTRA_LIFE && game_globals.edit_mode) {
		return -1;
	}
	if(base_type == POWERUP_CATCH && paddle.sticky) {
		return -1;
	}
	if(base_type == POWERUP_BALL_SHRINK && !CheckBallSizeExist(BALL_SIZE_BIG)) {
		return -1;
	}
	if(base_type == POWERUP_NEXT_LEVEL && (game_globals.edit_mode || (rand() % 4 != 0))) {
		return -1;
	}
	return base_type;
}

static void CreatePowerup(float x, float y)
{
	int i;
	int powerup_type = -1;
	for(i=0; i<MAX_POWERUPS; i++) {
		if(!powerups[i].exists) {
			powerups[i].exists = true;
			break;
		}
	}
	if(i == MAX_POWERUPS) {
		return;
	}
	while(powerup_type == -1) {
		powerup_type = GetPowerupType();
	}
	powerups[i].type = powerup_type;
	powerups[i].x = x;
	powerups[i].y = y;
	SpriteSetImage(&powerups[i].sprite, powerup_images[powerups[i].type]);
	SpriteSetPos(&powerups[i].sprite, x+MAP_X_OFS, y+MAP_Y_OFS);
}

static void SetPaddleType(int type)
{
	char *images[PADDLE_TYPE_COUNT] = {"paddle", "paddle_long", "paddle_short", "paddle_laser"};
	paddle.w = paddle_width[type];
	SpriteSetImage(&paddle.sprite, images[type]);
	if(type != PADDLE_TYPE_LASER) {
		paddle.type = type;
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

static int GetBrickWorth(MapBrick *brick)
{
	switch(brick->type) {
		case BRICK_EMPTY:
		case BRICK_GOLD:
		case BRICK_ROCK3:
		case BRICK_ROCK2:
			return 0;
		
		case BRICK_ROCK1:
			if(game_globals.edit_mode) {
				return 100;
			} else {
				return (50*(game_globals.map_num+1));
			}
			
			
		default:
			return 50+(10*(brick->type-BRICK_START));
	}
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
		if(brick->type != BRICK_GOLD && brick->type != BRICK_ROCK3 && brick->type != BRICK_ROCK2) {
			if(rand() % POWERUP_APPEAR_RATE == 0 && brick->type != BRICK_ROCK1 && MapGetNumBricks() != 1) {
				CreatePowerup(powerup_x, powerup_y);
			}
		}
		game_globals.score += GetBrickWorth(brick);
		if(game_globals.score > save_data->high_score && !game_globals.edit_mode) {
			save_data->high_score = game_globals.score;
			game_globals.update_high_score = true;
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
				balls[i].x += balls[i].vel_x*balls[i].vel_ratio;
				balls[i].y += balls[i].vel_y*balls[i].vel_ratio;
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
				if(balls[i].y > SCREEN_H-MAP_Y_OFS+balls[i].radius) {
					balls[i].exists = false;
					num_balls--;
					if(num_balls == 0) {
						reset_field = true;
						game_globals.num_lives--;
						if(game_globals.num_lives <= 0) {
							if (game_globals.edit_mode) {
								SetNextStage(STAGE_MAPEDITOR);
							} else {
								SetNextStage(STAGE_END);
							}
						}
					}
				}
				if(TestPaddleCollision(&balls[i])) {
					if(!paddle.sticky) {
						float rel_x = (balls[i].x-paddle.x);
						float angle = ((-PADDLE_ANGLE_RANGE*(rel_x/paddle.w))+90)*M_DTOR;
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
		paddle.sticky = false;
		paddle.laser = false;
		SetPaddleType(PADDLE_TYPE_NORMAL);
		CreateFirstBall();
	}
}

static void SetCurrentBallSize(int size)
{
	int i;
	char *ball_images[BALL_SIZE_COUNT] = { "ball", "big_ball" };
	for(i=0; i<MAX_BALLS; i++) {
		if(balls[i].exists) {
			balls[i].radius = ball_radius[size];
			SpriteSetImage(&balls[i].sprite, ball_images[size]);
		}
	}
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
	if(pad_data[0].stick_x < -STICK_DEADZONE || pad_data[0].stick_x >= STICK_DEADZONE) {
		if(pad_data[0].stick_x < -STICK_X_RANGE || pad_data[0].stick_x >= STICK_X_RANGE) {
			if(pad_data[0].stick_x < 0) {
				paddle.x -= PADDLE_VELOCITY;
			} else {
				paddle.x += PADDLE_VELOCITY;
			}
		} else {
			paddle.x += (PADDLE_VELOCITY*((float)pad_data[0].stick_x/STICK_X_RANGE));
		}
		
		if(paddle.x < (paddle.w/2)) {
			paddle.x = paddle.w/2;
		}
		if(paddle.x >= (MAP_WIDTH*MAP_BRICK_W)-(paddle.w/2)) {
			paddle.x = (MAP_WIDTH*MAP_BRICK_W)-(paddle.w/2);
		}
	}
	if(pad_data[0].trigger & A_BUTTON) {
		if(!paddle.laser) {
			ReleaseBalls();
		} else {
			CreateBullet(paddle.x-BULLET_X_OFS, paddle.y-4);
			CreateBullet(paddle.x+BULLET_X_OFS, paddle.y-4);
		}
	}
	SpriteSetPos(&paddle.sprite, paddle.x+MAP_X_OFS, paddle.y+MAP_Y_OFS);
}

static void AdvanceLevel()
{
	if(game_globals.map_num == num_maps-1) {
		SetNextStage(STAGE_END);
	} else {
		SetNextStage(STAGE_NEXTMAP);
	}
}

static void EjectPaddle()
{
	if(paddle.x < (paddle.w/2)) {
		paddle.x = paddle.w/2;
	}
	if(paddle.x >= (MAP_WIDTH*MAP_BRICK_W)-(paddle.w/2)) {
		paddle.x = (MAP_WIDTH*MAP_BRICK_W)-(paddle.w/2);
	}
}

static void ActivatePowerup(int type)
{
	switch(type) {
		case POWERUP_LASER:
			paddle.sticky = false;
			paddle.laser = true;
			SetPaddleType(PADDLE_TYPE_LASER);
			SetCurrentBallSize(BALL_SIZE_NORMAL);
			ReleaseBalls();
			break;
			
		case POWERUP_ENLARGE:
			paddle.laser = false;
			paddle.sticky = false;
			if(paddle.type == PADDLE_TYPE_SHORT) {
				SetPaddleType(PADDLE_TYPE_NORMAL);
			} else {
				SetPaddleType(PADDLE_TYPE_LONG);
			}
			EjectPaddle();
			SetCurrentBallSize(BALL_SIZE_NORMAL);
			ReleaseBalls();
			break;
			
		case POWERUP_BALL_ENLARGE:
			paddle.laser = false;
			paddle.sticky = false;
			SetPaddleType(paddle.type);
			SetCurrentBallSize(BALL_SIZE_BIG);
			ReleaseBalls();
			break;
			
		case POWERUP_SHRINK:
			paddle.laser = false;
			paddle.sticky = false;
			if(paddle.type == PADDLE_TYPE_LONG) {
				SetPaddleType(PADDLE_TYPE_NORMAL);
			} else {
				SetPaddleType(PADDLE_TYPE_SHORT);
			}
			SetCurrentBallSize(BALL_SIZE_NORMAL);
			ReleaseBalls();
			break;
			
		case POWERUP_EXTRA_LIFE:
			game_globals.num_lives++;
			paddle.sticky = false;
			SetCurrentBallSize(BALL_SIZE_NORMAL);
			ReleaseBalls();
			break;
			
		case POWERUP_TRIPLE:
			CreateBall();
			CreateBall();
			paddle.sticky = false;
			ReleaseBalls();
			break;
			
		case POWERUP_CATCH:
			paddle.laser = false;
			paddle.sticky = true;
			SetPaddleType(paddle.type);
			ReleaseBalls();
			break;
			
		case POWERUP_BALL_SHRINK:
			paddle.laser = false;
			paddle.sticky = false;
			SetPaddleType(paddle.type);
			SetCurrentBallSize(BALL_SIZE_NORMAL);
			ReleaseBalls();
			break;
			
		case POWERUP_NEXT_LEVEL:
			AdvanceLevel();
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
			if(powerups[i].y > SCREEN_H-MAP_Y_OFS+(MAP_BRICK_H/2)) {
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
				game_globals.score += GetBrickWorth(brick);
				if(game_globals.score > save_data->high_score && !game_globals.edit_mode) {
					save_data->high_score = game_globals.score;
					game_globals.update_high_score = true;
				}
				MapDestroyBrick(brick);
				bullets[i].exists = false;
				if(brick->type != BRICK_ROCK3 && brick->type != BRICK_ROCK2 && brick->type != BRICK_ROCK1) {
					if(rand() % POWERUP_APPEAR_RATE == 0 && brick->type != BRICK_GOLD && MapGetNumBricks() != 1) {
						float powerup_x = ((int)(bullets[i].x/MAP_BRICK_W)*MAP_BRICK_W)+(MAP_BRICK_W/2.0f);
						float powerup_y = ((int)((bullets[i].y-4)/MAP_BRICK_H)*MAP_BRICK_H)+(MAP_BRICK_H/2.0f);
						CreatePowerup(powerup_x, powerup_y);
					}
				}
				continue;
			}
			SpriteSetPos(&bullets[i].sprite, bullets[i].x+MAP_X_OFS, bullets[i].y+MAP_Y_OFS);
		}
	}
}

static void UnpauseGame()
{
	paused = false;
}

static void QuitGame()
{
	if(game_globals.edit_mode) {
		SetNextStage(STAGE_MAPEDITOR);
	} else {
		if(game_globals.update_high_score) {
			SaveWrite();
		}
		SetNextStage(STAGE_TITLE);
	}
}

static PauseOption pause_options[] = {
	{ "Continue", UnpauseGame },
	{ "Quit", QuitGame }
};

static int num_pause_options = sizeof(pause_options)/sizeof(PauseOption);
static int pause_options_y = (SCREEN_H-((sizeof(pause_options)/sizeof(PauseOption))*9))/2;

void StageGameUpdate()
{
	if(!paused) {
		UpdatePaddle();
		UpdateBalls();
		UpdatePowerups();
		UpdateBullets();
		if(MapGetNumBricks() == 0) {
			if(game_globals.edit_mode) {
				SetNextStage(STAGE_MAPEDITOR);
			} else {
				if(game_globals.map_num == num_maps-1) {
					SetNextStage(STAGE_END);
				} else {
					SetNextStage(STAGE_NEXTMAP);
				}
			}
		}
		if(pad_data[0].trigger & START_BUTTON) {
			pause_pos = 0;
			paused = true;
		}
	} else {
		if(pause_stick_timer == 0) {
			if(pad_data[0].stick_y <= -PAUSE_STICK_DEADZONE) {
				pause_pos--;
				if(pause_pos < 0) {
					pause_pos = num_pause_options-1;
				}
				pause_stick_timer = PAUSE_STICK_DELAY;
			}
			if(pad_data[0].stick_y >= PAUSE_STICK_DEADZONE) {
				pause_pos++;
				if(pause_pos >= num_pause_options) {
					pause_pos = 0;
				}
				pause_stick_timer = PAUSE_STICK_DELAY;
			}
		} else {
			pause_stick_timer--;
		}
		if(pad_data[0].trigger & (A_BUTTON|START_BUTTON)) {
			pause_options[pause_pos].activate_func();
		}
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
	if(!game_globals.edit_mode) {
		TextDraw(UI_POS_X, 24, TEXT_ALIGNMENT_LEFT, "High Score");
		sprintf(text_buf, "%d", save_data->high_score);
		TextDraw(UI_POS_X, 33, TEXT_ALIGNMENT_LEFT, text_buf);
		TextDraw(UI_POS_X, 60, TEXT_ALIGNMENT_LEFT, "Score");
		sprintf(text_buf, "%d", game_globals.score);
		TextDraw(UI_POS_X, 69, TEXT_ALIGNMENT_LEFT, text_buf);
		TextDraw(UI_POS_X, (SCREEN_H/2)-9, TEXT_ALIGNMENT_LEFT, "Lives");
		sprintf(text_buf, "%d", game_globals.num_lives);
		TextDraw(UI_POS_X, (SCREEN_H/2), TEXT_ALIGNMENT_LEFT, text_buf);
		TextDraw(UI_POS_X, (SCREEN_H-42), TEXT_ALIGNMENT_LEFT, "Level");
		sprintf(text_buf, "%d", game_globals.map_num+1);
		TextDraw(UI_POS_X, (SCREEN_H-33), TEXT_ALIGNMENT_LEFT, text_buf);
	} else {
		TextDraw(UI_POS_X, 24, TEXT_ALIGNMENT_LEFT, "Score");
		sprintf(text_buf, "%d", game_globals.score);
		TextDraw(UI_POS_X, 33, TEXT_ALIGNMENT_LEFT, text_buf);
		TextDraw(UI_POS_X, (SCREEN_H-42), TEXT_ALIGNMENT_LEFT, "Lives");
		sprintf(text_buf, "%d", game_globals.num_lives);
		TextDraw(UI_POS_X, (SCREEN_H-33), TEXT_ALIGNMENT_LEFT, text_buf);
	}
}

static void DrawPauseOtions()
{
	int i;
	for(i=0; i<num_pause_options; i++) {
		if(pause_pos == i) {
			TextSetColor(0, 255, 0, 255);
		} else {
			TextSetColor(255, 255, 255, 255);
		}
		TextDraw((SCREEN_W/2), pause_options_y+(i*9), TEXT_ALIGNMENT_CENTER, pause_options[i].name);
	}
}

void StageGameDraw()
{
	RenderClear(0, 0, 0);
	if(!paused) {
		SpriteDraw(border_sprite);
		MapDraw();
		DrawPaddle();
		DrawPowerups();
		DrawBalls();
		DrawBullets();
		DrawHUD();
	} else {
		DrawPauseOtions();
	}
}

void StageGameDestroy()
{
	if(!game_globals.edit_mode) {
		game_globals.map_num++;
	}
	MapUnload();
	SpriteDelete(border_sprite);
	free(game_sprites);
	game_sprites = NULL;
}