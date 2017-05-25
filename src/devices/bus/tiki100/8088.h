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

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_tiki100bus_card_interface overrides
	virtual uint8_t iorq_r(address_space &space, offs_t offset, uint8_t data) override;
	virtual void iorq_w(address_space &space, offs_t offset, uint8_t data) override;

private:
	required_device<i8088_cpu_device> m_maincpu;

	uint8_t m_data;
};


// device type definition
DECLARE_DEVICE_TYPE(TIKI100_8088, tiki100_8088_device)

#endif // MAME_BUS_TIKI100_8088_H
