/**
 * Wii64 - Recomp-Cache.h
 * Copyright (C) 2008, 2009, 2010 Mike Slegeir
 * 
 * Interface for allocating/freeing blocks of recompiled code
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: tehpola@gmail.com
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

#ifndef RECOMP_CACHE_H
#define RECOMP_CACHE_H

#ifdef HW_RVL
#ifdef GLN64_GX  // for glN64 GFX on Wii this is not in MEM2 so 10MB is unstable...
#define RECOMP_CACHE_SIZE (8*1024*1024)	// glN64 Wii 
#else  // for Rice GFX on Wii this is not in MEM2 so 10MB is unstable...
#define RECOMP_CACHE_SIZE (8*1024*1024)		// Rice Wii
#endif
#else
# ifdef USE_EXPANSION
#  ifdef GLN64_GX
#   define RECOMP_CACHE_SIZE (7*1024*1024) //7 MB for GC with expansion pak + glN64
#  else
#   define RECOMP_CACHE_SIZE (6*1024*1024) //6 MB for GC with expansion pak + Rice
#  endif
# else
#  define RECOMP_CACHE_SIZE (8*1024*1024) //8 MB for GC
# endif
#endif
#include "ppc/Recompile.h"

void RecompCache_Init(void);
// Allocate and free memory to be used for recompiled code
//   Any memory allocated this way can be freed at any time
//   you must check invalid_code before you can access it
void RecompCache_Alloc(unsigned int size, unsigned int address, PowerPC_func* func);
void RecompCache_Realloc(PowerPC_func* func, unsigned int new_size);
void RecompCache_Free(unsigned int addr);
#ifdef HW_DOL
void RecompCache_Release(int bytesRequired);
#endif
// Update the LRU info of the indicated block
//   (call when the block is accessed)
void RecompCache_Update(PowerPC_func* func);
void RecompCache_Link(PowerPC_func* src_func, PowerPC_instr* src_instr,
                      PowerPC_func* dst_func, PowerPC_instr* dst_instr);

// Allocate memory from the meta cache
//   This will free from both the recomp and meta caches if capacity is hit
void* MetaCache_Alloc(unsigned int num_bytes);
// Free data from the meta cache
void MetaCache_Free(void* ptr);

#endif
