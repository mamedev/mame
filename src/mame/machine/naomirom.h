// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NAOMIROM_H
#define MAME_MACHINE_NAOMIROM_H

#include "naomibd.h"

#define MCFG_NAOMI_ROM_BOARD_ADD(_tag, _eeprom_tag, _irq_cb) \
	MCFG_NAOMI_BOARD_ADD(_tag, NAOMI_ROM_BOARD, _eeprom_tag, _irq_cb)

class naomi_rom_board : public naomi_board
{
public:
	naomi_rom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void board_setup_address(uint32_t address, bool is_dma) override;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) override;
	virtual void board_advance(uint32_t size) override;

private:
	uint32_t rom_cur_address;
	required_memory_region m_region;
};

DECLARE_DEVICE_TYPE(NAOMI_ROM_BOARD, naomi_rom_board)

#endif // MAME_MACHINE_NAOMIROM_H
