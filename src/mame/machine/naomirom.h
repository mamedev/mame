// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef _NAOMIROM_H_
#define _NAOMIROM_H_

#include "naomibd.h"

#define MCFG_NAOMI_ROM_BOARD_ADD(_tag, _eeprom_tag, _actel_tag, _irq_cb) \
	MCFG_NAOMI_BOARD_ADD(_tag, NAOMI_ROM_BOARD, _eeprom_tag, _actel_tag, _irq_cb)

class naomi_rom_board : public naomi_board
{
public:
	naomi_rom_board(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void board_setup_address(UINT32 address, bool is_dma) override;
	virtual void board_get_buffer(UINT8 *&base, UINT32 &limit) override;
	virtual void board_advance(UINT32 size) override;

private:
	UINT32 rom_cur_address;
};

extern const device_type NAOMI_ROM_BOARD;

#endif
