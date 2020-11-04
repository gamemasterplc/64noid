#include <nusys.h>
#include <malloc.h>
#include <math.h>
#include "render.h"
#include "bool.h"

//#define ENABLE_FPS_COUNTER

#define GBI_LIST_COUNT 2
#define GBI_LIST_LEN 8192

static OSViMode vi_mode;
static Gfx gbi_list[GBI_LIST_COUNT][GBI_LIST_LEN];
static int gbi_list_idx;
static void *framebuf_list[MAX_FRAMEBUFS];
static void *framebuf_base;
static int fb_width;
static int fb_height;
static N64Rect curr_clip;

Gfx *gbi_ptr;

#ifdef ENABLE_FPS_COUNTER
static u8 fps_digits[] __attribute__((aligned(8))) = {
    0x0F, 0xFF, 0xFF, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0x00, 0xFF, 0x11, 0x1F, 0xF0, 0x00, 0xFF, 0xF1, 0x00,
    0xFF, 0x11, 0x1F, 0xF0, 0xFF, 0x11, 0x1F, 0xF0, 0x00, 0xFF, 0xF1, 0x10, 0xFF, 0x11, 0x11, 0x10,
    0xFF, 0x11, 0x11, 0x10, 0xFF, 0x11, 0x1F, 0xF1, 0xFF, 0x11, 0x1F, 0xF0, 0xFF, 0x11, 0x1F, 0xF0,
    0xFF, 0x10, 0x0F, 0xF1, 0x00, 0x0F, 0xF1, 0x00, 0xFF, 0x10, 0x0F, 0xF1, 0x01, 0x10, 0x0F, 0xF1,
    0x00, 0xFF, 0x11, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x10, 0xFF, 0x11,
    0xFF, 0x10, 0x0F, 0xF1, 0xFF, 0x10, 0x0F, 0xF1, 0xFF, 0x10, 0x0F, 0xF1, 0x00, 0x0F, 0xF1, 0x00,
    0x01, 0x10, 0x0F, 0xF1, 0x00, 0x0F, 0xFF, 0x11, 0x0F, 0xF1, 0x1F, 0x00, 0x01, 0x11, 0x1F, 0xF0,
    0xFF, 0x11, 0x1F, 0xF0, 0x00, 0x00, 0xFF, 0x10, 0x0F, 0xFF, 0xFF, 0x11, 0xFF, 0x10, 0x0F, 0xF1,
    0xFF, 0x10, 0x0F, 0xF1, 0x00, 0x0F, 0xF1, 0x00, 0x00, 0x0F, 0xF0, 0x11, 0x00, 0x00, 0x1F, 0xF0,
    0x0F, 0x11, 0xFF, 0x10, 0x00, 0x00, 0x0F, 0xF1, 0xFF, 0x10, 0x0F, 0xF1, 0x00, 0x0F, 0xF1, 0x10,
    0xFF, 0x11, 0x1F, 0xF0, 0x0F, 0xFF, 0xFF, 0xF1, 0xFF, 0x10, 0x0F, 0xF1, 0x00, 0x0F, 0xF1, 0x00,
    0x0F, 0xF0, 0x11, 0x00, 0xFF, 0x00, 0x0F, 0xF1, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0x00, 0x0F, 0xF1,
    0xFF, 0x10, 0x0F, 0xF1, 0x00, 0x0F, 0xF1, 0x00, 0xFF, 0x10, 0x0F, 0xF1, 0x00, 0x11, 0x1F, 0xF1,
    0x0F, 0xFF, 0xFF, 0x11, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0xFF, 0x11,
    0x01, 0x11, 0xFF, 0x11, 0x0F, 0xFF, 0xFF, 0x11, 0x0F, 0xFF, 0xFF, 0x11, 0x00, 0x0F, 0xF1, 0x00,
    0x0F, 0xFF, 0xFF, 0x11, 0x0F, 0xFF, 0xFF, 0x11, 0x00, 0x11, 0x11, 0x10, 0x00, 0x01, 0x11, 0x10,
    0x01, 0x11, 0x11, 0x11, 0x00, 0x11, 0x11, 0x10, 0x00, 0x00, 0x01, 0x10, 0x00, 0x11, 0x11, 0x10,
    0x00, 0x11, 0x11, 0x10, 0x00, 0x00, 0x11, 0x00, 0x00, 0x11, 0x11, 0x10, 0x00, 0x11, 0x11, 0x10
};
#endif

static Gfx rdpinit_dl[] = {
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPPipeSync(),
	gsDPSetTexturePersp(G_TP_NONE),
    gsSPEndDisplayList(),
};

static void MakeRenderMode(OSViMode *mode, int width, int height)
{
	mode->comRegs.width = width;
	mode->comRegs.xScale = (width*512)/320;
	if(height > 240) {
		mode->comRegs.ctrl |= 0x40; //Forces Serrate Bit On
		#ifdef FRAMEBUF_32BIT
		mode->fldRegs[0].origin = width*4;
		mode->fldRegs[1].origin = width*8;
		#else
		mode->fldRegs[0].origin = width*2;
		mode->fldRegs[1].origin = width*4;
		#endif
		mode->fldRegs[0].yScale = 0x2000000|((height*1024)/240);
		mode->fldRegs[1].yScale = 0x2000000|((height*1024)/240);
		mode->fldRegs[0].vStart = mode->fldRegs[1].vStart-0x20002;
	} else {
		#ifdef FRAMEBUF_32BIT
		mode->fldRegs[0].origin = width*4;
		mode->fldRegs[1].origin = width*4;
		#else
		mode->fldRegs[0].origin = width*2;
		mode->fldRegs[1].origin = width*2;
		#endif
		mode->fldRegs[0].yScale = ((height*1024)/240);
		mode->fldRegs[1].yScale = ((height*1024)/240);
	}
}

static void PreNMICallback()
{
	nuGfxDisplayOff();
	osViSetYScale(1);
}

void RenderSetSize(int width, int height)
{
	int i;
	if(width > MAX_FRAMEBUF_W) {
		width = MAX_FRAMEBUF_W;
	}
	if(height > MAX_FRAMEBUF_H) {
		height = MAX_FRAMEBUF_H;
	}
	framebuf_base = OS_PHYSICAL_TO_K0(osMemSize-MAX_FRAMEBUF_SIZE);
	for(i=0; i<MAX_FRAMEBUFS; i++) {
		u32 framebuf_ofs;
		#ifdef FRAMEBUF_32BIT
		framebuf_ofs = (((width*height*4)+63) & 0xFFFFFFC0)*i;
		framebuf_list[i] = (u8 *)framebuf_base+framebuf_ofs;
		memset(framebuf_list[i], 0, (((width*height*4)+63) & 0xFFFFFFC0));
		#else
		framebuf_ofs = (((width*height*2)+63) & 0xFFFFFFC0)*i;
		framebuf_list[i] = (u8 *)framebuf_base+framebuf_ofs;
		memset(framebuf_list[i], 0, (((width*height*2)+63) & 0xFFFFFFC0));
		#endif
	}
	fb_width = width;
	fb_height = height;
	gbi_list_idx = 0;
	nuGfxTaskAllEndWait();
	#ifdef FRAMEBUF_32BIT
	switch(osTvType) {
		case 0:
			memcpy(&vi_mode, &osViModePalLpn2, sizeof(OSViMode));
			break;
			
		case 1:
			memcpy(&vi_mode, &osViModeNtscLpn2, sizeof(OSViMode));
			break;
			
		case 2:
			memcpy(&vi_mode, &osViModeMpalLpn2, sizeof(OSViMode));
			break;
	}
	#else
	switch(osTvType) {
		case 0:
			memcpy(&vi_mode, &osViModePalLpn1, sizeof(OSViMode));
			break;
			
		case 1:
			memcpy(&vi_mode, &osViModeNtscLpn1, sizeof(OSViMode));
			break;
			
		case 2:
			memcpy(&vi_mode, &osViModeMpalLpn1, sizeof(OSViMode));
			break;
	}
	#endif
	MakeRenderMode(&vi_mode, width, height);
	osViSetMode(&vi_mode);
	osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
	if(osTvType == 0) {
		osViSetYScale(0.833);
	}
	nuPreNMIFuncSet((NUScPreNMIFunc)PreNMICallback);
	nuGfxSetCfb((u16 **)framebuf_list, MAX_FRAMEBUFS);
}

static void FlushClipRect()
{
	gDPSetScissor(gbi_ptr++, G_SC_NON_INTERLACE, curr_clip.left, curr_clip.top, curr_clip.right, curr_clip.bottom);
}

void RenderClear(u8 r, u8 g, u8 b)
{
	gDPSetCycleType(gbi_ptr++, G_CYC_FILL);
	#ifdef FRAMEBUF_32BIT
	gDPSetFillColor(gbi_ptr++, (r << 24)|(g << 16)|(b << 8)|255);
	#else
	gDPSetFillColor(gbi_ptr++, (GPACK_RGBA5551(r, g, b, 1) << 16) | GPACK_RGBA5551(r, g, b, 1));
	#endif
	gDPFillRectangle(gbi_ptr++, 0, 0, fb_width-1, fb_height-1);
	gDPPipeSync(gbi_ptr++);
	gDPSetCycleType(gbi_ptr++, G_CYC_1CYCLE);
}

void RenderSetClipRect(N64Rect *clip)
{
	curr_clip = *clip;
	FlushClipRect();
}

void RenderResetClipRect()
{
	curr_clip.left = 0;
	curr_clip.top = 0;
	curr_clip.right = fb_width;
	curr_clip.bottom = fb_height;
	FlushClipRect();
}

void RenderStartFrame()
{
	gbi_ptr = gbi_list[gbi_list_idx];
    // Set the segment register
    gSPSegment(gbi_ptr++, 0, 0);
    gSPDisplayList(gbi_ptr++, OS_K0_TO_PHYSICAL(rdpinit_dl));
	#ifdef FRAMEBUF_32BIT
	gDPSetColorImage(gbi_ptr++, G_IM_FMT_RGBA, G_IM_SIZ_32b, fb_width, OS_K0_TO_PHYSICAL(nuGfxCfb_ptr)); 
	#else
	gDPSetColorImage(gbi_ptr++, G_IM_FMT_RGBA, G_IM_SIZ_16b, fb_width, OS_K0_TO_PHYSICAL(nuGfxCfb_ptr)); 
	#endif
	RenderResetClipRect();
}

#ifdef ENABLE_FPS_COUNTER
static void DrawFPS()
{
	static OSTime last_time = 0;
	int text_x = fb_width-40;
	int text_y = 16;
	if(last_time == 0) {
		last_time = osGetTime();
	} else {
		OSTime this_time = osGetTime();
		float time_float = (float)(OS_CYCLES_TO_USEC(this_time-last_time)/1000)/1000;
		int fps = 1/time_float;
		if(fps > 99) {
			fps = 99;
		}
		last_time = this_time;
		gDPSetCombineMode(gbi_ptr++, G_CC_DECALRGBA, G_CC_DECALRGBA);
		gDPSetRenderMode(gbi_ptr++, G_RM_XLU_SURF, G_RM_XLU_SURF);
		gDPSetTextureLUT(gbi_ptr++, G_TT_NONE);
		gDPLoadTextureTile_4b(gbi_ptr++, fps_digits, G_IM_FMT_IA, 80, 8, 0, 0, 79, 7, 0, G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
		gSPTextureRectangle(gbi_ptr++, (text_x*4), (text_y*4), (text_x+8)*4, (text_y+8)*4, 0, (fps/10)*256, 0, 1024, 1024);
		text_x += 8;
		gSPTextureRectangle(gbi_ptr++, (text_x*4), (text_y*4), (text_x+8)*4, (text_y+8)*4, 0, (fps%10)*256, 0, 1024, 1024);
	}
}
#endif

void RenderEndFrame()
{
	#ifdef ENABLE_FPS_COUNTER
	DrawFPS();
	#endif
	gDPFullSync(gbi_ptr++);
    gSPEndDisplayList(gbi_ptr++);
    nuGfxTaskStart(gbi_list[gbi_list_idx], (s32)(gbi_ptr - gbi_list[gbi_list_idx]) * sizeof(Gfx), NU_GFX_UCODE_S2DEX, NU_SC_SWAPBUFFER);
	gbi_list_idx++;
	gbi_list_idx %= GBI_LIST_COUNT;
}

N64Rect *RenderGetCurrClipRect()
{
	return &curr_clip;
}