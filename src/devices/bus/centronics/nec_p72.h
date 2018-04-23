// license:BSD-3-Clause
// copyright-holders:Ramiro Polla

#ifndef MAME_BUS_CENTRONICS_NEC_P72_H
#define MAME_BUS_CENTRONICS_NEC_P72_H

#pragma once

#include "ctronics.h"
#include "cpu/nec/nec.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nec_p72_device

class nec_p72_device : public device_t,
					public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	nec_p72_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nec_p72_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cpu_device> m_maincpu;

	void p72_mem(address_map &map);
};

// device type definition
DECLARE_DEVICE_TYPE(NEC_P72, nec_p72_device)

#endif // MAME_BUS_CENTRONICS_NEC_P72_H
