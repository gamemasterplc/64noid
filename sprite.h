#ifndef SPRITE_H
#define SPRITE_H

#include <ultra64.h>
#include <PR/gs2dex.h>

#define SPRITE_IMG_FORMAT_I4 0
#define SPRITE_IMG_FORMAT_I8 1
#define SPRITE_IMG_FORMAT_IA4 2
#define SPRITE_IMG_FORMAT_IA8 3
#define SPRITE_IMG_FORMAT_IA16 4
#define SPRITE_IMG_FORMAT_CI4 5
#define SPRITE_IMG_FORMAT_CI8 6
#define SPRITE_IMG_FORMAT_RGBA16 7
#define SPRITE_IMG_FORMAT_RGBA32 8
#define SPRITE_IMG_FORMAT_MAX 9

#define SPRITE_PAL_FORMAT_RGBA16 0
#define SPRITE_PAL_FORMAT_IA16 1
#define SPRITE_PAL_FORMAT_DEFAULT 2
#define SPRITE_PAL_FORMAT_MAX 3

#define SPRITE_FLIP_NONE 0x0
#define SPRITE_FLIP_X 0x1
#define SPRITE_FLIP_Y 0x2
#define SPRITE_FLIP_XY 0x3

#define SPRITE_ATTR_NONE 0
#define SPRITE_ATTR_INVISIBLE 0x1
#define SPRITE_ATTR_BACKWARDS_ANIM 0x2
#define SPRITE_ATTR_ENABLE_TINT 0x4

typedef struct sprite_anim_frame {
    u16 image;
    u16 delay;
} SpriteAnimFrame;

typedef struct sprite_anim {
    u32 name_hash;
    u32 num_frames;
    SpriteAnimFrame *frames;
} SpriteAnim;

typedef struct sprite_image {
    u32 id_hash;
    void *data;
    void *pal_data;
	uObjSprite *s2d_sprites;
	Gfx *s2d_gbi;
	s16 origin_x;
	s16 origin_y;
    u16 format;
    u16 pal_format;
    u16 w;
    u16 h;
} SpriteImage;

typedef struct sprite_data {
    u16 magic;
	u16 num_refs;
    u16 num_anims;
    u16 num_images;
    SpriteAnim *anims;
    SpriteImage *images;
} SpriteData;

typedef struct sprite_info {
	SpriteData *data;
	SpriteAnim *curr_anim;
	SpriteImage *curr_image;
	unsigned int attr;
	uObjMtx matrix;
	float x;
	float y;
	float x_scale;
	float y_scale;
	float anim_time;
	float anim_speed;
	u16 angle;
	u16 anim_frame;
	u8 tint_r;
	u8 tint_g;
	u8 tint_b;
	u8 tint_a;
} SpriteInfo;

SpriteData *SpriteLoadMemory(void *ptr);
SpriteData *SpriteLoadFile(const char *filename);
SpriteInfo *SpriteCreate(SpriteData *data);
void SpriteDestroy(SpriteInfo *sprite);
void SpriteSetFlip(SpriteInfo *sprite, unsigned int mask);
void SpriteResetFlip(SpriteInfo *sprite, unsigned int mask);
void SpriteSetAttr(SpriteInfo *sprite, unsigned int mask);
void SpriteResetAttr(SpriteInfo *sprite, unsigned int mask);
void SpriteSetPos(SpriteInfo *sprite, float x, float y);
void SpriteSetScale(SpriteInfo *sprite, float x, float y);
void SpriteSetAngle(SpriteInfo *sprite, u16 angle);
void SpriteSetAngleDeg(SpriteInfo *sprite, float angle);
void SpriteSetAnimSpeed(SpriteInfo *sprite, float speed);
void SpriteSetAnim(SpriteInfo *sprite, const char *id);
void SpriteSetAnimIndex(SpriteInfo *sprite, int index);
void SpriteSetImage(SpriteInfo *sprite, const char *id);
void SpriteSetImageIndex(SpriteInfo *sprite, int index);
void SpriteSetTint(SpriteInfo *sprite, u8 r, u8 g, u8 b, u8 a);
void SpriteDraw(SpriteInfo *sprite);

#endif