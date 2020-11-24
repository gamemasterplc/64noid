#include <ultra64.h>
#include <malloc.h>
#include "sprite.h"
#include "file.h"
#include "hash.h"
#include "render.h"

//Convert Pointer to Memory Space Including NULL
#define PATCH_PTR(ptr, base, type) ((ptr) = ((ptr) ? (type *)((u32)(base)+(u32)(ptr)) : NULL))

//Sprite Display List Construction Data Tables
static int slice_word_count[SPRITE_IMG_FORMAT_MAX] = { 512, 512, 512, 512, 512, 256, 256, 512, 512 };
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

static void PatchSpriteAnim(SpriteAnim *anim_base, u16 num_anims)
{
	u16 i;
	//Patch all of the Frame Pointers in the List of Animations
	for(i=0; i<num_anims; i++) {
		PATCH_PTR(anim_base[i].frames, anim_base, SpriteAnimFrame);
	}
}

static void InitS2DSprite(SpriteImage *image, u16 spr_index, u16 y, u16 h)
{
	uObjSprite_t *s2d_sprite = &image->s2d_sprites[spr_index].s;
	s2d_sprite->objX = -image->origin_x*4;
	s2d_sprite->objY = y*4;
	s2d_sprite->scaleW = s2d_sprite->scaleH = 1024;
	s2d_sprite->imageW = image->w << 5;
	s2d_sprite->imageH = h << 5;
	s2d_sprite->paddingX = s2d_sprite->paddingY = 0;
	if(fmt_tile_bytes[image->format] != 0) {
		//Proper Texture Stride Calculation
		s2d_sprite->imageStride = ((image->w*fmt_tile_bytes[image->format])+7)>>3;
	} else {
		//Proper 4-bit Texture Stride Calculation
		s2d_sprite->imageStride = (((image->w)>>1)+7)>>3;
	}
	s2d_sprite->imageAdrs = 0;
	s2d_sprite->imageFmt = image_fmt_tbl[image->format];
	s2d_sprite->imageSiz = image_size_tbl[image->format];
	s2d_sprite->imagePal = 0;
	s2d_sprite->imageFlags = 0;
}

static int GetSliceHeight(SpriteImage *image)
{
	int stride;
	if(fmt_tile_bytes[image->format] != 0) {
		//Proper Texture Stride Calculation
		if(image->format == SPRITE_IMG_FORMAT_RGBA32) {
			stride = (((image->w)*fmt_tile_bytes[image->format])+7)>>2;
		} else {
			stride = (((image->w)*fmt_tile_bytes[image->format])+7)>>3;
		}
	} else {
		//Proper 4-bit Texture Stride Calculation
		stride = (((image->w)>>1)+7)>>3;
	}
	return slice_word_count[image->format]/stride;
}

static void InitS2DSprites(SpriteImage *image)
{
	u16 slice_h = GetSliceHeight(image);
	u16 slice_cnt = image->h/slice_h;
	u16 total_slice_cnt = slice_cnt;
	u16 remainder_h = image->h%slice_h;
	s16 slice_y = -image->origin_y;
	u16 i;
	if(remainder_h != 0) {
		//Allocate Additional Slice for Partial Slice
		total_slice_cnt = slice_cnt+1;
	}
	image->s2d_sprites = malloc(total_slice_cnt*sizeof(uObjSprite));
	for(i=0; i<slice_cnt; i++) {
		InitS2DSprite(image, i, slice_y, slice_h);
		slice_y += slice_h; //Move by Slice Height in S2D Sprite Coordinates
	}
	if(remainder_h != 0) {
		//Initialize Sprite for Partial Slice
		InitS2DSprite(image, i, slice_y, remainder_h);
	}
	//Flush S2D Sprites to Make RSP See It
	osWritebackDCache(image->s2d_sprites, total_slice_cnt*sizeof(uObjSprite));
}

static u32 GetS2DDispListSize(SpriteImage *image)
{
	u16 slice_h = GetSliceHeight(image);
	u16 slice_cnt = image->h/slice_h;
	u16 total_slice_cnt = slice_cnt;
	u16 remainder_h = image->h%slice_h;
	u32 num_cmds = 0;
	if(remainder_h != 0) {
		//Allocate Additional Slice for Partial Slice
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
	//Load Texture with Sensible Properties
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
	//Initialize Image Palette
	gDPSetTextureLUT(gbi++, pal_formats[image->pal_format]);
	LoadImagePalette(&gbi, image);
	for(i=0; i<slice_cnt; i++) {
		//Draw a Sprite Slice
		LoadImage(&gbi, image, 0, y, image->w-1, y+slice_h-1);
		gSPObjSprite(gbi++, &image->s2d_sprites[i]);
		gDPPipeSync(gbi++);
		y += slice_h;
	}
	if(remainder_h) {
		//Draw Remainder Sprite Slice
		LoadImage(&gbi, image, 0, y, image->w-1, y+remainder_h-1);
		gSPObjSprite(gbi++, &image->s2d_sprites[i]);
		gDPPipeSync(gbi++);
	}
	gSPEndDisplayList(gbi++);
	//Flush DCache for Display List
	osWritebackDCache(gbi_base, disp_size);
	image->s2d_gbi = gbi_base;
}

static void LoadSpriteImage(SpriteImage *image_base, u16 num_images)
{
	u16 i;
	for(i=0; i<num_images; i++) {
		PATCH_PTR(image_base[i].data, image_base, void);
		PATCH_PTR(image_base[i].pal_data, image_base, void);
		InitS2DSprites(&image_base[i]);
		MakeS2DDispList(&image_base[i]);
	}
}

SpriteData *SpriteLoadMemory(void *ptr)
{
	SpriteData *data = (SpriteData *)ptr;
	//Check for Loaded Sprite
	if(data->magic != 'SL') {
		//Load Sprite
		PATCH_PTR(data->anims, data, SpriteAnim);
		PATCH_PTR(data->images, data, SpriteImage);
		if(data->anims) {
			PatchSpriteAnim(data->anims, data->num_anims);
		}
		LoadSpriteImage(data->images, data->num_images);
		data->magic = 'SL'; //Magic Number to Detect Loaded Sprite
		data->num_refs = 1; //Reset Reference Count
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
	data->num_refs--; //Decrement Reference Counter
	if(data->num_refs == 0) {
		//Free Sprite Runtime Data
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
		//Generate Rotation Matrix
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
		//Generate Scaling Matrix
		sprite->matrix.m.A = sprite->x_scale*65536;
		sprite->matrix.m.D = sprite->y_scale*65536;
		sprite->matrix.m.B = sprite->matrix.m.C = 0;
	}
	UpdateSpritePosition(sprite);
}

void SpriteInit(SpriteInfo *sprite, SpriteData *data)
{
	//Initialize Sprite to a Sensible State
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
}

SpriteInfo *SpriteCreate(SpriteData *data)
{
	SpriteInfo *sprite = malloc(sizeof(SpriteInfo));
	SpriteInit(sprite, data);
	return sprite;
}

void SpriteDelete(SpriteInfo *sprite)
{
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
	sprite->angle = (angle/360.0f)*65536; //Convert Angle to N64 Units
	UpdateSpriteMatrix(sprite);
}

void SpriteSetAnimSpeed(SpriteInfo *sprite, float speed)
{
	sprite->anim_speed = speed;
}

int SpriteGetImageIndex(SpriteData *data, const char *id)
{
	int i;
	u32 hash = HashGet(id);
	for(i=0; i<data->num_images; i++) {
		if(data->images[i].id_hash == hash) {
			return i;
		}
	}
	//Return Dummy Value for Image Index
	return -1;
}

int SpriteGetAnimIndex(SpriteData *data, const char *id)
{
	int i;
	u32 hash = HashGet(id);
	for(i=0; i<data->num_images; i++) {
		if(data->anims[i].id_hash == hash) {
			return i;
		}
	}
	//Return Dummy Value for Image Index
	return -1;
}

void SpriteSetAnim(SpriteInfo *sprite, const char *id)
{
	//Don't Attempt to Set Animation for NULL ID
	if(id) {
		SpriteSetAnimIndex(sprite, SpriteGetAnimIndex(sprite->data, id));
	}
}

void SpriteSetAnimIndex(SpriteInfo *sprite, int index)
{
	if(index >= 0 && index < sprite->data->num_anims) {
		sprite->curr_anim = &sprite->data->anims[index];
		if(sprite->attr & SPRITE_ATTR_BACKWARDS_ANIM) {
			//Go to Last Animation Frame
			sprite->anim_frame = sprite->curr_anim->num_frames;
		} else {
			//Go to First Animation Frame
			sprite->anim_frame = 0;
		}
		//Initialize Animation
		sprite->curr_image = &sprite->data->images[sprite->curr_anim->frames[sprite->anim_frame].image_idx];
		sprite->anim_time = 0;
	} else {
		//Set to an Unanimated, Invisible Image In Case of OOB Access
		sprite->curr_anim = NULL;
		sprite->curr_image = NULL;
	}
}

void SpriteSetImage(SpriteInfo *sprite, const char *id)
{
	//Don't Attempt to Set Image for NULL ID
	if(id) {
		SpriteSetImageIndex(sprite, SpriteGetImageIndex(sprite->data, id));
	}
}

void SpriteSetImageIndex(SpriteInfo *sprite, int index)
{
	//Protect Against OOB Array Indices
	if(index >= 0 && index < sprite->data->num_images) {
		sprite->curr_image = &sprite->data->images[index];
		sprite->curr_anim = NULL; //Disable Animation Playback for Set Image
	} else {
		//Set to an Unanimated, Invisible Image In Case of OOB Access
		sprite->curr_image = NULL;
		sprite->curr_anim = NULL;
	}
}

void SpriteSetTint(SpriteInfo *sprite, u8 r, u8 g, u8 b, u8 a)
{
	sprite->tint_r = r;
	sprite->tint_g = g;
	sprite->tint_b = b;
	sprite->tint_a = a;
}

static void UpdateSpriteAnim(SpriteInfo *sprite)
{
	sprite->anim_time += sprite->anim_speed;
	//Check for Advance to Next Frame
	if(sprite->anim_time >= sprite->curr_anim->frames[sprite->anim_frame].delay) {
		if(sprite->attr & SPRITE_ATTR_BACKWARDS_ANIM) {
			//Advance Animation Frames Backwards
			sprite->anim_frame--;
			if(sprite->anim_frame < 0) {
				//Go to End of Animation
				sprite->anim_frame = sprite->curr_anim->num_frames-1;
			}
		} else {
			//Advance Animation Frames
			sprite->anim_frame++;
			if(sprite->anim_frame >= sprite->curr_anim->num_frames) {
				//Restart Animation
				sprite->anim_frame = 0;
			}
		}
		//Advance to Next Image
		sprite->curr_image = &sprite->data->images[sprite->curr_anim->frames[sprite->anim_frame].image_idx];
		sprite->anim_time = 0;
	}
}

void SpriteDraw(SpriteInfo *sprite)
{
	//Only Draw Sprites with an Image Set
	if(sprite->curr_image) {
		//Draw Sprites if Visible
		if(!(sprite->attr & SPRITE_ATTR_INVISIBLE)) {
			//Setup Render State for Sprite Drawing
			if(sprite->attr & SPRITE_ATTR_ENABLE_TINT) {
				if(render_mode != RENDER_MODE_SPRITE_TINT) {
					gDPSetCombineMode(render_dl_ptr++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
					gDPSetRenderMode(render_dl_ptr++, G_RM_XLU_SURF, G_RM_XLU_SURF);
					render_mode = RENDER_MODE_SPRITE_TINT;
				}
				gDPSetPrimColor(render_dl_ptr++, 0, 0, sprite->tint_r, sprite->tint_g, sprite->tint_b, sprite->tint_a);
			} else {
				if(render_mode != RENDER_MODE_SPRITE) {
					gDPSetCombineMode(render_dl_ptr++, G_CC_DECALRGBA, G_CC_DECALRGBA);
					gDPSetRenderMode(render_dl_ptr++, G_RM_XLU_SURF, G_RM_XLU_SURF);
					render_mode = RENDER_MODE_SPRITE;
				}
			}
			//Draw the Sprite with a Unique Matrix
			gSPObjMatrix(render_dl_ptr++, &sprite->matrix);
			gSPDisplayList(render_dl_ptr++, sprite->curr_image->s2d_gbi);
		}
		//Tick Animation if Animation is Playing
		if(sprite->curr_anim) {
			UpdateSpriteAnim(sprite);
		}
	}
}