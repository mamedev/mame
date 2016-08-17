// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    cbm_crt.h

    Commodore C64 cartridge images

*********************************************************************/

#pragma once

#ifndef __CBM_CRT__
#define __CBM_CRT__

#include "formats/imageutl.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define CRT_SIGNATURE       "C64 CARTRIDGE   "

#define CRT_HEADER_LENGTH   0x40
#define CRT_CHIP_LENGTH     0x10

#define UNSUPPORTED         "standard"


// C64 cartridge types
enum
{
	CRT_C64_STANDARD = 0,
	CRT_C64_ACTION_REPLAY,
	CRT_C64_KCS_POWER,
	CRT_C64_FINAL_III,
	CRT_C64_SIMONS_BASIC,
	CRT_C64_OCEAN,
	CRT_C64_EXPERT,
	CRT_C64_FUNPLAY,
	CRT_C64_SUPER_GAMES,
	CRT_C64_ATOMIC_POWER,
	CRT_C64_EPYX_FASTLOAD,
	CRT_C64_WESTERMANN,
	CRT_C64_REX,
	CRT_C64_FINAL_I,
	CRT_C64_MAGIC_FORMEL,
	CRT_C64_GS,
	CRT_C64_WARPSPEED,
	CRT_C64_DINAMIC,
	CRT_C64_ZAXXON,
	CRT_C64_MAGIC_DESK,
	CRT_C64_SUPER_SNAPSHOT_V5,
	CRT_C64_COMAL80,
	CRT_C64_STRUCTURED_BASIC,
	CRT_C64_ROSS,
	CRT_C64_DELA_EP64,
	CRT_C64_DELA_EP7x8,
	CRT_C64_DELA_EP256,
	CRT_C64_REX_EP256,
	CRT_C64_MIKRO_ASSEMBLER,
	CRT_C64_FINAL_PLUS,
	CRT_C64_ACTION_REPLAY4,
	CRT_C64_STARDOS,
	CRT_C64_EASYFLASH,
	CRT_C64_EASYFLASH_XBANK,
	CRT_C64_CAPTURE,
	CRT_C64_ACTION_REPLAY3,
	CRT_C64_RETRO_REPLAY,
	CRT_C64_MMC64,
	CRT_C64_MMC_REPLAY,
	CRT_C64_IDE64,
	CRT_C64_SUPER_SNAPSHOT,
	CRT_C64_IEEE488,
	CRT_C64_GAME_KILLER,
	CRT_C64_P64,
	CRT_C64_EXOS,
	CRT_C64_FREEZE_FRAME,
	CRT_C64_FREEZE_MACHINE,
	CRT_C64_SNAPSHOT64,
	CRT_C64_SUPER_EXPLODE_V5,
	CRT_C64_MAGIC_VOICE,
	CRT_C64_ACTION_REPLAY2,
	CRT_C64_MACH5,
	CRT_C64_DIASHOW_MAKER,
	CRT_C64_PAGEFOX,
	CRT_C64_KINGSOFT,
	CRT_C64_SILVERROCK,
	_CRT_C64_COUNT
};


// chip types
enum
{
	CRT_CHIP_TYPE_ROM = 0,
	CRT_CHIP_TYPE_RAM,
	CRT_CHIP_TYPE_FLASH_ROM
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct cbm_crt_header
{
	UINT8 signature[16];
	UINT8 header_length[4];
	UINT8 version[2];
	UINT8 hardware[2];
	UINT8 exrom;
	UINT8 game;
	UINT8 reserved[6];
	UINT8 name[32];
};


struct cbm_crt_chip
{
	UINT8 signature[4];
	UINT8 packet_length[4];
	UINT8 chip_type[2];
	UINT8 bank[2];
	UINT8 start_address[2];
	UINT8 image_size[2];
};



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

std::string cbm_crt_get_card(util::core_file &file);
bool cbm_crt_read_header(util::core_file &file, size_t *roml_size, size_t *romh_size, int *exrom, int *game);
bool cbm_crt_read_data(util::core_file &file, UINT8 *roml, UINT8 *romh);


#endif
