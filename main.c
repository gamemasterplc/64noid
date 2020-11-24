#include <nusys.h>
#include <stdlib.h>
#include <malloc.h>
#include "main.h"
#include "pad.h"
#include "file.h"
#include "render.h"
#include "game.h"
#include "mapsel.h"
#include "nextmap.h"
#include "stageend.h"
#include "mapedit.h"
#include "title.h"
#include "save.h"

extern u8 _codeSegmentEnd[];

static StageEntry stage_table[STAGE_MAX] = {
	{ MapSelectInit, MapSelectUpdate, MapSelectDraw, NULL },
	{ NextMapInit, NextMapUpdate, NextMapDraw, NULL },
	{ StageGameInit, StageGameUpdate, StageGameDraw, StageGameDestroy },
	{ StageEndInit, StageEndUpdate, StageEndDraw, StageEndDestroy },
	{ TitleInit, TitleUpdate, TitleDraw, TitleDestroy },
	{ MapEditorInit, MapEditorUpdate, MapEditorDraw, MapEditorDestroy },
};

static volatile StageID next_stage;
static volatile StageID transition_stage;
static StageID curr_stage = STAGE_NONE;

static void GfxCallback(u32 pendingTaskCount)
{
	rand(); //Tick RNG on Every Frame
    if (pendingTaskCount == 0) {
		PadRead();
		if(RenderIsFadeOver() && transition_stage == STAGE_NONE) {
			stage_table[curr_stage].update_func();
		} else {
			if(RenderIsFadeOver()) {
				next_stage = transition_stage;
			}
		}
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
	next_stage = STAGE_TITLE;
    while(1) {
		if(curr_stage != STAGE_NONE && stage_table[curr_stage].destroy_func) {
			stage_table[curr_stage].destroy_func();
		}
		nuGfxFuncRemove();
		nuGfxDisplayOff();
		curr_stage = next_stage;
		stage_table[curr_stage].init_func();
		RenderStartFade(true, 16);
		nuGfxDisplayOn();
		nuGfxFuncSet(GfxCallback);
		transition_stage = next_stage = STAGE_NONE;
		while(next_stage == STAGE_NONE);
	}
}

void SetNextStage(StageID stage)
{
	transition_stage = stage;
	RenderStartFade(false, 16);
}