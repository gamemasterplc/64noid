#ifndef GAME_H
#define GAME_H

#include "sprite.h"
#include "bool.h"

void StageGameInit();
void StageGameUpdate();
void StageGameDraw();
void StageGameDestroy();

typedef struct game_globals {
	int num_lives;
	int map_num;
	int score;
	bool edit_mode;
} GameGlobals;

extern GameGlobals game_globals;

#endif