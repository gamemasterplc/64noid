#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <nusys.h>
#include "bool.h"

extern NUContData cont_data[NU_CONT_MAXCONTROLLERS];
extern u8 cont_pattern;

void ControllerInit();
void ControllerRead();
bool ControllerIsConnected(int player);

#endif