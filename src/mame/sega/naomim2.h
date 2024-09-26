// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_NAOMIM2_H
#define MAME_SEGA_NAOMIM2_H

#pragma once

#include "naomibd.h"
#include "315-5881_crypt.h"


class naomi_m2_board : public naomi_board
{
public:
	template <typename T>
	naomi_m2_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&eeprom_tag)
		: naomi_m2_board(mconfig, tag, owner, clock)
	{
		eeprom.set_tag(std::forward<T>(eeprom_tag));
	}

	naomi_m2_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint32_t rom_cur_address;
	static const int RAM_SIZE = 65536;
	std::unique_ptr<uint8_t[]> ram;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void board_setup_address(uint32_t address, bool is_dma) override;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) override;
	virtual void board_advance(uint32_t size) override;
	virtual void board_write(offs_t offset, uint16_t data) override;

private:
	required_device<sega_315_5881_crypt_device> m_cryptdevice;
	required_memory_region m_region;

	uint16_t read_callback(uint32_t addr);
};

DECLARE_DEVICE_TYPE(NAOMI_M2_BOARD, naomi_m2_board)

#endif // MAME_SEGA_NAOMIM2_H
