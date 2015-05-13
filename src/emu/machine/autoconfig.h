// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Autoconfig

***************************************************************************/

#pragma once

#ifndef __AUTOCONFIG_H__
#define __AUTOCONFIG_H__

#include "emu.h"

class amiga_autoconfig
{
public:
	amiga_autoconfig();
	virtual ~amiga_autoconfig();

	// read from autoconfig space
	DECLARE_READ16_MEMBER( autoconfig_read );

	// write to autoconfig space
	DECLARE_WRITE16_MEMBER( autoconfig_write );

protected:
	enum board_type
	{
		BOARD_TYPE_ZORRO3 = 2,
		BOARD_TYPE_ZORRO2 = 3
	};

	enum board_size
	{
		BOARD_SIZE_8M = 0,
		BOARD_SIZE_64K = 1,
		BOARD_SIZE_128K = 2,
		BOARD_SIZE_256K = 3,
		BOARD_SIZE_512K = 4,
		BOARD_SIZE_1M = 5,
		BOARD_SIZE_2M = 6,
		BOARD_SIZE_4M = 7
	};

	// board type & size
	void autoconfig_board_type(board_type type);
	void autoconfig_board_size(board_size size);

	// various flags
	void autoconfig_rom_vector_valid(bool state);
	void autoconfig_link_into_memory(bool state);
	void autoconfig_multi_device(bool state);
	void autoconfig_8meg_preferred(bool state);
	void autoconfig_can_shutup(bool state);

	// product number, manufacturer number, serial number
	void autoconfig_product(UINT8 data);
	void autoconfig_manufacturer(UINT16 data);
	void autoconfig_serial(UINT32 data);

	// rom vector
	void autoconfig_rom_vector(UINT16 data);

	// called once we have received a valid base address from the host system
	virtual void autoconfig_base_address(offs_t address) = 0;

private:
	// configuration information about our autoconfig board, 256 nibbles
	UINT16 m_cfg[0x40];
};

#endif // __AUTOCONFIG_H__
