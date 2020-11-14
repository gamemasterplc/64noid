#ifndef TEXT_H
#define TEXT_H

#include <PR/ultratypes.h>

#define TEXT_ALIGNMENT_LEFT 0
#define TEXT_ALIGNMENT_CENTER 1
#define TEXT_ALIGNMENT_RIGHT 2

void TextDraw(int x, int y, int alignment, char *str);
void TextSetColor(u8 r, u8 g, u8 b, u8 a);

#endif