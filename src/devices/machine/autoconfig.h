// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Autoconfig

***************************************************************************/

#ifndef MAME_MACHINE_AUTOCONFIG_H
#define MAME_MACHINE_AUTOCONFIG_H

#pragma once


class amiga_autoconfig
{
public:
	amiga_autoconfig();
	virtual ~amiga_autoconfig();

	// read from autoconfig space
	uint16_t autoconfig_read(address_space &space, offs_t offset, uint16_t mem_mask = ~0);

	// write to autoconfig space
	void autoconfig_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

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
	void autoconfig_product(uint8_t data);
	void autoconfig_manufacturer(uint16_t data);
	void autoconfig_serial(uint32_t data);

	// rom vector
	void autoconfig_rom_vector(uint16_t data);

	// called once we have received a valid base address from the host system
	virtual void autoconfig_base_address(offs_t address) = 0;

private:
	// configuration information about our autoconfig board, 256 nibbles
	uint16_t m_cfg[0x40];
};

#endif // MAME_MACHINE_AUTOCONFIG_H
