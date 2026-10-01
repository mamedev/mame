// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mega-Cart cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_MEGACART_H
#define MAME_BUS_VIC20_MEGACART_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_megacart_device

class vic20_megacart_device :  public device_t,
								public device_vic20_expansion_card_interface,
								public device_nvram_interface
{
public:
	// construction/destruction
	vic20_megacart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	TIMER_CALLBACK_MEMBER(reset_tick);

	void io3_w(offs_t offset, uint8_t data);
	void update_map();

	memory_share_creator<uint8_t> m_ram;
	memory_share_creator<uint8_t> m_nvram;
	uint8_t *m_rom;
	emu_timer *m_reset_timer;

	int m_nvram_en;
	int m_oe;
	int m_software_reset;
	uint8_t m_bank_lo;
	uint8_t m_bank_hi;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_MEGACART, vic20_megacart_device)

#endif // MAME_BUS_VIC20_MEGACART_H
