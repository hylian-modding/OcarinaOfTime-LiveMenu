#ifndef MENU_H
#define MENU_H

#include <z64ovl/oot/u10.h>
#include <z64ovl/z64ovl_helpers.h>
#include "textures.h"
#include "z64_inputHandler.h"
#include "mathUtils.h"


#define SW_NONE 0b00000000 
#define SW_KOKIRI 0b00000001  /* 1 */
#define SW_MASTER 0b00000010  /* 2 */
#define SW_GIANT 0b00000100   /* 4 */

#define SH_NONE 0b00000000 
#define SH_DEKU 0b00000001  /* 1 */
#define SH_HYLIAN 0b00000010  /* 2 */
#define SH_MIRROR 0b00000100   /* 4 */

#define T_NONE 0b00000000 
#define T_KOKIRI 0b00000001  /* 1 */
#define T_GORON 0b00000010  /* 2 */
#define T_ZORA 0b00000100   /* 4 */

#define B_NONE 0b00000000 
#define B_KOKIRI 0b00000001  /* 1 */
#define B_GORON 0b00000010  /* 2 */
#define B_ZORA 0b00000100   /* 4 */


#define CLEFT 0
#define CDOWN 1
#define CRIGHT 2

#define CATEGORY_PROJECTILE 0
#define CATEGORY_WEAPON 1
#define CATEGORY_ARMOR 2
#define CATEGORY_HAND 3
#define CATEGORY_MAGIC 4
#define CATEGORY_BOTTLE 5
#define NUM_CATEGORIES 6
#define NUM_ITEMS 48

#define PROJ_BOW 0
#define PROJ_FIRE 1
#define PROJ_ICE 2
#define PROJ_LIGHT 3
#define PROJ_HOOK 4
#define PROJ_LONG 5
#define PROJ_SLING 6
#define PROJ_BOOMER 7

// Make defines for weapons/armor, see above/below

#define MAG_NAY 0
#define MAG_DIN 1
#define MAG_FAR 2

#define HAND_HAM 0
#define HAND_BOM 1
#define HAND_BOC 2
#define HAND_STI 3
#define HAND_NUT 4

#define BOTTLE_0 0
#define BOTTLE_1 1
#define BOTTLE_2 2
#define BOTTLE_3 3

#define WEAPON_KOKIRI 0
#define WEAPON_MASTER 1
#define WEAPON_BIGGORON 2
#define WEAPON_DEKU 3
#define WEAPON_HYLIAN 4
#define WEAPON_MIRROR 5

#define ARMOR_KOKIRI 0 
#define ARMOR_GORON 1
#define ARMOR_ZORA 2
#define ARMOR_KBOOTS 3
#define ARMOR_IBOOTS 4
#define ARMOR_HBOOTS 5


#define baseMenuPositionX 76.f
#define baseMenuPositionY 108.f
#define baseMenuSizeOffset 19.f
#define offscreenMenuPositionX -72.f
#define offscreenMenuOffsetY 14.f
#define noSelectOffsetX -4
#define categoryWidth 115

#define Inventory_Context 0x8011A644
#define Equipment_Context 0x8011A66C

//Save context + 74 (Inventory) 

///
/// GUI OBJECT
///

typedef struct {
	gfx_screen_tile_t tile;
	gfx_texture_t texture;
	float dampening;
	float lastUpdate;
	float targetX;
	float targetY;
	float velocityX;
	float velocityY;
} guiObject_t; // Single gui object with built in interpolator

///
/// MENU ITEM
///

typedef struct {
	guiObject_t item;
	uint8_t isShown;
	uint8_t* has;
	float offsetPositionX;
	float offsetPositionY;
} menuItem_t; // Selectable menu item; wrapper for other data

///
/// MENU CATEGORY
///

typedef struct {
	guiObject_t categoryBackground;
	menuItem_t* items;
	uint8_t length; // Number of items in category
	uint8_t id;
	interpolator_int_t alpha;
} menuCategory_t; // Category class


///
/// MENU META
///

typedef struct {
	int above;
	int below;
} menuMeta_t;

///
/// MENU
///

typedef struct {
	uint8_t doesExist;
	uint8_t demandImmediateUpdate;
	uint8_t menuOpen;
	uint8_t dPadShow;
	uint8_t cButton;
	int index;
	int category;
    int categoryLength[6];
	float lastTime;
	float lastScrollTime;
	float scrollTimeMin;
	float scrollTimeMax;
	float scrollTimeDecay;
	float currentScrollTime;
	float dampMax;
	float dampMin;
	float dampDecay;
	float currentDamp;

	guiObject_t smoothSelectionBox;
    guiObject_t selectionBox;
    interpolator_int_t selectionAlpha;
    int alphaDir;

	guiObject_t dPadTop;
	guiObject_t dPadBottom;

	menuItem_t items[NUM_ITEMS];
	menuCategory_t cCategory[6];
	menuMeta_t cMeta[6];
} menu_t; // Wrapper struct for all menu data

static int bit_test(char bit, char byte)
{
    bit = 1 << bit;
    return (bit & byte);
}

// Constructor for gui objects
void construct_guiObject_t(guiObject_t* guiObject) {
	guiObject->dampening = 1;
	guiObject->lastUpdate = 0;
	guiObject->targetX = 160;
	guiObject->targetY = 160;
	guiObject->tile.x = 160;
	guiObject->tile.y = 160;
	guiObject->velocityX = 0;
	guiObject->velocityY = 0;	
}

// Update gui object interpolator
void update_guiObject_t(guiObject_t* guiObject, float currentTime, uint8_t diu) {
	if (diu) {
		guiObject->tile.x = guiObject->targetX;
		guiObject->tile.y = guiObject->targetY;
	}
	else {
		float deltaTime = currentTime - guiObject->lastUpdate;
		float ddt = guiObject->dampening * deltaTime;
		float ddd = guiObject->dampening * ddt;

		float forceX = guiObject->velocityX - (guiObject->tile.x - guiObject->targetX) * ddd;
		float forceY = guiObject->velocityY - (guiObject->tile.y - guiObject->targetY) * ddd;

		float e = 1 + ddt;
		e *= e;

		guiObject->velocityX = forceX / e;
		guiObject->velocityY = forceY / e;

		guiObject->tile.x += guiObject->velocityX * deltaTime;
		guiObject->tile.y += guiObject->velocityY * deltaTime;
	}

	guiObject->lastUpdate = currentTime;
}

// Construct a category with number of menu items. We can't malloc so they much be define
void construct_menuCategory_t(menuCategory_t* category, menuItem_t* items, uint8_t length, uint8_t id) {
	construct_guiObject_t(&category->categoryBackground);
	category->alpha.p = 255;
	category->alpha.t = 255;

	//Long Black Box init
	category->categoryBackground.tile.x = baseMenuPositionX;
	category->categoryBackground.tile.y = baseMenuPositionY;
	category->categoryBackground.targetX = baseMenuPositionX;
	category->categoryBackground.targetY = baseMenuPositionY;
	category->categoryBackground.tile.origin_anchor = G_TX_ANCHOR_C;
	category->categoryBackground.tile.width = categoryWidth;
	category->categoryBackground.tile.height = 64;

	category->categoryBackground.texture.timg = &tLongBlackBox;
	category->categoryBackground.texture.width = 32;
	category->categoryBackground.texture.height = 32;
	category->categoryBackground.texture.fmt = G_IM_FMT_RGBA;
	category->categoryBackground.texture.bitsiz = 3;

	category->length = length;
	category->items = items;

	category->id = id;

	for (int i = 0; i < length; i++) {
		construct_guiObject_t(&category->items[i].item);
		category->items[i].offsetPositionX = -(categoryWidth / 2) + (16 * i);
		category->items[i].offsetPositionY = -25;
	}
}

// Update menu category; automated drawing
void update_menuCategory_t(menuCategory_t* category, z64_global_t* gl, z64_inputHandler_t* input, menu_t* state, float currentTime, float deltaTime) {
	update_guiObject_t(&category->categoryBackground, currentTime, state->demandImmediateUpdate);

	interpolateInt(deltaTime, 3, &category->alpha.v, &category->alpha.p, category->alpha.t);
    zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &category->categoryBackground.texture, &category->categoryBackground.tile, category->alpha.p);

	for (int i = 0; i < category->length; i++) {
		//Update positions relative to parent
		category->items[i].item.targetX = category->items[i].offsetPositionX + category->categoryBackground.targetX;
		category->items[i].item.targetY = category->items[i].offsetPositionY + category->categoryBackground.targetY;
		
			
		uint8_t drawBoxes = state->category == category->id;

		
		update_guiObject_t(&category->items[i].item, currentTime, state->demandImmediateUpdate);
		
		if (category->items[i].isShown) zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &category->items[i].item.texture, &category->items[i].item.tile, category->alpha.p);
	}
}


// Construct menu data
void construct_menu_t(menu_t* state) {
	state->doesExist = 1;
	state->demandImmediateUpdate = 0;
	state->menuOpen = 0;
	state->dPadShow = 1;
	state->category = 0;
	state->cButton = 0;
    state->selectionAlpha.p = 255;
    state->selectionAlpha.t = 255;
    state->selectionAlpha.v = 0;
    state->alphaDir = -11;
    state->index = 0;
	state->lastScrollTime = 0;
	state->scrollTimeMin = 0.1f;
	state->scrollTimeMax = 0.25f;
	state->scrollTimeDecay = 0.05f;
	state->currentScrollTime = state->scrollTimeMax;
	state->dampMax = 108;
	state->dampMin = 9;
	state->dampDecay = 11;
	state->currentDamp = state->dampMin;

	construct_menuCategory_t(&state->cCategory[CATEGORY_PROJECTILE], state->items, 8, CATEGORY_PROJECTILE);
	construct_menuCategory_t(&state->cCategory[CATEGORY_WEAPON], &state->items[8], 6, CATEGORY_WEAPON);
	construct_menuCategory_t(&state->cCategory[CATEGORY_ARMOR], &state->items[14], 6, CATEGORY_ARMOR);
	construct_menuCategory_t(&state->cCategory[CATEGORY_HAND], &state->items[20], 5, CATEGORY_HAND);
	construct_menuCategory_t(&state->cCategory[CATEGORY_MAGIC], &state->items[25], 3, CATEGORY_MAGIC);
	construct_menuCategory_t(&state->cCategory[CATEGORY_BOTTLE], &state->items[28], 4, CATEGORY_BOTTLE);

    state->categoryLength[CATEGORY_PROJECTILE] = 8;
    state->categoryLength[CATEGORY_WEAPON] = 6;
	state->categoryLength[CATEGORY_ARMOR] = 6;
	state->categoryLength[CATEGORY_HAND] = 5;
	state->categoryLength[CATEGORY_MAGIC] = 3;
    state->categoryLength[CATEGORY_BOTTLE] = 4;

	state->cMeta[0].above = CATEGORY_BOTTLE;
	state->cMeta[0].below = CATEGORY_WEAPON;

	state->cMeta[1].above = CATEGORY_PROJECTILE;
	state->cMeta[1].below = CATEGORY_ARMOR;

	state->cMeta[2].above = CATEGORY_WEAPON;
	state->cMeta[2].below = CATEGORY_HAND;

	state->cMeta[3].above = CATEGORY_ARMOR;
	state->cMeta[3].below = CATEGORY_MAGIC;

	state->cMeta[4].above = CATEGORY_HAND;
	state->cMeta[4].below = CATEGORY_BOTTLE;

	state->cMeta[5].above = CATEGORY_MAGIC;
	state->cMeta[5].below = CATEGORY_PROJECTILE;

	
    state->cCategory[CATEGORY_PROJECTILE].items[PROJ_BOW].item.texture.timg = &tBow;
    state->cCategory[CATEGORY_PROJECTILE].items[PROJ_FIRE].item.texture.timg = &tFireArrow;
    state->cCategory[CATEGORY_PROJECTILE].items[PROJ_ICE].item.texture.timg = &tIceArrow;
    state->cCategory[CATEGORY_PROJECTILE].items[PROJ_LIGHT].item.texture.timg = &tLightArrows;
    state->cCategory[CATEGORY_PROJECTILE].items[PROJ_HOOK].item.texture.timg = &tHookshot;
    state->cCategory[CATEGORY_PROJECTILE].items[PROJ_LONG].item.texture.timg = &tLongshot;
    state->cCategory[CATEGORY_PROJECTILE].items[PROJ_SLING].item.texture.timg = &tSlingshot;
    state->cCategory[CATEGORY_PROJECTILE].items[PROJ_BOOMER].item.texture.timg = &tBoomerang;

	//Set the icons for these, annd make defiens. See above/below
	state->cCategory[CATEGORY_WEAPON].items[0].item.texture.timg = &tKokiriSword;
	state->cCategory[CATEGORY_WEAPON].items[1].item.texture.timg = &tMasterSword;
	state->cCategory[CATEGORY_WEAPON].items[2].item.texture.timg = &tBiggoron;
	state->cCategory[CATEGORY_WEAPON].items[3].item.texture.timg = &tDekuShield;
	state->cCategory[CATEGORY_WEAPON].items[4].item.texture.timg = &tHylianShield;
	state->cCategory[CATEGORY_WEAPON].items[5].item.texture.timg = &tMirrorShield;

	state->cCategory[CATEGORY_ARMOR].items[0].item.texture.timg = &tKokiriTunic;
	state->cCategory[CATEGORY_ARMOR].items[1].item.texture.timg = &tGoronTunic;
	state->cCategory[CATEGORY_ARMOR].items[2].item.texture.timg = &tZoraTunic;
	state->cCategory[CATEGORY_ARMOR].items[3].item.texture.timg = &tKokiriBoots;
	state->cCategory[CATEGORY_ARMOR].items[4].item.texture.timg = &tIronBoots;
	state->cCategory[CATEGORY_ARMOR].items[5].item.texture.timg = &tHoverBoots;
    
	state->cCategory[CATEGORY_HAND].items[HAND_HAM].item.texture.timg = &tHammer;
    state->cCategory[CATEGORY_HAND].items[HAND_BOM].item.texture.timg = &tBombs;
    state->cCategory[CATEGORY_HAND].items[HAND_BOC].item.texture.timg = &tBombchu;
    state->cCategory[CATEGORY_HAND].items[HAND_STI].item.texture.timg = &tDekuStick;
    state->cCategory[CATEGORY_HAND].items[HAND_NUT].item.texture.timg = &tDekuNuts;

    state->cCategory[CATEGORY_MAGIC].items[MAG_NAY].item.texture.timg = &tNayru;
    state->cCategory[CATEGORY_MAGIC].items[MAG_DIN].item.texture.timg = &tDin;
    state->cCategory[CATEGORY_MAGIC].items[MAG_FAR].item.texture.timg = &tFarore;

    state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_0].item.texture.timg = &tEmptyBottle;
    state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_1].item.texture.timg = &tEmptyBottle;
    state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_2].item.texture.timg = &tEmptyBottle;
    state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_3].item.texture.timg = &tEmptyBottle;


	construct_guiObject_t(&state->selectionBox);
	state->selectionBox.tile.x = 0;
	state->selectionBox.tile.y = 0;
	state->selectionBox.tile.origin_anchor = G_TX_ANCHOR_C;
	state->selectionBox.tile.width = 19;
	state->selectionBox.tile.height= 19;

	state->selectionBox.texture.timg = &tRedBox;
	state->selectionBox.texture.width = 32;
	state->selectionBox.texture.height = 32;
	state->selectionBox.texture.fmt = G_IM_FMT_RGBA;
	state->selectionBox.texture.bitsiz = 3;
    state->selectionBox.dampening = 999;

	state->cCategory[CATEGORY_PROJECTILE].categoryBackground.dampening = state->dampMin;
	state->cCategory[CATEGORY_WEAPON].categoryBackground.dampening = state->dampMin;
	state->cCategory[CATEGORY_ARMOR].categoryBackground.dampening = state->dampMin;
	state->cCategory[CATEGORY_PROJECTILE].categoryBackground.dampening = state->dampMin;
	state->cCategory[CATEGORY_MAGIC].categoryBackground.dampening = state->dampMin;
	state->cCategory[CATEGORY_HAND].categoryBackground.dampening = state->dampMin;
	state->cCategory[CATEGORY_BOTTLE].categoryBackground.dampening = state->dampMin;


	construct_guiObject_t(&state->dPadTop);
	state->dPadTop.tile.x = 50;
	state->dPadTop.tile.y = 156;
	state->dPadTop.tile.origin_anchor = G_TX_ANCHOR_C;
	state->dPadTop.tile.width = 40;
	state->dPadTop.tile.height= 24;

	state->dPadTop.texture.timg = &tDpad1;
	state->dPadTop.texture.width = 64;
	state->dPadTop.texture.height = 32;
	state->dPadTop.texture.fmt = G_IM_FMT_RGBA;
	state->dPadTop.texture.bitsiz = 2;


	construct_guiObject_t(&state->dPadBottom);
	state->dPadBottom.tile.x = 50;
	state->dPadBottom.tile.y = 180;
	state->dPadBottom.tile.origin_anchor = G_TX_ANCHOR_C;
	state->dPadBottom.tile.width = 40;
	state->dPadBottom.tile.height= 24;

	state->dPadBottom.texture.timg = &tDpad0;
	state->dPadBottom.texture.width = 64;
	state->dPadBottom.texture.height = 32;
	state->dPadBottom.texture.fmt = G_IM_FMT_RGBA;
	state->dPadBottom.texture.bitsiz = 2;

    //Construct BGs and icons
    for (int i = 0; i < NUM_ITEMS; i++) {
        state->items[i].item.dampening = state->dampMin - 1;
        state->items[i].item.tile.x = 0;
        state->items[i].item.tile.y = 0;
        state->items[i].item.tile.origin_anchor = G_TX_ANCHOR_C;
        state->items[i].item.tile.width = 16;
        state->items[i].item.tile.height = 16;

        state->items[i].item.texture.width = 32;
        state->items[i].item.texture.height = 32;
        state->items[i].item.texture.fmt = G_IM_FMT_RGBA;
        state->items[i].item.texture.bitsiz = 2;
    }
}

inline void forceMoveCategory(menuCategory_t* category, float damp) {
	for (int i = 0; i < category->length; i++) {
		category->items[i].item.dampening = damp - 1;
	}
}

inline void forceMove(menu_t* state) {
	state->currentDamp += state->dampDecay;

	for (int i = 0; i < NUM_CATEGORIES; i++) {
		state->cCategory[i].categoryBackground.dampening = state->currentDamp;
		forceMoveCategory(&state->cCategory[i], state->currentDamp);
	}
}

inline void updateCategoryPosition(menuCategory_t* offset, int position) {
	float positionX = baseMenuPositionX;
	float selectedPositionY = baseMenuPositionY;
	float belowPositionY = baseMenuPositionY + baseMenuSizeOffset;
	float abovePositionY = baseMenuPositionY - baseMenuSizeOffset;
	float offscreenPositionX = offscreenMenuPositionX;
	float offscreenAbovePositionY = baseMenuPositionY - baseMenuSizeOffset;
	float offscreenBelowPositionY = baseMenuPositionY + baseMenuSizeOffset;

	if (position == 0) {
		offset->categoryBackground.targetX = positionX;
		offset->categoryBackground.targetY = selectedPositionY;
		offset->alpha.t = 240;
	}
	else offset->alpha.t = 85;

	if (position == 1) {
		offset->categoryBackground.targetX = positionX + noSelectOffsetX;
		offset->categoryBackground.targetY = abovePositionY;
	}

	if (position == -1) {
		offset->categoryBackground.targetX = positionX + noSelectOffsetX;
		offset->categoryBackground.targetY = belowPositionY;
	}

	if (position == 2) {
		offset->categoryBackground.targetX = offscreenPositionX;
		offset->categoryBackground.targetY = offscreenAbovePositionY;
	}

	if (position == -2) {
		offset->categoryBackground.targetX = offscreenPositionX;
		offset->categoryBackground.targetY = offscreenBelowPositionY;
	}

	if (position <= -3 || position >= 3) {
		offset->categoryBackground.targetX = offscreenPositionX;
		offset->categoryBackground.targetY = baseMenuPositionY;
	}
}

// Update Equipment State
uint16_t updateLinkEquip(void* current_equip, uint8_t boot, uint8_t tunic, uint8_t shield, uint8_t sword)
{
	uint16_t out = 0x0000;
	out |= (boot << 12);
	out |= (tunic << 8);
	out |= (shield << 4);
	out |= sword;
	return out;
}

// Update menu data
void update_menu_t(menu_t* state, z64_inputHandler_t* input, z64_global_t *gl, float currentTime, uint32_t* debug, uint32_t* debug2) {
	float deltaTime = currentTime - state->lastTime;
	state->lastTime = currentTime;

	for (int i = 0; i < NUM_CATEGORIES; i++) {
		update_menuCategory_t(&state->cCategory[i], gl, input, state, currentTime, deltaTime);
	}

	if (!state->menuOpen) {
		if (input->du.buttonState == STATE_PRESSED) state->dPadShow = !state->dPadShow;

		if (input->dl.buttonState == STATE_PRESSED || input->dd.buttonState == STATE_PRESSED || input->dr.buttonState == STATE_PRESSED) {
			if (input->dl.buttonState == STATE_PRESSED) state->cButton = CLEFT;
			if (input->dd.buttonState == STATE_PRESSED) state->cButton = CDOWN;
			if (input->dr.buttonState == STATE_PRESSED) state->cButton = CRIGHT;

			state->menuOpen = 1; 

			for (int i = 0; i < NUM_CATEGORIES; i++) {
				state->cCategory[i].categoryBackground.tile.y = baseMenuPositionY;
				state->cCategory[i].alpha.t = 0;
			}
			
			state->smoothSelectionBox.targetX = offscreenMenuPositionX;
			state->smoothSelectionBox.targetY = baseMenuPositionY;

			return;
		}
	}
	else if (input->l.buttonState == STATE_PRESSED && state->menuOpen) { state->menuOpen = 0; return; }
	
	
	if (state->menuOpen) 
	{
		if (input->dr.buttonState == STATE_PRESSED) state->index++;
		if (input->dl.buttonState == STATE_PRESSED) state->index--;

		if (input->du.buttonState == STATE_PRESSED) {
            state->category--;
            state->index = state->index > state->categoryLength[state->category] - 1 ? state->categoryLength[state->category] - 1: state->index;
			forceMove(state);
        }
		if (input->dd.buttonState == STATE_PRESSED) {
            state->category++;
            state->index = state->index > state->categoryLength[state->category] - 1 ? state->categoryLength[state->category] - 1 : state->index;
			forceMove(state);
        }

		if (input->du.buttonState == STATE_DOWN && currentTime - input->du.invokeTime > 0.5f && currentTime - state->lastScrollTime > state->currentScrollTime) {
            state->category--;
            state->index = state->index > state->categoryLength[state->category] - 1 ? state->categoryLength[state->category] - 1: state->index;
			forceMove(state);
			state->currentScrollTime -= state->scrollTimeDecay;
			state->lastScrollTime = currentTime;
        }
		if (input->dd.buttonState == STATE_DOWN && currentTime - input->dd.invokeTime > 0.5f && currentTime - state->lastScrollTime > state->currentScrollTime) {
            state->category++;
            state->index = state->index > state->categoryLength[state->category] - 1 ? state->categoryLength[state->category] - 1 : state->index;
			forceMove(state);
			state->currentScrollTime -= state->scrollTimeDecay;
			state->lastScrollTime = currentTime;
        }
		if (state->category > NUM_CATEGORIES - 1) state->category = 0;
		if (state->category < 0) state->category = NUM_CATEGORIES - 1;


		if (input->du.buttonState == STATE_UP && input->dd.buttonState == STATE_UP) {
			state->currentScrollTime += state->scrollTimeDecay;
			state->currentDamp -= state->dampDecay;
		}
		state->currentScrollTime = state->currentScrollTime < state->scrollTimeMin ? state->scrollTimeMin : state->currentScrollTime > state->scrollTimeMax ? state->scrollTimeMax : state->currentScrollTime;
		state->currentDamp = state->currentDamp < state->dampMin ? state->dampMin : state->currentDamp > state->dampMax ? state->dampMax : state->currentDamp;


        if (state->index > state->categoryLength[state->category] - 1) state->index = 0;
        if (state->index < 0) state->index = state->categoryLength[state->category] - 1;
		
		uint8_t* inventory = (uint8_t*)Inventory_Context;
        uint16_t* equipment = (uint16_t*)Equipment_Context;
        uint8_t ctx_sword = *equipment & 0x000F;
        uint8_t ctx_shield = (*equipment & 0x00F0) >> 4;
        uint8_t ctx_tunic = (*equipment & 0x0F00) >> 8;
        uint8_t ctx_boot = (*equipment & 0xF000) >> 12;

        state->cCategory[CATEGORY_WEAPON].items[WEAPON_KOKIRI].isShown = ctx_sword & 0b0001; 
        state->cCategory[CATEGORY_WEAPON].items[WEAPON_MASTER].isShown = ctx_sword & 0b0010;    
        state->cCategory[CATEGORY_WEAPON].items[WEAPON_BIGGORON].isShown = ctx_sword & 0b0100;

        state->cCategory[CATEGORY_WEAPON].items[WEAPON_DEKU].isShown = ctx_shield & 0b0001;
        state->cCategory[CATEGORY_WEAPON].items[WEAPON_HYLIAN].isShown = ctx_shield & 0b0010;
        state->cCategory[CATEGORY_WEAPON].items[WEAPON_MIRROR].isShown = ctx_shield & 0b0100;

		state->cCategory[CATEGORY_ARMOR].items[ARMOR_KOKIRI].isShown = ctx_tunic & 0b0001;
        state->cCategory[CATEGORY_ARMOR].items[ARMOR_GORON].isShown = ctx_tunic & 0b0010;
        state->cCategory[CATEGORY_ARMOR].items[ARMOR_ZORA].isShown = ctx_tunic & 0b0100;
    
        state->cCategory[CATEGORY_ARMOR].items[ARMOR_KBOOTS].isShown = ctx_boot & 0b0001;
        state->cCategory[CATEGORY_ARMOR].items[ARMOR_IBOOTS].isShown = ctx_boot & 0b0010;
        state->cCategory[CATEGORY_ARMOR].items[ARMOR_HBOOTS].isShown = ctx_boot & 0b0100;

		state->cCategory[CATEGORY_PROJECTILE].items[PROJ_BOW].isShown = inventory[3] != 0xFF;
		state->cCategory[CATEGORY_PROJECTILE].items[PROJ_FIRE].isShown = inventory[4] != 0xFF;
		state->cCategory[CATEGORY_PROJECTILE].items[PROJ_ICE].isShown = inventory[10] != 0xFF;
		state->cCategory[CATEGORY_PROJECTILE].items[PROJ_LIGHT].isShown = inventory[16] != 0xFF;
		state->cCategory[CATEGORY_PROJECTILE].items[PROJ_SLING].isShown = inventory[6] != 0xFF;
		state->cCategory[CATEGORY_PROJECTILE].items[PROJ_BOOMER].isShown = inventory[12] != 0xFF;
		state->cCategory[CATEGORY_PROJECTILE].items[PROJ_HOOK].isShown = inventory[9] != 0xFF;
		state->cCategory[CATEGORY_PROJECTILE].items[PROJ_LONG].isShown = inventory[9] != 0xFF;

		// Set these to check foreach index in weapon/armor if we have said items. See above/below


		state->cCategory[CATEGORY_MAGIC].items[MAG_DIN].isShown = inventory[5] != 0xFF;
		state->cCategory[CATEGORY_MAGIC].items[MAG_FAR].isShown = inventory[11] != 0xFF;
		state->cCategory[CATEGORY_MAGIC].items[MAG_NAY].isShown = inventory[17] != 0xFF;

		state->cCategory[CATEGORY_HAND].items[HAND_BOC].isShown = inventory[8] != 0xFF;
		state->cCategory[CATEGORY_HAND].items[HAND_BOM].isShown = inventory[2] != 0xFF;
		state->cCategory[CATEGORY_HAND].items[HAND_HAM].isShown = inventory[15] != 0xFF;
		state->cCategory[CATEGORY_HAND].items[HAND_NUT].isShown = inventory[1] != 0xFF;
		state->cCategory[CATEGORY_HAND].items[HAND_STI].isShown = inventory[0] != 0xFF;

		state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_0].isShown = inventory[18] != 0xFF;
		state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_1].isShown = inventory[19] != 0xFF;
		state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_2].isShown = inventory[20] != 0xFF;
		state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_3].isShown = inventory[21] != 0xFF;
		

		if (input->a.buttonState == STATE_PRESSED) 
		{

			uint8_t* current_item = (uint8_t*)(Z64GL_SAVE_CONTEXT + 0x68);
			uint16_t* current_equip = (uint16_t*)(Z64GL_SAVE_CONTEXT + 0x70);

			uint8_t value = state->cButton + 1;

			uint8_t equip_ctx_sword = (*current_equip & 0x000F);
			uint8_t equip_ctx_shield = (*current_equip & 0x00F0) >> 4;
			uint8_t equip_ctx_tunic = (*current_equip & 0x0F00) >> 8;
			uint8_t equip_ctx_boot = (*current_equip & 0xF000) >> 12;

			if (state->category == CATEGORY_PROJECTILE) {
				if (state->index == PROJ_BOW && state->cCategory[0].items[PROJ_BOW].isShown) current_item[value] = 0x03;
				if (state->index == PROJ_BOOMER && state->cCategory[0].items[PROJ_BOOMER].isShown) current_item[value] = 0x0E;
				if (state->index == PROJ_FIRE && state->cCategory[0].items[PROJ_FIRE].isShown) current_item[value] = 0x38;
				if (state->index == PROJ_ICE && state->cCategory[0].items[PROJ_ICE].isShown) current_item[value] = 0x39;
				if (state->index == PROJ_LIGHT && state->cCategory[0].items[PROJ_LIGHT].isShown) current_item[value] = 0x3A;
				if (state->index == PROJ_HOOK && state->cCategory[0].items[PROJ_HOOK].isShown) current_item[value] = 0x0A;
				if (state->index == PROJ_LONG && state->cCategory[0].items[PROJ_LONG].isShown) current_item[value] = 0x0B;
				if (state->index == PROJ_SLING && state->cCategory[0].items[PROJ_SLING].isShown) current_item[value] = 0x06;
			}

			if (state->category == CATEGORY_WEAPON) {
				
				if (state->index == WEAPON_KOKIRI && state->cCategory[CATEGORY_WEAPON].items[WEAPON_KOKIRI].isShown) {current_item[0] = 0x3B; *current_equip = updateLinkEquip(&current_equip, equip_ctx_boot, equip_ctx_tunic, equip_ctx_shield, 1);}
				if (state->index == WEAPON_MASTER && state->cCategory[CATEGORY_WEAPON].items[WEAPON_MASTER].isShown) {current_item[0] = 0x3C; *current_equip = updateLinkEquip(&current_equip, equip_ctx_boot, equip_ctx_tunic, equip_ctx_shield, 2);}
				if (state->index == WEAPON_BIGGORON && state->cCategory[CATEGORY_WEAPON].items[WEAPON_BIGGORON].isShown) {current_item[0] = 0x3D; *current_equip = updateLinkEquip(&current_equip, equip_ctx_boot, equip_ctx_tunic, equip_ctx_shield, 3);}
				if (state->index == WEAPON_DEKU && state->cCategory[CATEGORY_WEAPON].items[WEAPON_DEKU].isShown) equip_ctx_shield = *current_equip = updateLinkEquip(&current_equip, equip_ctx_boot, equip_ctx_tunic, 1, equip_ctx_sword);
				if (state->index == WEAPON_HYLIAN && state->cCategory[CATEGORY_WEAPON].items[WEAPON_HYLIAN].isShown) equip_ctx_shield = *current_equip = updateLinkEquip(&current_equip, equip_ctx_boot, equip_ctx_tunic, 2, equip_ctx_sword);
				if (state->index == WEAPON_MIRROR && state->cCategory[CATEGORY_WEAPON].items[WEAPON_MIRROR].isShown) equip_ctx_shield = *current_equip = updateLinkEquip(&current_equip, equip_ctx_boot, equip_ctx_tunic, 3, equip_ctx_sword);
				
			}

			if (state->category == CATEGORY_ARMOR) {
				
				if (state->index == ARMOR_KOKIRI && state->cCategory[CATEGORY_WEAPON].items[ARMOR_KOKIRI].isShown) *current_equip = updateLinkEquip(&current_equip, equip_ctx_boot, 1, equip_ctx_shield, equip_ctx_sword);
				if (state->index == ARMOR_GORON && state->cCategory[CATEGORY_WEAPON].items[ARMOR_GORON].isShown) *current_equip = updateLinkEquip(&current_equip, equip_ctx_boot, 2, equip_ctx_shield, equip_ctx_sword);
				if (state->index == ARMOR_ZORA && state->cCategory[CATEGORY_WEAPON].items[ARMOR_ZORA].isShown) *current_equip = updateLinkEquip(&current_equip, equip_ctx_boot, 3, equip_ctx_shield, equip_ctx_sword);
				if (state->index == ARMOR_KBOOTS && state->cCategory[CATEGORY_WEAPON].items[ARMOR_KBOOTS].isShown) *current_equip = updateLinkEquip(&current_equip, 1, equip_ctx_tunic, equip_ctx_shield, equip_ctx_sword);
				if (state->index == ARMOR_IBOOTS && state->cCategory[CATEGORY_WEAPON].items[ARMOR_IBOOTS].isShown) *current_equip = updateLinkEquip(&current_equip, 2, equip_ctx_tunic, equip_ctx_shield, equip_ctx_sword);
				if (state->index == ARMOR_HBOOTS && state->cCategory[CATEGORY_WEAPON].items[ARMOR_HBOOTS].isShown) *current_equip = updateLinkEquip(&current_equip, 3, equip_ctx_tunic, equip_ctx_shield, equip_ctx_sword);
				
			}

			if (state->category == CATEGORY_BOTTLE) {
				if (state->index == BOTTLE_0 && state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_0].isShown) current_item[value] = 0x14;
				if (state->index == BOTTLE_1 && state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_1].isShown) current_item[value] = 0x15;
				if (state->index == BOTTLE_2 && state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_2].isShown) current_item[value] = 0x16;
				if (state->index == BOTTLE_3 && state->cCategory[CATEGORY_BOTTLE].items[BOTTLE_3].isShown) current_item[value] = 0x17;
			}
			
			if (state->category == CATEGORY_HAND) {
				if (state->index == HAND_HAM && state->cCategory[CATEGORY_HAND].items[HAND_HAM].isShown) current_item[value] = 0x11;
				if (state->index == HAND_BOM && state->cCategory[CATEGORY_HAND].items[HAND_BOM].isShown) current_item[value] = 0x02;
				if (state->index == HAND_BOC && state->cCategory[CATEGORY_HAND].items[HAND_BOC].isShown) current_item[value] = 0x09;
				if (state->index == HAND_STI && state->cCategory[CATEGORY_HAND].items[HAND_STI].isShown) current_item[value] = 0x00;
				if (state->index == HAND_NUT && state->cCategory[CATEGORY_HAND].items[HAND_NUT].isShown) current_item[value] = 0x01;
			}

			if (state->category == CATEGORY_MAGIC) {
				if (state->index == MAG_DIN && state->cCategory[CATEGORY_MAGIC].items[MAG_DIN].isShown) current_item[value] = 0x05;
				if (state->index == MAG_FAR && state->cCategory[CATEGORY_MAGIC].items[MAG_FAR].isShown) current_item[value] = 0x0D;
				if (state->index == MAG_NAY && state->cCategory[CATEGORY_MAGIC].items[MAG_NAY].isShown) current_item[value] = 0x13;
			}	
			gfx_update_item_icon(gl, state->cButton + 1);
			gfx_update_item_icon(gl, 0);
			player_refresh_equipment(gl, zh_get_player(gl));
		}

		int aCat = 0;
		int aaCat = 0;
		int bCat = 0;
		int bbCat = 0;
		int bbbCat = 0;

		aCat = state->cMeta[state->category].above;
		aaCat = state->cMeta[aCat].above;
		bCat = state->cMeta[state->category].below;
		bbCat = state->cMeta[bCat].below;
		bbbCat = state->cMeta[bbCat].below;

		updateCategoryPosition(&state->cCategory[state->category], 0);
		updateCategoryPosition(&state->cCategory[aCat], 1);
		updateCategoryPosition(&state->cCategory[aaCat], 2);
		updateCategoryPosition(&state->cCategory[bCat], -1);
		updateCategoryPosition(&state->cCategory[bbCat], -2);
		updateCategoryPosition(&state->cCategory[bbbCat], -2);

		state->smoothSelectionBox.targetX = state->cCategory[state->category].items[state->index].item.tile.x;
		state->smoothSelectionBox.targetY = state->cCategory[state->category].items[state->index].item.tile.y;
	}
	else 
	{
		for (int i = 0; i < 6; i++) {
			state->cCategory[i].categoryBackground.targetX = offscreenMenuPositionX;
		}

		state->cCategory[state->cMeta[state->category].above].categoryBackground.targetY += offscreenMenuOffsetY;
		state->cCategory[state->cMeta[state->category].below].categoryBackground.targetY -= offscreenMenuOffsetY;
        
		state->smoothSelectionBox.targetX = offscreenMenuPositionX;
	}

    state->selectionAlpha.t += state->alphaDir;
    if (state->selectionAlpha.t < 35 || state->selectionAlpha.t >= 255) state->alphaDir = -state->alphaDir;
    if (state->selectionAlpha.t < 0) state->selectionAlpha.t = 0;
    if (state->selectionAlpha.t > 255) state->selectionAlpha.t = 255;
    interpolateInt(deltaTime, 3, &state->selectionAlpha.v, &state->selectionAlpha.p, state->selectionAlpha.t);
    update_guiObject_t(&state->smoothSelectionBox, currentTime, state->demandImmediateUpdate);

    state->selectionBox.tile.x = state->smoothSelectionBox.targetX;
    state->selectionBox.tile.y = state->smoothSelectionBox.targetY;

    zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &state->selectionBox.texture, &state->selectionBox.tile, state->selectionAlpha.p);
    zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &state->smoothSelectionBox.texture, &state->smoothSelectionBox.tile, state->selectionAlpha.p / 3);

    zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &state->selectionBox.texture, &state->selectionBox.tile, state->selectionAlpha.p);
    zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &state->smoothSelectionBox.texture, &state->smoothSelectionBox.tile, state->selectionAlpha.p / 3);

	if (state->menuOpen && state->dPadShow) 
	{
		state->dPadBottom.texture.timg = &tDpad3;
		zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &state->dPadBottom.texture, &state->dPadBottom.tile, 240);
		state->dPadTop.texture.timg = &tDpad2;
		zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &state->dPadTop.texture, &state->dPadTop.tile, 240);
	}
	else if (state->dPadShow)
	{
		state->dPadTop.texture.timg = &tDpad1;
		zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &state->dPadTop.texture, &state->dPadTop.tile, 240);
		state->dPadBottom.texture.timg = &tDpad0;
		zh_draw_ui_sprite(&gl->common.gfx_ctxt->overlay, &state->dPadBottom.texture, &state->dPadBottom.tile, 240);
	}
	state->demandImmediateUpdate = 0;
}

#endif


