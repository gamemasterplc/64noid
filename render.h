#ifndef RENDER_H
#define RENDER_H

#include <ultra64.h>
#include "bool.h"

#define MAX_FRAMEBUF_W 640
#define MAX_FRAMEBUF_H 480
//#define FRAMEBUF_32BIT
#define MAX_FRAMEBUFS 2
#define TARGET_ALIGNMENT 64
#ifdef FRAMEBUF_32BIT
#define MAX_FRAMEBUF_SIZE ((((MAX_FRAMEBUF_W*MAX_FRAMEBUF_H*4)+TARGET_ALIGNMENT-1)&~(TARGET_ALIGNMENT-1))*MAX_FRAMEBUFS)
#else
#define MAX_FRAMEBUF_SIZE ((((MAX_FRAMEBUF_W*MAX_FRAMEBUF_H*2)+TARGET_ALIGNMENT-1)&~(TARGET_ALIGNMENT-1))*MAX_FRAMEBUFS)
#endif

#define RENDER_MODE_IMAGE 0
#define RENDER_MODE_IMAGE_TINT 1
#define RENDER_MODE_BITMAP 2

typedef struct n64_rect {
	int left;
	int top;
	int right;
	int bottom;
} N64Rect;

void RenderSetSize(int width, int height);
void RenderClear(u8 r, u8 g, u8 b);
void RenderSetClipRect(N64Rect *clip);
void RenderResetClipRect();
void RenderStartFrame();
void RenderEndFrame();

extern Gfx *gbi_ptr;

#endif