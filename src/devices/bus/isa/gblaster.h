// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_BUS_ISA_GBLASTER_H
#define MAME_BUS_ISA_GBLASTER_H

#pragma once

#include "isa.h"
#include "sound/saa1099.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_gblaster_device

class isa8_gblaster_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_gblaster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t saa1099_16_r(offs_t offset);
	void saa1099_1_16_w(offs_t offset, uint8_t data);
	void saa1099_2_16_w(offs_t offset, uint8_t data);
	uint8_t detect_r(offs_t offset);
	void detect_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	required_device<saa1099_device> m_saa1099_1;
	required_device<saa1099_device> m_saa1099_2;
	uint8_t detect_reg;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_GAME_BLASTER, isa8_gblaster_device)

#endif // MAME_BUS_ISA_GBLASTER_H
