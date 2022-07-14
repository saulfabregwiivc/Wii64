/**
 * Wii64 - controller-Classic.c
 * Copyright (C) 2007, 2008, 2009, 2010 Mike Slegeir
 * Copyright (C) 2007, 2008, 2009, 2010 sepp256
 * 
 * Classic controller input module
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: tehpola@gmail.com
 *                sepp256@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
**/


#include <string.h>
#include <malloc.h>
#include <math.h>
#include <wiiuse/wpad.h>
#include "controller.h"
#include "../gui/DEBUG.h"

#ifndef PI
#define PI 3.14159f
#endif

enum { STICK_X, STICK_Y };
static int getStickValue(joystick_t* j, float maxMag, int axis, int maxAbsValue){
	double angle = PI * j->ang/180.0f;
	double magnitude = (j->mag/maxMag > 1.0f) ? 1.0f :
	                    (j->mag/maxMag < -1.0f) ? -1.0f : j->mag/maxMag;
	double value;
	if(axis == STICK_X)
		value = magnitude * sin( angle );
	else
		value = magnitude * cos( angle );
	if(value < -0.1f) value = (value+0.1f)*1.111f;
	else if(value > 0.1f) value = (value-0.1f)*1.111f;
	else value = 0.f;
	return (int)(value * maxAbsValue);
}

enum {
	LSTICK_AS_ANALOG = 1, RSTICK_AS_ANALOG = 2,
};

enum {
	LSTICK_L = 0x01 << 24,
	LSTICK_R = 0x02 << 24,
	LSTICK_U = 0x04 << 24,
	LSTICK_D = 0x08 << 24,
	RSTICK_L = 0x10 << 24,
	RSTICK_R = 0x20 << 24,
	RSTICK_U = 0x40 << 24,
	RSTICK_D = 0x80 << 24,
};

#define NUM_CLASSIC_BUTTONS 24
static button_t buttons[] = {
	{  0, ~0,                         "None" },
	{  1, CLASSIC_CTRL_BUTTON_UP,     "D-Up" },
	{  2, CLASSIC_CTRL_BUTTON_LEFT,   "D-Left" },
	{  3, CLASSIC_CTRL_BUTTON_RIGHT,  "D-Right" },
	{  4, CLASSIC_CTRL_BUTTON_DOWN,   "D-Down" },
	{  5, CLASSIC_CTRL_BUTTON_FULL_L, "L" },
	{  6, CLASSIC_CTRL_BUTTON_FULL_R, "R" },
	{  7, CLASSIC_CTRL_BUTTON_ZL,     "Left Z" },
	{  8, CLASSIC_CTRL_BUTTON_ZR,     "Right Z" },
	{  9, CLASSIC_CTRL_BUTTON_A,      "A" },
	{ 10, CLASSIC_CTRL_BUTTON_B,      "B" },
	{ 11, CLASSIC_CTRL_BUTTON_X,      "X" },
	{ 12, CLASSIC_CTRL_BUTTON_Y,      "Y" },
	{ 13, CLASSIC_CTRL_BUTTON_PLUS,   "+" },
	{ 14, CLASSIC_CTRL_BUTTON_MINUS,  "-" },
	{ 15, CLASSIC_CTRL_BUTTON_HOME,   "Home" },
	{ 16, RSTICK_U,                   "RS-Up" },
	{ 17, RSTICK_L,                   "RS-Left" },
	{ 18, RSTICK_R,                   "RS-Right" },
	{ 19, RSTICK_D,                   "RS-Down" },
	{ 20, LSTICK_U,                   "LS-Up" },
	{ 21, LSTICK_L,                   "LS-Left" },
	{ 22, LSTICK_R,                   "LS-Right" },
	{ 23, LSTICK_D,                   "LS-Down" },
	{ 24, WIIU_PRO_CTRL_BUTTON_LS,    "LS" },
	{ 25, WIIU_PRO_CTRL_BUTTON_RS,    "RS" },
};

static button_t analog_sources[] = {
	{ 0, L_STICK_AS_ANALOG,  "Left Stick" },
	{ 1, R_STICK_AS_ANALOG,  "Right Stick" },
};

static button_t menu_combos[] = {
	{ 0, CLASSIC_CTRL_BUTTON_X|CLASSIC_CTRL_BUTTON_Y, "X+Y" },
	{ 1, CLASSIC_CTRL_BUTTON_ZL|CLASSIC_CTRL_BUTTON_ZR, "ZL+ZR" },
	{ 2, CLASSIC_CTRL_BUTTON_HOME, "Home" },
};

static int checkType(int Control, int type){
	int err;
	u32 expType;
	err = WPAD_Probe(Control, &expType);

	if(err != WPAD_ERR_NONE)
		return -1;

	switch(expType){
	case WPAD_EXP_NONE:
		controller_Wiimote.available[Control] = 1;
		break;
	case WPAD_EXP_NUNCHUK:
		controller_WiimoteNunchuk.available[Control] = 1;
		break;
	case WPAD_EXP_CLASSIC:
		controller_Classic.available[Control] = 1;
		break;
	case WPAD_EXP_WIIUPRO:
		controller_WiiUPro.available[Control] = 1;
		break;
	}

	return expType;
}

static unsigned int getButtonsCC(classic_ctrl_t* controller, float maxLMag, float maxRMag)
{
	unsigned int b = (unsigned)controller->btns;
	s8 stickX      = getStickValue(&controller->ljs, maxLMag, STICK_X, 7);
	s8 stickY      = getStickValue(&controller->ljs, maxLMag, STICK_Y, 7);
	s8 substickX   = getStickValue(&controller->rjs, maxRMag, STICK_X, 7);
	s8 substickY   = getStickValue(&controller->rjs, maxRMag, STICK_Y, 7);
	
	if(stickX    < -3) b |= LSTICK_L;
	if(stickX    >  3) b |= LSTICK_R;
	if(stickY    >  3) b |= LSTICK_U;
	if(stickY    < -3) b |= LSTICK_D;
	
	if(substickX < -3) b |= RSTICK_L;
	if(substickX >  3) b |= RSTICK_R;
	if(substickY >  3) b |= RSTICK_U;
	if(substickY < -3) b |= RSTICK_D;
	
	return b;
}

static int availableCC(int Control){
	if(checkType(Control, WPAD_EXP_CLASSIC) != WPAD_EXP_CLASSIC){
		controller_Classic.available[Control] = 0;
		return 0;
	} else {
		return 1;
	}
}

#define DEFAULT_MAX_MAG 0.667f
typedef struct maxMagEntry_t {
	struct bd_addr bdaddr;
	float maxLMag;
	float maxRMag;
} maxMagEntry;

static maxMagEntry* maxMagTable = NULL;
static int maxMagTableSize = 0;

static void SetMaxMag(const struct bd_addr *bdaddr, float magL, float magR, float* maxLMag, float* maxRMag)
{
	int i;
	int match_ind = -1;
	//Find bdaddr in table
	if (maxMagTable)
	{
		for (i = 0; i<maxMagTableSize; i++)
		{
			if (bd_addr_cmp(bdaddr, &maxMagTable[i].bdaddr)) //Found a match!
			{
				match_ind = i;
				break;
				maxMagTable[maxMagTableSize-1].maxLMag = magL > DEFAULT_MAX_MAG ? magL : DEFAULT_MAX_MAG;
				maxMagTable[maxMagTableSize-1].maxRMag = magR > DEFAULT_MAX_MAG ? magR : DEFAULT_MAX_MAG;
				*maxLMag = maxMagTable[maxMagTableSize-1].maxLMag;
				*maxRMag = maxMagTable[maxMagTableSize-1].maxRMag;
			}
		}
	}
	if (match_ind < 0) //Make new entry in table
	{ 
		maxMagTableSize++;
		maxMagTable = realloc(maxMagTable, sizeof(maxMagEntry)*maxMagTableSize);
		if (maxMagTable==NULL) return;
		match_ind = maxMagTableSize-1;
		maxMagTable[match_ind].bdaddr = *bdaddr;
		maxMagTable[match_ind].maxLMag = DEFAULT_MAX_MAG;
		maxMagTable[match_ind].maxRMag = DEFAULT_MAX_MAG;
	}
	//Compare and Update Table Entry
	maxMagTable[match_ind].maxLMag = magL > maxMagTable[match_ind].maxLMag ? magL : maxMagTable[match_ind].maxLMag;
	maxMagTable[match_ind].maxRMag = magR > maxMagTable[match_ind].maxRMag ? magR : maxMagTable[match_ind].maxRMag;
	*maxLMag = maxMagTable[match_ind].maxLMag;
	*maxRMag = maxMagTable[match_ind].maxRMag;
}

static int GetKeysCC(int Control, BUTTONS * Keys, controller_config_t* config)
{
	if(wpadNeedScan){ WPAD_ScanPads(); wpadNeedScan = 0; }
	WPADData* wpad = WPAD_Data(Control);
	BUTTONS* c = Keys;
	memset(c, 0, sizeof(BUTTONS));

	// Only use a connected classic controller
	if(!availableCC(Control))
		return 0;

	//Look up BT address
	//wiimote* WPAD_GetWiimotes(s32 chan)
	//wiimote* wm = WPAD_GetWiimote(Control);
	float maxLMag = DEFAULT_MAX_MAG;
	float maxRMag = DEFAULT_MAX_MAG; 
	//if(wm) SetMaxMag(&wm->bdaddr, wpad->exp.classic.ljs.mag, wpad->exp.classic.rjs.mag, &maxLMag, &maxRMag);
	
	unsigned int b = getButtonsCC(&wpad->exp.classic, maxLMag, maxRMag);
	inline int isHeld(button_tp button){
		return (b & button->mask) == button->mask;
	}
	
	c->R_DPAD       = isHeld(config->DR);
	c->L_DPAD       = isHeld(config->DL);
	c->D_DPAD       = isHeld(config->DD);
	c->U_DPAD       = isHeld(config->DU);
	
	c->START_BUTTON = isHeld(config->START);
	c->B_BUTTON     = isHeld(config->B);
	c->A_BUTTON     = isHeld(config->A);

	c->Z_TRIG       = isHeld(config->Z);
	c->R_TRIG       = isHeld(config->R);
	c->L_TRIG       = isHeld(config->L);

	c->R_CBUTTON    = isHeld(config->CR);
	c->L_CBUTTON    = isHeld(config->CL);
	c->D_CBUTTON    = isHeld(config->CD);
	c->U_CBUTTON    = isHeld(config->CU);

	if(config->analog->mask == LSTICK_AS_ANALOG){
		c->X_AXIS = getStickValue(&wpad->exp.classic.ljs, maxLMag, STICK_X, 80);
		c->Y_AXIS = getStickValue(&wpad->exp.classic.ljs, maxLMag, STICK_Y, 80);
		//sprintf(txtbuffer,"GetKeys: ctr %d, ang %f, mag %f, max %f, posx %x, posy %x, x %d, y %d", Control, wpad->exp.classic.ljs.ang, wpad->exp.classic.ljs.mag, maxLMag, wpad->exp.classic.ljs.pos.x, wpad->exp.classic.ljs.pos.y, c->X_AXIS, c->Y_AXIS);
	} else if(config->analog->mask == RSTICK_AS_ANALOG){
		c->X_AXIS = getStickValue(&wpad->exp.classic.rjs, maxRMag, STICK_X, 80);
		c->Y_AXIS = getStickValue(&wpad->exp.classic.rjs, maxRMag, STICK_Y, 80);
		//sprintf(txtbuffer,"GetKeys: ctr %d, ang %f, mag %f, max %f, posx %x, posy %x, x %d, y %d", Control, wpad->exp.classic.rjs.ang, wpad->exp.classic.rjs.mag, maxRMag, wpad->exp.classic.rjs.pos.x, wpad->exp.classic.rjs.pos.y, c->X_AXIS, c->Y_AXIS);
	}
	if(config->invertedY) c->Y_AXIS = -c->Y_AXIS;

	//DEBUG_print(txtbuffer,DBG_RSPINFO1+Control);

	// Return whether the exit button(s) are pressed
	return isHeld(config->exit);
}

static unsigned int getButtonsWUP(wiiu_pro_ctrl_t* controller)
{
	unsigned int b = controller->btns;

	float stickX    = getStickValue(&controller->ljs, STICK_X, 1);
	float stickY    = getStickValue(&controller->ljs, STICK_Y, 1);
	float substickX = getStickValue(&controller->rjs, STICK_X, 1);
	float substickY = getStickValue(&controller->rjs, STICK_Y, 1);

	if(stickX    < -.5) b |= LSTICK_L;
	if(stickX    >  .5) b |= LSTICK_R;
	if(stickY    >  .5) b |= LSTICK_U;
	if(stickY    < -.5) b |= LSTICK_D;

	if(substickX < -.5) b |= RSTICK_L;
	if(substickX >  .5) b |= RSTICK_R;
	if(substickY >  .5) b |= RSTICK_U;
	if(substickY < -.5) b |= RSTICK_D;

	return b;
}

static int availableWUP(int Control){
	if(checkType(Control, WPAD_EXP_WIIUPRO) != WPAD_EXP_WIIUPRO){
		controller_WiiUPro.available[Control] = 0;
		return 0;
	} else {
		return 1;
	}
}

static int GetKeysWUP(int Control, BUTTONS * Keys, controller_config_t* config)
{
	WPADData* wpad = WPAD_Data(Control);
	BUTTONS* c = Keys;
	memset(c, 0, sizeof(BUTTONS));

	// Only use a connected wiiu pro controller
	if(!availableWUP(Control))
		return 0;

	WPAD_ReadPending(Control, NULL);

	unsigned int b = getButtonsWUP(&wpad->exp.wup);
	inline int isHeld(button_tp button){
		return (b & button->mask) == button->mask;
	}

	c->R_DPAD       = isHeld(config->DR);
	c->L_DPAD       = isHeld(config->DL);
	c->D_DPAD       = isHeld(config->DD);
	c->U_DPAD       = isHeld(config->DU);

	c->START_BUTTON = isHeld(config->START);
	c->B_BUTTON     = isHeld(config->B);
	c->A_BUTTON     = isHeld(config->A);

	c->Z_TRIG       = isHeld(config->Z);
	c->R_TRIG       = isHeld(config->R);
	c->L_TRIG       = isHeld(config->L);

	c->R_CBUTTON    = isHeld(config->CR);
	c->L_CBUTTON    = isHeld(config->CL);
	c->D_CBUTTON    = isHeld(config->CD);
	c->U_CBUTTON    = isHeld(config->CU);

	if(config->analog->mask == LSTICK_AS_ANALOG){
		c->X_AXIS = getStickValue(&wpad->exp.wup.ljs, STICK_X, 80);
		c->Y_AXIS = getStickValue(&wpad->exp.wup.ljs, STICK_Y, 80);
	} else if(config->analog->mask == RSTICK_AS_ANALOG){
		c->X_AXIS = getStickValue(&wpad->exp.wup.rjs, STICK_X, 80);
		c->Y_AXIS = getStickValue(&wpad->exp.wup.rjs, STICK_Y, 80);
	} else if(config->analog->mask == BUTTON_AS_ANALOG){
		if(b & WIIU_PRO_CTRL_BUTTON_RIGHT)
			c->X_AXIS = +80;
		else if(b & WIIU_PRO_CTRL_BUTTON_LEFT)
			c->X_AXIS = -80;
		else
			c->X_AXIS = 0;

		if(b & WIIU_PRO_CTRL_BUTTON_UP)
			c->Y_AXIS = +80;
		else if(b & WIIU_PRO_CTRL_BUTTON_DOWN)
			c->Y_AXIS = -80;
		else
			c->Y_AXIS = 0;
	}
	if(config->invertedY) c->Y_AXIS = -c->Y_AXIS;

	// Return whether the exit button(s) are pressed
	return isHeld(config->exit);
}

static void pause(int Control){
	WPAD_Rumble(Control, 0);
}

static void resume(int Control){ }

static void rumble(int Control, int rumble){
	WPAD_Rumble(Control, rumble ? 1 : 0);
}

static void configure(int Control, controller_config_t* config){
	// Don't know how this should be integrated
}

static void assign(int p, int v){
	// TODO: Light up the LEDs appropriately
}

static void refreshAvailableCC(void);
static void refreshAvailableWUP(void);

controller_t controller_Classic =
	{ 'C',
	  GetKeysCC,
	  configure,
	  assign,
	  pause,
	  resume,
	  rumble,
	  refreshAvailableCC,
	  {0, 0, 0, 0},
	  NUM_CLASSIC_BUTTONS,
	  buttons,
	  sizeof(analog_sources)/sizeof(analog_sources[0]),
	  analog_sources,
	  sizeof(menu_combos)/sizeof(menu_combos[0]),
	  menu_combos,
	  { .DU        = &buttons[1],  // D-Pad Up
	    .DL        = &buttons[2],  // D-Pad Left
	    .DR        = &buttons[3],  // D-Pad Right
	    .DD        = &buttons[4],  // D-Pad Down
	    .Z         = &buttons[5],  // Left Z
	    .L         = &buttons[8],  // Left Trigger
	    .R         = &buttons[6],  // Right Trigger
	    .A         = &buttons[9],  // A
	    .B         = &buttons[10], // B
	    .START     = &buttons[13], // +
	    .CU        = &buttons[16], // Right Stick Up
	    .CL        = &buttons[17], // Right Stick Left
	    .CR        = &buttons[18], // Right Stick Right
	    .CD        = &buttons[19], // Right Stick Down
	    .analog    = &analog_sources[0],
	    .exit      = &menu_combos[2],
	    .invertedY = 0,
	  }
	 };

controller_t controller_WiiUPro =
	{ 'P',
	  GetKeysWUP,
	  configure,
	  assign,
	  pause,
	  resume,
	  rumble,
	  refreshAvailableWUP,
	  {0, 0, 0, 0},
	  sizeof(buttons)/sizeof(buttons[0]),
	  buttons,
	  sizeof(analog_sources)/sizeof(analog_sources[0]),
	  analog_sources,
	  sizeof(menu_combos)/sizeof(menu_combos[0]),
	  menu_combos,
	  { .DU        = &buttons[1],  // D-Pad Up
	    .DL        = &buttons[2],  // D-Pad Left
	    .DR        = &buttons[3],  // D-Pad Right
	    .DD        = &buttons[4],  // D-Pad Down
	    .Z         = &buttons[7],  // Left Z
	    .L         = &buttons[6],  // Right Trigger
	    .R         = &buttons[8],  // Right Z
	    .A         = &buttons[9],  // A
	    .B         = &buttons[10], // B
	    .START     = &buttons[13], // +
	    .CU        = &buttons[16], // Right Stick Up
	    .CL        = &buttons[17], // Right Stick Left
	    .CR        = &buttons[18], // Right Stick Right
	    .CD        = &buttons[19], // Right Stick Down
	    .analog    = &analog_sources[0],
	    .exit      = &menu_combos[2],
	    .invertedY = 0,
	  }
	 };

static void refreshAvailableCC(void){
	int i;
	for(i=0; i<4; ++i){
		availableCC(i);
	}
}

static void refreshAvailableWUP(void){
	int i;
	for(i=0; i<4; ++i){
		availableWUP(i);
	}
}
