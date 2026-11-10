// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_BUS_ISA_PROGA_H
#define MAME_BUS_ISA_PROGA_H

#pragma once

#include "isa.h"
#include "video/hd63484.h"

#include "emupal.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_proga_device

class isa8_proga_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_proga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	void videoram_map(address_map &map) ATTR_COLD;
	void palette_init(palette_device &palette) const ATTR_COLD;

	uint8_t acrtc_r(offs_t offset);
	void acrtc_w(offs_t offset, uint8_t data);

	required_device<hd63484_device> m_acrtc;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_PROGA, isa8_proga_device)

#endif // MAME_BUS_ISA_PROGA_H
