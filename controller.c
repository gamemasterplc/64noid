#include "controller.h"

NUContData cont_data[NU_CONT_MAXCONTROLLERS];
u8 cont_pattern;

void ControllerInit()
{
	cont_pattern = nuContInit();
}

void ControllerRead()
{
	nuContDataGetExAll(cont_data);
}

bool ControllerIsConnected(int player)
{
	if((cont_pattern & (1 << player)) && cont_data[player].errno == 0) {
		return true;
	} else {
		return false;
	}
}
