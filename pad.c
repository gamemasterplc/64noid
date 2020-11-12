#include "pad.h"

NUContData pad_data[NU_CONT_MAXCONTROLLERS];
static u8 pad_mask;

void PadInit()
{
	pad_mask = nuContInit();
}

void PadRead()
{
	nuContDataGetExAll(pad_data);
}

bool PadIsConnected(int player)
{
	if((pad_mask & (1 << player)) && pad_data[player].errno == 0) { //Check for Connected Pad
		return true;
	} else {
		return false;
	}
}
