#ifndef SAVE_H
#define SAVE_H

#include <PR/ultratypes.h>
#include "map.h"
#include "bool.h"

#define SAVE_SIZE 512
#define MAX_EDITOR_MAPS 3

typedef struct save_data {
	int num_lives;
	int map_num;
	int score;
	int high_score;
	bool enable_continue;
	char edited_maps[MAX_EDITOR_MAPS][MAP_WIDTH*MAP_HEIGHT];
	//The Following Member Must be Last
	u16 checksum;
} SaveData;

typedef struct save_buffer {
	u16 magic; //Must be Equal to SA to Load
	SaveData data;
	u8 pad[SAVE_SIZE-sizeof(u16)-sizeof(SaveData)];
} SaveBuffer;

extern SaveData *save_data;

#endif