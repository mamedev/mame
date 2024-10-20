// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_BUS_ISA_ADLIB_H
#define MAME_BUS_ISA_ADLIB_H

#pragma once

#include "isa.h"
#include "sound/ymopl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_adlib_device

class isa8_adlib_device :
	public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_adlib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ym3812_16_r(offs_t offset);
	void ym3812_16_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	required_device<ym3812_device> m_ym3812;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_ADLIB, isa8_adlib_device)

#endif // MAME_BUS_ISA_ADLIB_H
