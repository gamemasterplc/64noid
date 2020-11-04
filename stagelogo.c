#include <PR/ultratypes.h>
#include "render.h"
#include "sprite.h"
#include "controller.h"

static SpriteInfo *n64brew;
static u16 angle;
static int image_idx;

void StageLogoInit()
{
	RenderSetSize(424, 240);
	angle = 0;
	image_idx = 0;
	n64brew = SpriteCreate(SpriteLoadFile("n64brew.spr"));
	SpriteSetPos(n64brew, 212, 120);
	SpriteSetImage(n64brew, "world");
}

void StageLogoUpdate()
{
	if(cont_data[0].button & R_TRIG) {
		angle += 256;
	}
	if(cont_data[0].button & L_TRIG) {
		angle -= 256;
	}
	SpriteSetAngle(n64brew, angle);
}

void StageLogoDraw()
{
    RenderStartFrame();
	RenderClear(0, 0, 255);
	RenderPutRect(24, 16, (angle*376)/65536, 4, 255, 0, 0, 255);
	SpriteDraw(n64brew);
	RenderEndFrame();
}

void StageLogoDestroy()
{
    SpriteDestroy(n64brew);
}