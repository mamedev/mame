// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_ISA_NUM9REV_H
#define MAME_BUS_ISA_NUM9REV_H

#pragma once

#include "isa.h"
#include "video/upd7220.h"
#include "machine/bankdev.h"
#include "emupal.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_vga_device

class isa8_number_9_rev_device :
	public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_number_9_rev_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	UPD7220_DISPLAY_PIXELS_MEMBER(hgdc_display_pixels);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t pal8_r(offs_t offset);
	void pal8_w(offs_t offset, uint8_t data);
	uint8_t pal12_r(offs_t offset);
	void pal12_w(offs_t offset, uint8_t data);
	uint8_t overlay_r(offs_t offset);
	void overlay_w(offs_t offset, uint8_t data);
	uint8_t bank_r();
	void bank_w(uint8_t data);
	uint8_t ctrl_r(offs_t offset);
	void ctrl_w(offs_t offset, uint8_t data);
	uint8_t read8(offs_t offset);
	void write8(offs_t offset, uint8_t data);

	void upd7220_map(address_map &map) ATTR_COLD;

	required_device<upd7220_device> m_upd7220;
	required_device<palette_device> m_palette;
	std::vector<uint8_t> m_ram;
	std::vector<uint8_t> m_overlay;

	uint8_t m_bank;
	uint8_t m_mode;
	bool m_1024;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_NUM_9_REV, isa8_number_9_rev_device)

#endif // MAME_BUS_ISA_NUM9REV_H
