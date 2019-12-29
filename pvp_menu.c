#include <z64ovl/oot/u10.h>
#include <z64ovl/z64ovl_helpers.h>
#include "z64_inputHandler.h"
#include "menu.h"

#define GLOBAL_SCENE_FRAME 0x802120BC
#define ACT_ID 0x0082


#define G_IM_FMT_RGBA                 0
#define G_IM_FMT_YUV                  1
#define G_IM_FMT_CI                   2
#define G_IM_FMT_IA                   3
#define G_IM_FMT_I                    4
#define G_IM_SIZ_4b                   0
#define G_IM_SIZ_8b                   1
#define G_IM_SIZ_16b                  2
#define G_IM_SIZ_32b                  3

#define FRAMETIME 0.05f


typedef struct {
	z64_actor_t actor;
	z64_inputHandler_t inputHandler;
	float currentTime;
	uint32_t currentFrame;
	
	menu_t menu;
	uint32_t debug;
	uint32_t debug2;
	uint32_t end;
	uint32_t end2;
	float* LinkPos;
} entity_t;


static void init(entity_t *en, z64_global_t *gl) 
{
	en->currentTime = 0;
	en->end = 0xDEADBEEF;
	en->end2 = 0xDEADBEEF;
	
	#if OOT_DEBUG
	en->LinkPos = 0x802245D4; // Debug
	#elif OOT_U_1_0
	en->LinkPos = 0x801DAA54; //1.0
	#endif
	
	construct_z64_inputHandler_t(&en->inputHandler, &gl->common.input[0].raw);
	construct_menu_t(&en->menu);
	
}

static void dest(entity_t *en, z64_global_t *gl) {}

static void play(entity_t *en, z64_global_t *gl) 
{

	update_z64_inputHandler_t(&en->inputHandler, en->currentTime);
	en->actor.pos_2.x = en->LinkPos[0];
	en->actor.pos_2.y = en->LinkPos[1];
	en->actor.pos_2.z = en->LinkPos[2];
}

static void draw(entity_t *en, z64_global_t *gl)
{
	en->currentTime += FRAMETIME; //TODO: Get from game
	en->currentFrame++;

	update_menu_t(&en->menu, &en->inputHandler, gl, en->currentTime, &en->debug, &en->debug2);
}


const z64_actor_init_t init_vars = {
	.number = ACT_ID,
	.type = 0x4,
	.room = 0xFF,
	.flags = 0x00000010,
	.object = 0x01,
  .padding = 0x0000,
	.instance_size = sizeof(entity_t),
	.init = init,
	.dest = dest,
	.main = play,
	.draw = draw
};



