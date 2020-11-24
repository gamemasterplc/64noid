#ifndef MAIN_H
#define MAIN_H

typedef void (*StageFunc)();

typedef struct stage_entry {
	StageFunc init_func;
	StageFunc update_func;
	StageFunc draw_func;
	StageFunc destroy_func; //NULL Means it Doesn't Exist
} StageEntry;

typedef enum stage_id {
	STAGE_MAPSELECT = 0,
	STAGE_NEXTMAP,
	STAGE_GAME,
	STAGE_END,
	STAGE_TITLE,
	STAGE_MAPEDITOR,
	STAGE_MAX,
	STAGE_NONE = -1
} StageID;

void SetNextStage(StageID stage);

#endif