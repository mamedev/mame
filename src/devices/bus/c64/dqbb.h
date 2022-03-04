// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Brown Boxes Double Quick Brown Box emulation

**********************************************************************/

#ifndef MAME_BUS_C64_DQBB_H
#define MAME_BUS_C64_DQBB_H

#pragma once


#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_dqbb_cartridge_device

class c64_dqbb_cartridge_device : public device_t,
									public device_c64_expansion_card_interface,
									public device_nvram_interface
{
public:
	// construction/destruction
	c64_dqbb_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override { }
	virtual bool nvram_read(util::read_stream &file) override { size_t actual; return !file.read(m_nvram.get(), 0x4000, actual) && actual == 0x4000; }
	virtual bool nvram_write(util::write_stream &file) override { size_t actual; return !file.write(m_nvram.get(), 0x4000, actual) && actual == 0x4000; }

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	int m_cs;
	int m_we;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_DQBB, c64_dqbb_cartridge_device)


#endif // MAME_BUS_C64_DQBB_H
