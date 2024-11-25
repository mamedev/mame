// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_NAOMIROM_H
#define MAME_SEGA_NAOMIROM_H

#include "naomibd.h"

class naomi_rom_board : public naomi_board
{
public:
	template <typename T>
	naomi_rom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&eeprom_tag)
		: naomi_rom_board(mconfig, tag, owner, clock)
	{
		eeprom.set_tag(std::forward<T>(eeprom_tag));
	}

	naomi_rom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void board_setup_address(uint32_t address, bool is_dma) override;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) override;
	virtual void board_advance(uint32_t size) override;

private:
	uint32_t rom_cur_address;
	required_memory_region m_region;
};

DECLARE_DEVICE_TYPE(NAOMI_ROM_BOARD, naomi_rom_board)

#endif // MAME_SEGA_NAOMIROM_H
