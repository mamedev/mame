// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII RAM PROM loading and decoding
 *
 *****************************************************************************/
#ifndef _CPU_A2ROMS_H_
#define _CPU_A2ROMS_H_

#include "emu.h"

/**
 * @brief structure to define a ROM's or PROM's loading options
 */
typedef struct {
	const char *name;       //!< default filename of the ROM image
	const char *altname;    //!< alternate filename of the ROM image
	const char *crc32;      //!< CRC32 hash of the file
	const char *sha1;       //!< SHA1 hash of the file
	size_t size;            //!< size of the file, and elements in destination memory
	const UINT8 amap[16];   //!< address bit mapping
	UINT32 axor;            //!< address XOR mask (applied to source address)
	UINT32 dxor;            //!< data XOR mask (applied before shifting and mapping)
	UINT8 width;            //!< width in bits
	UINT8 shift;            //!< left shift in bits
	const UINT8 dmap[16];   //!< data bit mapping
	UINT32 dand;            //!< ANDing destination with this value, before XORing the data
	size_t type;            //!< type of the destination, i.e. sizeof(type)
}   prom_load_t;

#define ZERO            0
#define KEEP            ~0U

#define AMAP_DEFAULT        {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
#define AMAP_CONST_PROM     {3,2,1,4,5,6,7,0,}
#define AMAP_REVERSE_0_7    {7,6,5,4,3,2,1,0,}

#define DMAP_DEFAULT        {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
#define DMAP_REVERSE_0_3    {3,2,1,0,}

extern UINT8* prom_load(running_machine& machine, const prom_load_t* prom, const UINT8* src, int pages = 1, int segments = 1);
#endif // _CPU_A2ROMS_H_
