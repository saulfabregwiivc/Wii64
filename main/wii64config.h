/* Wii64 - wii64config.h
 * External declaration and enumeration of config variables
 * by sepp256
**/

#ifndef WII64CONFIG_H
#define WII64CONFIG_H

#include "timers.h"


extern char audioEnabled;
enum audioEnabled
{
	AUDIO_DISABLE=0,
	AUDIO_ENABLE
};

extern char showFPSonScreen;
enum showFPSonScreen
{
	FPS_HIDE=0,
	FPS_SHOW
};

extern char printToScreen;
enum printToScreen
{
	DEBUG_HIDE=0,
	DEBUG_SHOW
};

extern char printToSD;
enum printToSD
{
	SDLOG_DISABLE=0,
	SDLOG_ENABLE
};

extern timers Timers;			//Timers.limitVIs: 
enum limitVIs
{
	LIMITVIS_NONE=0,
	LIMITVIS_WAIT_FOR_VI,
	LIMITVIS_WAIT_FOR_FRAME
};

extern char saveEnabled;	//???

extern char nativeSaveDevice;
enum nativeSaveDevice
{
	NATIVESAVEDEVICE_SD=0,
	NATIVESAVEDEVICE_USB,
	NATIVESAVEDEVICE_CARDA,
	NATIVESAVEDEVICE_CARDB
};

extern char saveStateDevice;
enum saveStateDevice
{
	SAVESTATEDEVICE_SD=0,
	SAVESTATEDEVICE_USB
};

extern char autoSave; //TODO: Use me
enum autoSave
{
	AUTOSAVE_DISABLE=0,
	AUTOSAVE_ENABLE
};

extern char autoLoad; //TODO: Use me
enum autoLoad
{
	AUTOLOAD_DISABLE=0,
	AUTOLOAD_ENABLE
};

extern char creditsScrolling;	//deprecated?

extern unsigned long dynacore;
enum dynacore
{
	DYNACORE_INTERPRETER=0,
	DYNACORE_DYNAREC,
	DYNACORE_PURE_INTERP
};

extern char widescreen;
enum widescreen
{
	SCREENMODE_4x3=0,
	SCREENMODE_16x9
};

#ifdef GLN64_GX
extern char glN64_useFrameBufferTextures;
enum glN64_useFrameBufferTextures
{
	GLN64_FBTEX_DISABLE=0,
	GLN64_FBTEX_ENABLE
};

extern char glN64_use2xSaiTextures;
enum glN64_use2xSaiTextures
{
	GLN64_2XSAI_DISABLE=0,
	GLN64_2XSAI_ENABLE
};
#endif //GLN64_GX


#endif //WII64CONFIG_H
