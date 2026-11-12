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

	uint16_t autoconfig_read(address_space &space, offs_t offset, uint16_t mem_mask);
	void autoconfig_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);

	// zorro3 autoconfig
	uint32_t autoconfig_read32(address_space &space, offs_t offset, uint32_t mem_mask);
	void autoconfig_write32(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);

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
		BOARD_SIZE_4M = 7,
		// zorro3 additions follow
		BOARD_SIZE_16M = 8,
		BOARD_SIZE_32M = 9,
		BOARD_SIZE_64M = 10,
		BOARD_SIZE_128M = 11,
		BOARD_SIZE_256M = 12,
		BOARD_SIZE_512M = 13,
		BOARD_SIZE_1G = 14
	};

	// zorro3 only
	enum board_subsize
	{
		BOARD_SUBSIZE_SAME = 0,
		BOARD_SUBSIZE_AUTOSIZE = 1,
		BOARD_SUBSIZE_64K = 2,
		BOARD_SUBSIZE_128K = 3,
		BOARD_SUBSIZE_256K = 4,
		BOARD_SUBSIZE_512K = 5,
		BOARD_SUBSIZE_1M = 6,
		BOARD_SUBSIZE_2M = 7,
		BOARD_SUBSIZE_4M = 8,
		BOARD_SUBSIZE_6M = 9,
		BOARD_SUBSIZE_8M = 10,
		BOARD_SUBSIZE_10M = 11,
		BOARD_SUBSIZE_12M = 12,
		BOARD_SUBSIZE_14M = 13
	};

	// board type & size
	void autoconfig_board_type(board_type type);
	void autoconfig_board_size(board_size size);
	void autoconfig_board_subsize(board_subsize size);

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
	bool is_zorro3() const { return (m_cfg[0x00] >> 2) == BOARD_TYPE_ZORRO3; }

	// configuration information about our autoconfig board
	uint8_t m_cfg[0x40];
};

#endif // MAME_MACHINE_AUTOCONFIG_H
