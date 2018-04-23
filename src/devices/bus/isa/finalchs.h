// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
#ifndef MAME_BUS_ISA_FINALCHS_H
#define MAME_BUS_ISA_FINALCHS_H

#pragma once

#include "isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_finalchs_device

class isa8_finalchs_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_finalchs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	// internal state
	uint8_t m_FCH_latch_data;

	DECLARE_READ8_MEMBER(finalchs_r);
	DECLARE_WRITE8_MEMBER(finalchs_w);

	DECLARE_WRITE8_MEMBER( io7ff8_write );
	DECLARE_READ8_MEMBER( io7ff8_read );
	DECLARE_READ8_MEMBER( io6000_read );
	DECLARE_WRITE8_MEMBER( io6000_write );

	void finalchs_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_FINALCHS, isa8_finalchs_device)

#endif // MAME_BUS_ISA_FINALCHS_H
