// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

	Documentation and consistent naming for ROM types.

	This list is intended as a first step to identify and catalog the
	known ROM types on PCBs. Eventually things like access time need
	to be of consideration too.

***************************************************************************/

#ifndef MAME_EMU_DRIVERS_ROMTYPE_H
#define MAME_EMU_DRIVERS_ROMTYPE_H

#pragma once

enum
{
	// 28-pin
	EPROM_27C64   = 0x02000,   //  8k*8
	EPROM_2764    = 0x02000,   //  8k*8
	EPROM_27C128  = 0x04000,   // 16k*8
	EPROM_27C256  = 0x08000,   // 32k*8
	EPROM_27C512  = 0x10000,   // 64k*8

	// 32-pin
	EPROM_27C010  = 0x20000,  // 1 MBit, 128k*8
	EPROM_27C101  = 0x20000,  // 1 MBit, 128k*8
	EPROM_27C1001 = 0x20000,  // 1 MBit, 128k*8
	EPROM_27C2001 = 0x40000,  // 2 MBit, 256k*8
	EPROM_27C040  = 0x80000,  // 4 MBit, 512k*8

	// 40-pin
	EPROM_27C1024 = 0x20000,  // 1 MBit, 64k*16
	EPROM_27C2048 = 0x40000,  // 2 MBit, 128k*16
	EPROM_27C4096 = 0x80000   // 4 MBit, 256k*16
};

#endif // MAME_EMU_DRIVERS_ROMTYPE_H
