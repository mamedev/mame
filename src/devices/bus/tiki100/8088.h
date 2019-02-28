// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 8/16 8088/8087 expansion card emulation

**********************************************************************/

#ifndef MAME_BUS_TIKI100_8088_H
#define MAME_BUS_TIKI100_8088_H

#pragma once

#include "bus/tiki100/exp.h"
#include "cpu/i86/i86.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tiki100_8088_device

class tiki100_8088_device : public device_t,
						public device_tiki100bus_card_interface
{
public:
	// construction/destruction
	tiki100_8088_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_tiki100bus_card_interface overrides
	virtual uint8_t iorq_r(offs_t offset, uint8_t data) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

private:
	required_device<i8088_cpu_device> m_maincpu;

	uint8_t m_data;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	void i8088_io(address_map &map);
	void i8088_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(TIKI100_8088, tiki100_8088_device)

#endif // MAME_BUS_TIKI100_8088_H
