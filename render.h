#ifndef RENDER_H
#define RENDER_H

#include <ultra64.h>
#include "bool.h"

#define MAX_FRAMEBUF_W 424
#define MAX_FRAMEBUF_H 240
//#define FRAMEBUF_32BIT
#define MAX_FRAMEBUFS 2
#define TARGET_ALIGNMENT 64
#ifdef FRAMEBUF_32BIT
#define MAX_FRAMEBUF_SIZE ((((MAX_FRAMEBUF_W*MAX_FRAMEBUF_H*4)+TARGET_ALIGNMENT-1)&~(TARGET_ALIGNMENT-1))*MAX_FRAMEBUFS)
#else
#define MAX_FRAMEBUF_SIZE ((((MAX_FRAMEBUF_W*MAX_FRAMEBUF_H*2)+TARGET_ALIGNMENT-1)&~(TARGET_ALIGNMENT-1))*MAX_FRAMEBUFS)
#endif

#define RENDER_MODE_SPRITE 0
#define RENDER_MODE_SPRITE_TINT 1
#define RENDER_MODE_RECT 2
#define RENDER_MODE_CLEAR 3
#define RENDER_MODE_TEXT 4

void RenderSetSize(int width, int height);
void RenderClear(u8 r, u8 g, u8 b);
void RenderSetScissor(int x, int y, int w, int h);
void RenderResetScissor();
void RenderStartFrame();
void RenderEndFrame();
void RenderPutRect(int x, int y, int w, int h, u8 r, u8 g, u8 b, u8 a);
void RenderStartFade(bool fade_in, int duration);
bool RenderIsFadeOver();

extern Gfx *render_dl_ptr;
extern int render_mode;

#endif