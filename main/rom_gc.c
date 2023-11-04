/**
 * Mupen64 - rom_gc.c
 * Copyright (C) 2002 Hacktarux,
 * Wii/GC Additional code by tehpola, emu_kidid
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 *
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
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
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ogc/machine/processor.h>
#include "../gui/GUI.h"
#include "rom.h"
#include "ROM-Cache.h"
#include "wii64config.h"
#include "../gc_memory/memory.h"
#include "../fileBrowser/fileBrowser.h"
#include "gamehacks.h"

#define PRINT GUI_print

static fileBrowser_file* rom_file;
int rom_length;
rom_header ROM_HEADER;
rom_settings ROM_SETTINGS;

void stripInvalidChars(char *str) {
	int i = 0;
	for(i = 0; i < strlen(str); i++) {
		if(str[i] == '\\' || str[i] == '/' || str[i] == ':'|| str[i] == '*'
		|| str[i] == '?'|| str[i] == '"'|| str[i] == '<'|| str[i] == '>'|| str[i] == '|') {
			str[i] = '_';
		}
	}
}

int init_byte_swap(unsigned int magicWord){

	switch(magicWord){
		case 0x37804012:					//37804012 aka byteswapped
			return BYTE_SWAP_BYTE;
		case 0x40123780:					//40123780 aka little endian, aka halfswapped
			return BYTE_SWAP_HALF;
		case 0x80371240:
			return BYTE_SWAP_NONE;
	}
	return BYTE_SWAP_BAD;
}

void byte_swap(char* buffer, unsigned int length, int byte_swap_type) {
	if(byte_swap_type == BYTE_SWAP_NONE || byte_swap_type == BYTE_SWAP_BAD)
		return;

	int i = 0;
	
	if(byte_swap_type == BYTE_SWAP_HALF){	//aka little endian (40123780) vs (80371240)
		for (i = 0; i < (length & ~3); i += 4)
			*(u32 *)(buffer + i) = __lwbrx(buffer, i);
	} else if(byte_swap_type == BYTE_SWAP_BYTE){	// (37804012) vs (80371240)
		for (i = 0; i < (length & ~3); i += 2)
			*(u16 *)(buffer + i) = __lhbrx(buffer, i);
	}
}

#define TOTAL_NUM_16KBIT 56
static unsigned int CRC_TABLE[TOTAL_NUM_16KBIT][2] = {
	{ 0x514B6900, 0xB4B19881 }, // Banjo to Kazooie no Daibouken 2 (J)
	{ 0x155B7CDF, 0xF0DA7325 }, // Banjo-Tooie (A)
	{ 0xC9176D39, 0xEA4779D1 }, // Banjo-Tooie (E) (M4)
	{ 0xC2E9AA9A, 0x475D70AA }, // Banjo-Tooie (U)
	{ 0x373F5889, 0x9A6CA80A }, // Conker's Bad Fur Day (E)
	{ 0x8BC3A47A, 0x74221294 }, // Conker's Bad Fur Day (U) (Debug Version)
	{ 0xA08D0F77, 0x6F82E38C }, // Conker's Bad Fur Day (U) (ECTS Demo)
	{ 0x30C7AC50, 0x7704072D }, // Conker's Bad Fur Day (U)
	{ 0x83F3931E, 0xCB72223D }, // Cruis'n World (E)
	{ 0xDFE61153, 0xD76118E6 }, // Cruis'n World (U)
	{ 0x079501B9, 0xAB0232AB }, // Custom Robo V2 (J)
	{ 0x17C54A61, 0x4A83F2E7 }, // Densha de GO! 64 (J)
	{ 0x68D128AE, 0x67D60F21 }, // Densha de GO! 64 (J) [T]
	{ 0x11936D8C, 0x6F2C4B43 }, // Donkey Kong 64 (E)
	{ 0x053C89A7, 0xA5064302 }, // Donkey Kong 64 (J)
	{ 0xEC58EABF, 0xAD7C7169 }, // Donkey Kong 64 (U)
	{ 0x0DD4ABAB, 0xB5A2A91E }, // Donkey Kong 64 (U) (Kiosk Demo)
	{ 0xB6306E99, 0xB63ED2B2 }, // Doraemon 2 - Nobita to Hikari no Shinden (J)
	{ 0xA8275140, 0xB9B056E8 }, // Doraemon 3 - Nobita no Machi SOS! (J)
	{ 0x202A8EE4, 0x83F88B89 }, // Excitebike 64 (E)
	{ 0x861C3519, 0xF6091CE5 }, // Excitebike 64 (J)
	{ 0x07861842, 0xA12EBC9F }, // Excitebike 64 (U) (V1.0)
	{ 0xF9D411E3, 0x7CB29BC0 }, // Excitebike 64 (U) (V1.1)
	{ 0xAF754F7B, 0x1DD17381 }, // Excitebike 64 (U) (Kiosk Demo)
	{ 0xFF42BE66, 0x91CA9B1D }, // GoldenEye X 5d
	{ 0xA4844AA2, 0x8E13A961 }, // GoldenEye X 5d (Cloudless)
	{ 0x0C581C7A, 0x3D6E20E4 }, // Hoshi no Kirby 64 (J) (V1.2)
	{ 0xBCB1F89F, 0x060752A2 }, // Hoshi no Kirby 64 (J) (V1.3)
	{ 0x0D93BA11, 0x683868A6 }, // Kirby 64 - The Crystal Shards (E)
	{ 0x46039FB4, 0x0337822C }, // Kirby 64 - The Crystal Shards (U)
	{ 0x1739EFBA, 0xD0B43A68 }, // Kobe Bryant's NBA Courtside (E)
	{ 0x616B8494, 0x8A509210 }, // Kobe Bryant's NBA Courtside (U)
	{ 0xD7134F8D, 0xC11A00B5 }, // Madden NFL 2002 (U)
	{ 0xC5674160, 0x0F5F453C }, // Mario Party 3 (E) (M4)
	{ 0x0B0AB4CD, 0x7B158937 }, // Mario Party 3 (J)
	{ 0x7C3829D9, 0x6E8247CE }, // Mario Party 3 (U)
	{ 0x839F3AD5, 0x406D15FA }, // Mario Tennis (E)
	{ 0x5001CF4F, 0xF30CB3BD }, // Mario Tennis (U)
	{ 0x3A6C42B5, 0x1ACADA1B }, // Mario Tennis 64 (J)
	{ 0x147E0EDB, 0x36C5B12C }, // Neon Genesis Evangelion (J)
	{ 0xCFE2CB31, 0x4D6B1E1D }, // Parlor! Pro 64 - Pachinko Jikki Simulation Game (J)
	{ 0xF468118C, 0xE32EE44E }, // PD Ultraman Battle Collection 64 (J)
	{ 0xE4B08007, 0xA602FF33 }, // Perfect Dark (E) (M5)
	{ 0x96747EB4, 0x104BB243 }, // Perfect Dark (J)
	{ 0xDDF460CC, 0x3CA634C0 }, // Perfect Dark (U) (v1.0)
	{ 0x41F2B98F, 0xB458B466 }, // Perfect Dark (U) (v1.1)
	{ 0x766309EA, 0x3F64A369 }, // Perfect Dark XBLA Mp3 (1.0)
	{ 0x272B690F, 0xAD0A7A77 }, // Robot Ponkottsu 64 - 7tsu no Umi no Caramel (J)
	{ 0xFEE97010, 0x4E94A9A0 }, // RR64 - Ridge Racer 64 (E)
	{ 0x2500267E, 0x2A7EC3CE }, // RR64 - Ridge Racer 64 (U)
	{ 0x53ED2DC4, 0x06258002 }, // Star Wars Episode I - Racer (E) (M3)
	{ 0x61F5B152, 0x046122AB }, // Star Wars Episode I - Racer (J)
	{ 0x72F70398, 0x6556A98B }, // Star Wars Episode I - Racer (U)
	{ 0x2DCFCA60, 0x8354B147 }, // Yoshi Story (J)
	{ 0xD3F97D49, 0x6924135B }, // Yoshi's Story (E) (M3)
	{ 0x2337D8E8, 0x6B8E7CEC }, // Yoshi's Story (U) (M2)
  };

// Checks if the current game is in the CRC list for 16kbit eeprom save type
// cause it's cheaper to have a CRC list than an entire .ini file :)
int isROMEEPROM16k()
{
  int i = 0;
  unsigned int curCRC[2];
  ROMCache_read((u8*)&curCRC[0], 0x10, sizeof(unsigned int)*2);

  for (i = 0; i < TOTAL_NUM_16KBIT; i++)
  {
    if((CRC_TABLE[i][0] == curCRC[0])&&(CRC_TABLE[i][1] == curCRC[1]))
      return 1;
  }
  return 0;
}

/* Loads the ROM into the ROM cache */
int rom_read(fileBrowser_file* file){

   int i;

   rom_file = file;
   rom_length = file->size;

   ROMCache_init(rom_file);
   int ret = ROMCache_load(rom_file);
   if(ret) {
     ROMCache_deinit(rom_file);
     return ret;
   }
   ROMCache_read(&ROM_HEADER, 0, sizeof(rom_header));

   // Swap country code back since I know the emulator relies on this being little endian.
  ROM_HEADER.Country_code = bswap16(ROM_HEADER.Country_code);
  //Copy header name as Goodname (in the .ini we can use CRC to identify ROMS)
  memset((char*)buffer,0,1024);
  strncpy(buffer, (char*)ROM_HEADER.Name,20);
  //Maximum ROM name is 20 bytes. Lets make sure we cut off trailing spaces
  for(i = strlen(buffer); i>0; i--)
  {
    if(buffer[i-1] !=  ' ') {
  		strncpy(&ROM_SETTINGS.goodname[0],&buffer[0],i);
  		ROM_SETTINGS.goodname[i] = 0; //terminate it too
  		break;
    }
  }
  // Replace any non file system complaint chars with underscores
  stripInvalidChars((char*)&ROM_SETTINGS.goodname[0]);
  // Fix save type for certain special sized (16kbit) eeprom games
  ROM_SETTINGS.eeprom_16kb = isROMEEPROM16k();
  // Apply game specific hacks
  GameSpecificHackSetup();
  //Set VI limit based on ROM header
  InitTimer();
	
  return ret;
}

#define tr 
void countrycodestring(unsigned short countrycode, char *string)
{
    switch (countrycode)
    {
    case 0:    /* Demo */
        strcpy(string, ("Demo"));
        break;

    case '7':  /* Beta */
        strcpy(string, ("Beta"));
        break;

    case 0x41: /* Japan / USA */
        strcpy(string, ("USA/Japan"));
        break;

    case 0x44: /* Germany */
        strcpy(string, ("Germany"));
        break;

    case 0x45: /* USA */
        strcpy(string, ("USA"));
        break;

    case 0x46: /* France */
        strcpy(string, ("France"));
        break;

    case 'I':  /* Italy */
        strcpy(string, ("Italy"));
        break;

    case 0x4A: /* Japan */
        strcpy(string, ("Japan"));
        break;

    case 'S':  /* Spain */
        strcpy(string, ("Spain"));
        break;

    case 0x55: case 0x59:  /* Australia */
        sprintf(string, ("Australia (0x%2.2X)"), countrycode);
        break;

    case 0x50: case 0x58: case 0x20:
    case 0x21: case 0x38: case 0x70:
        sprintf(string, ("Europe (0x%02X)"), countrycode);
        break;

    default:
        sprintf(string, ("Unknown (0x%02X)"), countrycode);
        break;
    }
}

char *saveregionstr()
{
    switch (ROM_HEADER.Country_code&0xFF)
    {
    case 0:    /* Demo */
        return "(Demo)";
        break;
    case '7':  /* Beta */
        return "(Beta)";
        break;
    case 0x41: /* Japan / USA */
        return "(JU)";
        break;
    case 0x44: /* Germany */
        return "(G)";
        break;
    case 0x45: /* USA */
        return "(U)";
        break;
    case 0x46: /* France */
        return "(F)";
        break;
    case 'I':  /* Italy */
        return "(I)";
        break;
    case 0x4A: /* Japan */
        return "(J)";
        break;
    case 'S':  /* Spain */
        return "(S)";
        break;
    case 0x55: case 0x59:  /* Australia */
        return "(A)";
        break;
    case 0x50: case 0x58: case 0x20:
    case 0x21: case 0x38: case 0x70:
        return "(E)";
        break;
    default:
        return "(Unk)";
        break;
    }
}

