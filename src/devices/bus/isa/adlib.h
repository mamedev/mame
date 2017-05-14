// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_BUS_ISA_ADLIB_H
#define MAME_BUS_ISA_ADLIB_H

#pragma once

#include "isa.h"
#include "sound/3812intf.h"

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

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;

		DECLARE_READ8_MEMBER(ym3812_16_r);
		DECLARE_WRITE8_MEMBER(ym3812_16_w);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
private:
		// internal state
		required_device<ym3812_device> m_ym3812;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_ADLIB, isa8_adlib_device)

#endif // MAME_BUS_ISA_ADLIB_H
