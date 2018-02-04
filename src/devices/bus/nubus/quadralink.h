// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_NUBUS_QUADRALINK_H
#define MAME_BUS_NUBUS_QUADRALINK_H

#pragma once

#include "nubus.h"
#include "machine/z80scc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_quadralink_device

class nubus_quadralink_device :
		public device_t,
		public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_quadralink_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nubus_quadralink_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<z80scc_device> m_scc1, m_scc2;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_READ32_MEMBER(dev_r);
	DECLARE_WRITE32_MEMBER(dev_w);
};

// device type definition
DECLARE_DEVICE_TYPE(NUBUS_QUADRALINK, nubus_quadralink_device)

#endif // MAME_BUS_NUBUS_QUADRALINK_H
