#include <ultra64.h>
#include <math.h>
#include <malloc.h>
#include "sprite.h"
#include "file.h"
#include "hash.h"
#include "render.h"

#define PATCH_PTR(ptr, base, type) ((ptr) = ((ptr) ? (type *)((u32)(base)+(u32)(ptr)) : NULL))

static void PatchSpriteAnim(SpriteAnim *anim_base, u16 num_anims)
{
	u16 i;
	for(i=0; i<num_anims; i++) {
		PATCH_PTR(anim_base[i].frames, anim_base, SpriteAnimFrame);
	}
}

static u8 bpp_table[SPRITE_IMG_FORMAT_MAX] = { 4, 8, 4, 8, 16, 4, 8, 16, 32 };
static int slice_pixel_cnt[SPRITE_IMG_FORMAT_MAX] = { 8192, 4096, 8192, 4096, 2048, 4096, 2048, 2048, 1024 };
static u8 fmt_tile_bytes[SPRITE_IMG_FORMAT_MAX] = { 0, 1, 0, 1, 2, 0, 1, 2, 2 };

static u8 image_fmt_tbl[SPRITE_IMG_FORMAT_MAX] = {
	G_IM_FMT_I,
	G_IM_FMT_I,
	G_IM_FMT_IA,
	G_IM_FMT_IA,
	G_IM_FMT_IA,
	G_IM_FMT_CI,
	G_IM_FMT_CI,
	G_IM_FMT_RGBA,
	G_IM_FMT_RGBA
};

static u8 image_size_tbl[SPRITE_IMG_FORMAT_MAX] = {
	G_IM_SIZ_4b,
	G_IM_SIZ_8b,
	G_IM_SIZ_4b,
	G_IM_SIZ_8b,
	G_IM_SIZ_16b,
	G_IM_SIZ_4b,
	G_IM_SIZ_8b,
	G_IM_SIZ_16b,
	G_IM_SIZ_32b
};

static u32 pal_formats[SPRITE_PAL_FORMAT_MAX] = {
	G_TT_RGBA16,
	G_TT_IA16,
	G_TT_NONE
};

static void MakeS2DSprite(SpriteImage *image, u16 spr_index, u16 y, u16 h)
{
	uObjSprite_t *s2d_sprite = &image->s2d_sprites[spr_index].s;
	s2d_sprite->objX = -image->origin_x;
	s2d_sprite->objY = y;
	s2d_sprite->scaleW = s2d_sprite->scaleH = 1024;
	s2d_sprite->imageW = image->w << 5;
	s2d_sprite->imageH = h << 5;
	s2d_sprite->paddingX = s2d_sprite->paddingY = 0;
	if(fmt_tile_bytes[image->format] != 0) {
		s2d_sprite->imageStride = (((image->w)*fmt_tile_bytes[image->format])+7)>>3;
	} else {
		s2d_sprite->imageStride = (((image->w)>>1)+7)>>3;
	}
	s2d_sprite->imageAdrs = 0;
	s2d_sprite->imageFmt = image_fmt_tbl[image->format];
	s2d_sprite->imageSiz = image_size_tbl[image->format];
	s2d_sprite->imagePal = 0;
	s2d_sprite->imageFlags = 0;
}

static void MakeS2DSprites(SpriteImage *image)
{
	u16 slice_h = slice_pixel_cnt[image->format]/image->w;
	u16 slice_cnt = image->h/slice_h;
	u16 total_slice_cnt = slice_cnt;
	u16 remainder_h = image->h%slice_h;
	s16 slice_y = -image->origin_y;
	u16 i;
	if(remainder_h != 0) {
		total_slice_cnt = slice_cnt+1;
	}
	image->s2d_sprites = malloc(total_slice_cnt*sizeof(uObjSprite));
	for(i=0; i<slice_cnt; i++) {
		MakeS2DSprite(image, i, slice_y, slice_h);
		slice_y += slice_h*4;
	}
	if(remainder_h != 0) {
		MakeS2DSprite(image, i, slice_y, remainder_h);
	}
	osWritebackDCache(image->s2d_sprites, total_slice_cnt*sizeof(uObjSprite));
}

static u32 GetS2DDispListSize(SpriteImage *image)
{
	u16 slice_h = slice_pixel_cnt[image->format]/image->w;
	u16 slice_cnt = image->h/slice_h;
	u16 total_slice_cnt = slice_cnt;
	u16 remainder_h = image->h%slice_h;
	u32 num_cmds = 0;
	if(remainder_h != 0) {
		total_slice_cnt = slice_cnt+1;
	}
	num_cmds += (9*total_slice_cnt); //Texture Load, Sprite Draw, and Pipe Sync for Each Sprite
	num_cmds += 2; //Palette Format Set and Return
	if(image_fmt_tbl[image->format] == G_IM_FMT_CI) {
		num_cmds += 6; //Palette Load Command
	}
	return num_cmds*sizeof(Gfx);
}

static void LoadImagePalette(Gfx **gbi, SpriteImage *image)
{
	switch(image->format) {
		case SPRITE_IMG_FORMAT_CI4:
			gDPLoadTLUT_pal16((*gbi)++, 0, image->pal_data);
			break;
			
		case SPRITE_IMG_FORMAT_CI8:
			gDPLoadTLUT_pal256((*gbi)++, image->pal_data);
			break;
			
		default:
			break;
	}
}

static void LoadImage(Gfx **gbi, SpriteImage *image, u16 uls, u16 ult, u16 lrs, u16 lrt)
{
	switch(image->format) {
		case SPRITE_IMG_FORMAT_I4:
			gDPLoadTextureTile_4b((*gbi)++, image->data, G_IM_FMT_I, image->w, image->h, uls, ult,
				lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case SPRITE_IMG_FORMAT_I8:
			gDPLoadTextureTile((*gbi)++, image->data, G_IM_FMT_I, G_IM_SIZ_8b, image->w, image->h, uls, ult,
				lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case SPRITE_IMG_FORMAT_IA4:
			gDPLoadTextureTile_4b((*gbi)++, image->data, G_IM_FMT_IA, image->w, image->h, uls, ult,
				lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case SPRITE_IMG_FORMAT_IA8:
			gDPLoadTextureTile((*gbi)++, image->data, G_IM_FMT_IA, G_IM_SIZ_8b, image->w, image->h, uls, ult,
				lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case SPRITE_IMG_FORMAT_IA16:
			gDPLoadTextureTile((*gbi)++, image->data, G_IM_FMT_IA, G_IM_SIZ_16b, image->w, image->h, uls, ult,
				lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case SPRITE_IMG_FORMAT_CI4:
			gDPLoadTextureTile_4b((*gbi)++, image->data, G_IM_FMT_CI, image->w, image->h, uls, ult,
				lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case SPRITE_IMG_FORMAT_CI8:
			gDPLoadTextureTile((*gbi)++, image->data, G_IM_FMT_CI, G_IM_SIZ_8b, image->w, image->h, uls, ult,
				lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case SPRITE_IMG_FORMAT_RGBA16:
			gDPLoadTextureTile((*gbi)++, image->data, G_IM_FMT_RGBA, G_IM_SIZ_16b, image->w, image->h, uls, ult,
				lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case SPRITE_IMG_FORMAT_RGBA32:
			gDPLoadTextureTile((*gbi)++, image->data, G_IM_FMT_RGBA, G_IM_SIZ_32b, image->w, image->h, uls, ult,
				lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		default:
			break;
	}
}

static void MakeS2DDispList(SpriteImage *image)
{
	u32 disp_size = GetS2DDispListSize(image);
	Gfx *gbi_base = malloc(disp_size);
	Gfx *gbi = gbi_base;
	u16 slice_h = GetSliceHeight(image);
	u16 slice_cnt = image->h/slice_h;
	u16 remainder_h = image->h%slice_h;
	u16 y = 0;
	u16 i;
	gDPSetTextureLUT(gbi++, pal_formats[image->pal_format]);
	LoadImagePalette(&gbi, image);
	for(i=0; i<slice_cnt; i++) {
		LoadImage(&gbi, image, 0, y, image->w-1, y+slice_h-1);
		gSPObjSprite(gbi++, &image->s2d_sprites[i]);
		gDPPipeSync(gbi++);
		y += slice_h;
	}
	if(remainder_h) {
		LoadImage(&gbi, image, 0, y, image->w-1, y+remainder_h-1);
		gSPObjSprite(gbi++, &image->s2d_sprites[i]);
		gDPPipeSync(gbi++);
	}
	gSPEndDisplayList(gbi++);
	osWritebackDCache(gbi_base, disp_size);
	image->s2d_gbi = gbi_base;
}

static void LoadSpriteImage(SpriteImage *image_base, u16 num_images)
{
	u16 i;
	for(i=0; i<num_images; i++) {
		PATCH_PTR(image_base[i].data, image_base, void);
		PATCH_PTR(image_base[i].pal_data, image_base, void);
		MakeS2DSprites(&image_base[i]);
		MakeS2DDispList(&image_base[i]);
	}
}

SpriteData *SpriteLoadMemory(void *ptr)
{
	SpriteData *data = (SpriteData *)ptr;
	if(data->magic != 'SL') {
		PATCH_PTR(data->anims, data, SpriteAnim);
		PATCH_PTR(data->images, data, SpriteImage);
		if(data->anims) {
			PatchSpriteAnim(data->anims, data->num_anims);
		}
		LoadSpriteImage(data->images, data->num_images);
		data->magic = 'SL';
		data->num_refs = 0;
	} else {
		data->num_refs++;
	}
	return data;
}

SpriteData *SpriteLoadFile(const char *filename)
{
	return SpriteLoadMemory(FileRead(filename));
}

void SpriteFreeData(SpriteData *data)
{
	u16 i;
	data->num_refs--;
	if(data->num_refs == 0) {
		for(i=0; i<data->num_images; i++) {
			free(data->images[i].s2d_sprites);
			free(data->images[i].s2d_gbi);
		}
		free(data);
	}
}

static void UpdateSpritePosition(SpriteInfo *sprite)
{
	sprite->matrix.m.X = sprite->x*4;
	sprite->matrix.m.Y = sprite->y*4;
	osWritebackDCache(&sprite->matrix, sizeof(uObjMtx));
}

static void UpdateSpriteMatrix(SpriteInfo *sprite)
{
	if(sprite->angle != 0) {
		s32 c = (s32)coss(sprite->angle)*2;
		s32 s = (s32)sins(sprite->angle)*2;
		if (c == 0xFFFE) c = 0x10000;
		if (s == 0xFFFE) s = 0x10000;
		if (c == -0xFFFE) c = -0x10000;
		if (s == -0xFFFE) s = -0x10000;
		sprite->matrix.m.A = c*sprite->x_scale;
		sprite->matrix.m.D = c*sprite->y_scale;
		sprite->matrix.m.B = -s*sprite->y_scale;
		sprite->matrix.m.C = s*sprite->x_scale;
	} else {
		sprite->matrix.m.A = sprite->x_scale*65536;
		sprite->matrix.m.D = sprite->y_scale*65536;
		sprite->matrix.m.B = sprite->matrix.m.C = 0;
	}
	sprite->matrix.m.X = sprite->x*4;
	sprite->matrix.m.Y = sprite->y*4;
	osWritebackDCache(&sprite->matrix, sizeof(uObjMtx));
}

SpriteInfo *SpriteCreate(SpriteData *data)
{
	SpriteInfo *sprite = malloc(sizeof(SpriteInfo));
	sprite->data = data;
	sprite->curr_anim = NULL;
	sprite->curr_image = NULL;
	sprite->attr = SPRITE_ATTR_NONE;
	sprite->x = sprite->y = 0.0f;
	sprite->x_scale = sprite->y_scale = 1.0f;
	sprite->angle = 0;
	UpdateSpriteMatrix(sprite);
	sprite->anim_time = 0.0f;
	sprite->anim_speed = 1.0f;
	sprite->anim_frame = 0;
	sprite->tint_r = sprite->tint_g = sprite->tint_b = sprite->tint_a = 255;
	return sprite;
}

void SpriteDestroy(SpriteInfo *sprite)
{
	SpriteFreeData(sprite->data);
	free(sprite);
}

void SpriteSetAttr(SpriteInfo *sprite, unsigned int mask)
{
	sprite->attr |= mask;
}

void SpriteResetAttr(SpriteInfo *sprite, unsigned int mask)
{
	sprite->attr &= ~mask;
}

void SpriteSetPos(SpriteInfo *sprite, float x, float y)
{
	sprite->x = x;
	sprite->y = y;
	UpdateSpritePosition(sprite);
}

void SpriteSetScale(SpriteInfo *sprite, float x, float y)
{
	sprite->x_scale = x;
	sprite->y_scale = y;
	UpdateSpriteMatrix(sprite);
}

void SpriteSetAngle(SpriteInfo *sprite, u16 angle)
{
	sprite->angle = angle;
	UpdateSpriteMatrix(sprite);
}

void SpriteSetAngleDeg(SpriteInfo *sprite, float angle)
{
	sprite->angle = (angle/360.0f)*65536;
	UpdateSpriteMatrix(sprite);
}

void SpriteSetAnimSpeed(SpriteInfo *sprite, float speed)
{
	sprite->anim_speed = speed;
}

static SpriteAnim *FindAnimationHash(SpriteData *data, u32 name_hash)
{
	int i;
	for(i=0; i<data->num_anims; i++) {
		if(data->anims[i].name_hash == name_hash) {
			return &data->anims[i];
		}
	}
	return NULL;
}

static SpriteImage *FindImageHash(SpriteData *data, u32 id_hash)
{
	int i;
	for(i=0; i<data->num_images; i++) {
		if(data->images[i].id_hash == id_hash) {
			return &data->images[i];
		}
	}
	return NULL;
}

void SpriteSetAnim(SpriteInfo *sprite, const char *id)
{
	if(id) {
		sprite->curr_anim = FindAnimationHash(sprite->data, HashGet(id));
		if(sprite->curr_anim) {
			sprite->anim_frame = 0;
			sprite->curr_image = &sprite->data->images[sprite->curr_anim->frames[sprite->anim_frame].image];
			sprite->anim_time = 0;
		} else {
			sprite->curr_image = NULL;
		}
	}
}

void SpriteSetAnimIndex(SpriteInfo *sprite, int index)
{
	sprite->curr_anim = &sprite->data->anims[index];
	if(sprite->curr_anim) {
		sprite->anim_frame = 0;
		sprite->curr_image = &sprite->data->images[sprite->curr_anim->frames[sprite->anim_frame].image];
		sprite->anim_time = 0;
	} else {
		sprite->curr_image = NULL;
	}
}

void SpriteSetImage(SpriteInfo *sprite, const char *id)
{
	if(id) {
		sprite->curr_image = FindImageHash(sprite->data, HashGet(id));
		sprite->curr_anim = NULL;
	}
}

void SpriteSetImageIndex(SpriteInfo *sprite, int id)
{
	sprite->curr_image = &sprite->data->images[id];
	sprite->curr_anim = NULL;
}

void SpriteSetTint(SpriteInfo *sprite, u8 r, u8 g, u8 b, u8 a)
{
	sprite->tint_r = r;
	sprite->tint_g = g;
	sprite->tint_b = b;
	sprite->tint_a = a;
}

void SpriteUpdateAnim(SpriteInfo *sprite)
{
	
}

void SpriteDraw(SpriteInfo *sprite)
{
	if(sprite->curr_image) {
		gDPSetCombineMode(gbi_ptr++, G_CC_DECALRGBA, G_CC_DECALRGBA);
		gDPSetRenderMode(gbi_ptr++, G_RM_XLU_SURF, G_RM_XLU_SURF);
		gSPObjMatrix(gbi_ptr++, &sprite->matrix);
		gSPDisplayList(gbi_ptr++, sprite->curr_image->s2d_gbi);
		if(sprite->curr_anim) {
			
		}
	}
}