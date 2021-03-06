#include <PR/ultratypes.h>
#include "render.h"
#include "text.h"

static u8 font_texture[] __attribute__((aligned(8))) = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0xF0, 0x0F, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0xF0,
    0x00, 0xF0, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0x0F, 0x0F, 0x00, 0x0F, 0xFF, 0xF0, 0x00,
    0x00, 0xF0, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0xF0,
    0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0,
    0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0x00, 0x00,
    0x0F, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0,
    0xF0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x0F, 0xFF, 0x00, 0x00,
    0xF0, 0x00, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x0F, 0xFF,
    0x00, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xF0, 0x00, 0xF0, 0xF0, 0x0F,
    0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x0F, 0xFF,
    0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0xFF, 0xFF, 0x00, 0xF0,
    0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0xF0,
    0xF0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0xF0,
    0x00, 0xF0, 0x0F, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xF0, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xF0, 0x0F, 0x00, 0xFF,
    0xFF, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xFF, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0xF0, 0x00,
    0xF0, 0x0F, 0x00, 0xFF, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0,
    0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x00,
    0xF0, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0,
    0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0xFF, 0xFF, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00,
    0xF0, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xFF, 0xFF, 0x00, 0xFF,
    0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0x00,
    0xF0, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x0F, 0x00, 0xF0, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0xFF, 0xFF, 0xF0, 0x00, 0xF0, 0x00, 0x0F, 0x00, 0x00,
    0xF0, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0xF0,
    0x0F, 0x00, 0xF0, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xF0, 0x00, 0x0F, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x0F,
    0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xF0, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF,
    0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x0F, 0xFF, 0x00,
    0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0,
    0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0xFF, 0x0F, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0,
    0xF0, 0xFF, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0,
    0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xFF, 0x00, 0xF0, 0xF0, 0x00, 0xF0,
    0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xFF,
    0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xF0, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0xFF, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0xF0,
    0xF0, 0xFF, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0,
    0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0xF0, 0x0F,
    0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x0F, 0xF0, 0xF0, 0x00, 0xF0,
    0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0,
    0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0xF0, 0x0F,
    0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0,
    0x0F, 0xFF, 0x00, 0xF0, 0x00, 0xF0, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF,
    0xFF, 0xF0, 0xF0, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0x0F, 0xF0,
    0x00, 0xF0, 0x00, 0xF0, 0xFF, 0xFF, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x0F, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0xFF, 0xFF, 0xF0, 0xF0,
    0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xFF, 0xFF,
    0xF0, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xF0,
    0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0x00,
    0xF0, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0,
    0x00, 0xF0, 0x0F, 0x0F, 0x00, 0xF0, 0x00, 0xF0, 0x0F, 0x0F, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x0F,
    0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0x00, 0xF0, 0x00, 0xF0, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0xF0, 0x00, 0xF0,
    0x00, 0xF0, 0x0F, 0x0F, 0x00, 0xF0, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0,
    0x00, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xF0,
    0x00, 0xF0, 0x0F, 0x0F, 0x00, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0x0F, 0x00,
    0x00, 0xF0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xF0,
    0x00, 0xF0, 0x00, 0xF0, 0x00, 0x0F, 0x0F, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xF0, 0x00,
    0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0x00, 0x00, 0x0F, 0xF0, 0xF0, 0xF0, 0x00, 0xF0, 0x0F, 0xFF, 0x00, 0x00, 0xF0, 0x00, 0x0F,
    0xFF, 0x00, 0x00, 0xF0, 0x00, 0x0F, 0x0F, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xFF, 0xFF,
    0xF0, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xFF, 0x00, 0x0F,
    0xF0, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0,
    0x0F, 0x00, 0xFF, 0xF0, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00,
    0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0xFF,
    0xFF, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0xFF, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0,
    0x00, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00,
    0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xFF, 0x00, 0x0F,
    0xFF, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x0F,
    0x00, 0xF0, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0xF0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0xF0, 0x00, 0x0F, 0x0F, 0x00,
    0xFF, 0xF0, 0x00, 0x0F, 0xFF, 0x00, 0xF0, 0xF0, 0x00, 0x0F, 0xFF, 0x00, 0xFF, 0xF0, 0x00, 0xF0,
    0x0F, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0x00, 0xF0, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0xFF, 0xF0,
    0x00, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0,
    0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xFF, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0xF0,
    0x0F, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0x00, 0x00, 0xF0,
    0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0,
    0xF0, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0x00, 0x00, 0xF0,
    0x0F, 0x00, 0xF0, 0xF0, 0x00, 0xF0, 0xF0, 0xF0, 0x0F, 0x00, 0x00, 0xF0, 0xF0, 0x00, 0x0F, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0,
    0xFF, 0xF0, 0x00, 0x0F, 0xFF, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0xF0,
    0x0F, 0x00, 0xF0, 0xF0, 0x00, 0x0F, 0x0F, 0x00, 0xF0, 0xF0, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0,
    0xF0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0xF0, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x0F,
    0xFF, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0xF0, 0xF0, 0x00, 0x0F, 0x00, 0x00, 0xFF, 0xF0,
    0x00, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xF0,
    0xF0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static char char_widths[] = {
	3, 2, 6, 6, 6, 6, 6, 3, 4, 4, 6, 6, 3, 6, 2, 6,
	5, 3, 5, 5, 5, 5, 5, 5, 5, 5, 2, 3, 5, 6, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 5, 6, 5, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6, 3, 6, 6,
	3, 5, 5, 5, 5, 5, 4, 5, 5, 2, 5, 5, 3, 6, 5, 5,
	5, 5, 4, 5, 4, 5, 4, 6, 4, 4, 4, 4, 2, 4, 5, 6
};

static u8 text_r, text_g, text_b, text_a;

void TextSetColor(u8 r, u8 g, u8 b, u8 a)
{
	text_r = r;
	text_g = g;
	text_b = b;
	text_a = a;
}

static int GetTextWidth(char *str)
{
	int line_w = 0;
	int w = 0;
	while(*str) {
		char c = *str;
		if(c == '\n') {
			if(line_w > w) {
				w = line_w;
			}
			line_w = 0;
		} else if(c >= 32 && c < 128){
			char glyph = c-32;
			line_w += char_widths[glyph];
		} else {
			line_w += char_widths[0];
		}
		str++;
	}
	if(line_w > w) {
		w = line_w;
	}
	if(w > 0) {
		return w-1;
	} else {
		return 0;
	}
	
}

void TextDraw(int x, int y, int alignment, char *str)
{
	int orig_x;
	//Initialize Text Rendering
	if(render_mode != RENDER_MODE_TEXT) {
		render_mode = RENDER_MODE_TEXT;
		gDPSetCombineMode(render_dl_ptr++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
		gDPSetRenderMode(render_dl_ptr++, G_RM_XLU_SURF, G_RM_XLU_SURF);
		//Load Font Texture
		gDPSetTextureLUT(render_dl_ptr++, G_TT_NONE);
		gDPLoadTextureTile_4b(render_dl_ptr++, font_texture, G_IM_FMT_I, 96, 54, 0, 0, 95, 53, 0, G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
	}
	//Set Text Color
	gDPSetPrimColor(render_dl_ptr++, 0, 0, text_r, text_g, text_b, text_a);
	//Offset Text for Alignment
	switch(alignment) {
		case TEXT_ALIGNMENT_LEFT:
			break;
			
		case TEXT_ALIGNMENT_CENTER:
			x -= GetTextWidth(str)/2;
			break;
			
		case TEXT_ALIGNMENT_RIGHT:
			x -= GetTextWidth(str);
			break;
	}
	orig_x = x;
	while(*str) {
		char c = *str;
		if(c == '\n') {
			//Advance to Next Line
			y += 9;
			x = orig_x;
		} else if(c > 32 && c < 128){
			//Draw Non-Space Character
			char glyph = c-32;
			gSPScisTextureRectangle(render_dl_ptr++, x*4, y*4, (x+6)*4, (y+9)*4, 0, (glyph%16)*192, (glyph/16)*288, 1024, 1024);
			gDPPipeSync(render_dl_ptr++);
			x += char_widths[glyph];
		} else {
			x += char_widths[0];
		}
		str++;
	}
}
