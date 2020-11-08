#ifndef PAD_H
#define PAD_H

#include <nusys.h>
#include "bool.h"

extern NUContData pad_data[NU_CONT_MAXCONTROLLERS];

void PadInit();
void PadRead();
bool PadIsConnected(int player);

#endif