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
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override { }
	virtual bool nvram_read(util::read_stream &file) override { size_t actual; return !file.read(m_nvram, 0x2000, actual) && actual == 0x2000; }
	virtual bool nvram_write(util::write_stream &file) override { size_t actual; return !file.write(m_nvram, 0x2000, actual) && actual == 0x2000; }

	// device_vic20_expansion_card_interface overrides
	virtual uint8_t vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

private:
	memory_share_creator<uint8_t> m_nvram;
	int m_nvram_en;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_MEGACART, vic20_megacart_device)

#endif // MAME_BUS_VIC20_MEGACART_H
