#include <nusys.h>
#include <stdlib.h>
#include <malloc.h>
#include "main.h"
#include "pad.h"
#include "file.h"
#include "render.h"
#include "game.h"
#include "mapsel.h"
#include "save.h"

extern u8 _codeSegmentEnd[];

static StageEntry stage_table[STAGE_MAX] = {
	{ MapSelectInit, MapSelectUpdate, MapSelectDraw, NULL },
	{ StageGameInit, StageGameUpdate, StageGameDraw, StageGameDestroy },
};

static volatile StageID next_stage;
static StageID curr_stage = STAGE_NONE;

static void GfxCallback(u32 pendingTaskCount)
{
	rand(); //Tick RNG on Every Frame
    if (pendingTaskCount == 0) {
		PadRead();
		stage_table[curr_stage].update_func();
		RenderStartFrame();
		stage_table[curr_stage].draw_func();
		RenderEndFrame();
	}
}

void mainproc(void *dummy)
{
	srand(0xD9ED); //Start with known RNG Value
    nuGfxInit();
	PadInit();
	SaveInit();
	InitHeap(_codeSegmentEnd, (u8 *)OS_PHYSICAL_TO_K0(osMemSize-MAX_FRAMEBUF_SIZE)-_codeSegmentEnd);
    FilePackInit();
	next_stage = STAGE_MAPSELECT;
    while(1) {
		if(curr_stage != STAGE_NONE && stage_table[curr_stage].destroy_func) {
			stage_table[curr_stage].destroy_func();
		}
		nuGfxFuncRemove();
		nuGfxDisplayOff();
		curr_stage = next_stage;
		stage_table[curr_stage].init_func();
		nuGfxDisplayOn();
		nuGfxFuncSet(GfxCallback);
		next_stage = STAGE_NONE;
		while(next_stage == STAGE_NONE);
	}
}

void SetNextStage(StageID stage)
{
	next_stage = stage;
}