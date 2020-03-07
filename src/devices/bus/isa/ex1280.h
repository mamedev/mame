// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_BUS_ISA_EX1280_H
#define MAME_BUS_ISA_EX1280_H

#pragma once

#include "isa.h"
#include "cpu/tms34010/tms34010.h"
#include "video/bt45x.h"
#include "screen.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_ex1280_device

class isa16_ex1280_device : public device_t,
	public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_ex1280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Flag non working features
	static constexpr feature_type unemulated_features() { return feature::GRAPHICS | feature::PALETTE; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	DECLARE_READ16_MEMBER(vram_r);
	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_READ16_MEMBER(regs_r);
	DECLARE_WRITE16_MEMBER(regs_w);
	DECLARE_WRITE_LINE_MEMBER(vblank_w);

	required_device<tms34010_device> m_cpu;
	required_device<bt451_device> m_ramdac;
	required_device<screen_device> m_screen;

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);

	void main_map(address_map &map);

	std::vector<uint16_t> m_vram;
	uint16_t m_flags;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA16_EX1280, isa16_ex1280_device)

#endif // MAME_BUS_ISA_EX1280_H
