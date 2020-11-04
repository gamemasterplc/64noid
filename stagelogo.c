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
}

void StageLogoUpdate()
{
	if(cont_data[0].button & R_TRIG) {
		angle += 256;
	}
	if(cont_data[0].button & L_TRIG) {
		angle -= 256;
	}
	if(cont_data[0].trigger & A_BUTTON) {
		image_idx++;
		if(image_idx >= 3) {
			image_idx = 0;
		}
	}
	if(cont_data[0].trigger & B_BUTTON) {
		image_idx--;
		if(image_idx < 0) {
			image_idx = 2;
		}
	}
	SpriteSetImageIndex(n64brew, image_idx);
	SpriteSetAngle(n64brew, angle);
}

void StageLogoDraw()
{
    RenderStartFrame();
	RenderClear(0, 0, 255);
	SpriteDraw(n64brew);
	RenderEndFrame();
}

void StageLogoDestroy()
{
    SpriteDestroy(n64brew);
}