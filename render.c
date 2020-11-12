#include <nusys.h>
#include <malloc.h>
#include <math.h>
#include "render.h"
#include "text.h"
#include "bool.h"

#define ENABLE_FPS_COUNTER

#define GBI_LIST_COUNT 2
#define GBI_LIST_LEN 8192

static OSViMode vi_mode;
static Gfx gbi_list[GBI_LIST_COUNT][GBI_LIST_LEN];
static int gbi_list_idx;
static void *framebuf_list[MAX_FRAMEBUFS];
static void *framebuf_base;
static int fb_width;
static int fb_height;

Gfx *render_dl_ptr;
int render_mode;

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
	//Attempt to Prevent Reset Crash
	nuGfxDisplayOff();
	osViSetYScale(1);
}

void RenderSetSize(int width, int height)
{
	int i;
	//Clamp Framebuffer Size
	if(width > MAX_FRAMEBUF_W) {
		width = MAX_FRAMEBUF_W;
	}
	if(height > MAX_FRAMEBUF_H) {
		height = MAX_FRAMEBUF_H;
	}
	framebuf_base = OS_PHYSICAL_TO_K0(osMemSize-MAX_FRAMEBUF_SIZE);
	for(i=0; i<MAX_FRAMEBUFS; i++) {
		u32 framebuf_ofs;
		u32 framebuf_size;
		//Initialize Framebuffers
		#ifdef FRAMEBUF_32BIT
		framebuf_size = (((width*height*4)+63) & 0xFFFFFFC0);
		framebuf_ofs = framebuf_size*i;
		framebuf_list[i] = (u8 *)framebuf_base+framebuf_ofs;
		memset(framebuf_list[i], 0, framebuf_size);
		#else
		framebuf_size = (((width*height*2)+63) & 0xFFFFFFC0);
		framebuf_ofs = framebuf_size*i;
		framebuf_list[i] = (u8 *)framebuf_base+framebuf_ofs;
		memset(framebuf_list[i], 0, framebuf_size);
		#endif
	}
	//Initialize Render Properies
	fb_width = width;
	fb_height = height;
	gbi_list_idx = 0;
	nuGfxTaskAllEndWait();
	#ifdef FRAMEBUF_32BIT
	//Copy 32-bit VI Mode
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
	//Copy 16-bit VI Mode
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
	//Initialize VI Mode
	MakeRenderMode(&vi_mode, width, height);
	osViSetMode(&vi_mode);
	osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
	if(osTvType == 0) {
		osViSetYScale(0.833);
	}
	nuPreNMIFuncSet((NUScPreNMIFunc)PreNMICallback); //Callback to attempt to prevent reset crash
	nuGfxSetCfb((u16 **)framebuf_list, MAX_FRAMEBUFS); //Initialize CFB List
}

void RenderClear(u8 r, u8 g, u8 b)
{
	//Enable Render Settings for Framebuffer Clear
	gDPSetCycleType(render_dl_ptr++, G_CYC_FILL);
	if(render_mode != RENDER_MODE_CLEAR) {
		gDPSetRenderMode(render_dl_ptr++, G_RM_NOOP, G_RM_NOOP2);
		render_mode = RENDER_MODE_CLEAR;
	}
	//Draw Clear Rectangle
	#ifdef FRAMEBUF_32BIT
	gDPSetFillColor(render_dl_ptr++, (r << 24)|(g << 16)|(b << 8)|255);
	#else
	gDPSetFillColor(render_dl_ptr++, (GPACK_RGBA5551(r, g, b, 1) << 16) | GPACK_RGBA5551(r, g, b, 1));
	#endif
	gDPFillRectangle(render_dl_ptr++, 0, 0, fb_width-1, fb_height-1);
	gDPPipeSync(render_dl_ptr++);
	//Enable 1-Cycle Mode for Further Rendering
	gDPSetCycleType(render_dl_ptr++, G_CYC_1CYCLE);
}

void RenderSetScissor(int x, int y, int w, int h)
{
	gDPSetScissor(render_dl_ptr++, G_SC_NON_INTERLACE, x, y, x+w, y+h);
}

void RenderResetScissor()
{
	//Set Scissor to Fullscreen
	gDPSetScissor(render_dl_ptr++, G_SC_NON_INTERLACE, 0, 0, fb_width, fb_height);
}

void RenderStartFrame()
{
	render_dl_ptr = gbi_list[gbi_list_idx]; //Initialize Display Pointer
    // Set the segment register
    gSPSegment(render_dl_ptr++, 0, 0);
    gSPDisplayList(render_dl_ptr++, OS_K0_TO_PHYSICAL(rdpinit_dl));
	//Set Framebuffer Pointer
	#ifdef FRAMEBUF_32BIT
	gDPSetColorImage(render_dl_ptr++, G_IM_FMT_RGBA, G_IM_SIZ_32b, fb_width, OS_K0_TO_PHYSICAL(nuGfxCfb_ptr)); 
	#else
	gDPSetColorImage(render_dl_ptr++, G_IM_FMT_RGBA, G_IM_SIZ_16b, fb_width, OS_K0_TO_PHYSICAL(nuGfxCfb_ptr)); 
	#endif
	//Initialize Rendering
	render_mode = RENDER_MODE_SPRITE;
	gDPSetCombineMode(render_dl_ptr++, G_CC_DECALRGBA, G_CC_DECALRGBA);
	gDPSetRenderMode(render_dl_ptr++, G_RM_XLU_SURF, G_RM_XLU_SURF);
	RenderResetScissor();
	TextSetColor(255, 255, 255, 255); //Force Text Color to White
}

#ifdef ENABLE_FPS_COUNTER
static void DrawFPS()
{
	static OSTime last_time = 0; //Persistent Time Variable for FPS Calculation
	//Local Variables for Text Position
	int text_x = fb_width-40;
	int text_y = 16;
	if(last_time == 0) {
		last_time = osGetTime();
	} else {
		OSTime this_time = osGetTime();
		float delta_time = (float)(OS_CYCLES_TO_USEC(this_time-last_time)/1000)/1000;
		int fps = 1/delta_time; //Inverse of Delta Time
		//Clamp FPS to 99
		if(fps > 99) {
			fps = 99;
		}
		last_time = this_time;
		//Setup Render Mode to Draw FPS Digits Sprites
		if(render_mode != RENDER_MODE_SPRITE) {
			gDPSetCombineMode(render_dl_ptr++, G_CC_DECALRGBA, G_CC_DECALRGBA);
			gDPSetRenderMode(render_dl_ptr++, G_RM_XLU_SURF, G_RM_XLU_SURF);
			render_mode = RENDER_MODE_SPRITE;
		}
		//Load FPS Digit Textures
		gDPSetTextureLUT(render_dl_ptr++, G_TT_NONE);
		gDPLoadTextureTile_4b(render_dl_ptr++, fps_digits, G_IM_FMT_IA, 80, 8, 0, 0, 79, 7, 0, G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
		//Draw FPS Digits
		gSPTextureRectangle(render_dl_ptr++, (text_x*4), (text_y*4), (text_x+8)*4, (text_y+8)*4, 0, (fps/10)*256, 0, 1024, 1024);
		text_x += 8;
		gSPTextureRectangle(render_dl_ptr++, (text_x*4), (text_y*4), (text_x+8)*4, (text_y+8)*4, 0, (fps%10)*256, 0, 1024, 1024);
	}
}
#endif

void RenderEndFrame()
{
	#ifdef ENABLE_FPS_COUNTER
	DrawFPS();
	#endif
	//Write Display List End and Send Task
	gDPFullSync(render_dl_ptr++);
    gSPEndDisplayList(render_dl_ptr++);
    nuGfxTaskStart(gbi_list[gbi_list_idx], (s32)(render_dl_ptr - gbi_list[gbi_list_idx]) * sizeof(Gfx), NU_GFX_UCODE_S2DEX, NU_SC_SWAPBUFFER);
	//Change to Next Task
	gbi_list_idx++;
	gbi_list_idx %= GBI_LIST_COUNT;
}

void RenderPutRect(int x, int y, int w, int h, u8 r, u8 g, u8 b, u8 a)
{
	//Setup Rectangle Drawing
	if(render_mode != RENDER_MODE_RECT) {
		gDPSetCombineMode(render_dl_ptr++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
		gDPSetRenderMode(render_dl_ptr++, G_RM_XLU_SURF, G_RM_XLU_SURF);
		render_mode = RENDER_MODE_RECT;
	}
	//Draw Rectangle
	gDPSetPrimColor(render_dl_ptr++, 0, 0, r, g, b, a);
	gDPScisFillRectangle(render_dl_ptr++, x, y, x+w, y+h);
}