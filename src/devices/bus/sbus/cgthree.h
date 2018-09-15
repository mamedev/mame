// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun cgthree color video controller

***************************************************************************/

#ifndef MAME_BUS_SBUS_CGTHREE_H
#define MAME_BUS_SBUS_CGTHREE_H

#pragma once

#include "sbus.h"
#include "emupal.h"

class sbus_cgthree_device : public device_t, public device_sbus_card_interface
{
public:
	// construction/destruction
	sbus_cgthree_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_sbus_slot_interface overrides
	virtual void install_device() override;

	void palette_init(palette_device &palette);

	DECLARE_READ32_MEMBER(unknown_r);
	DECLARE_WRITE32_MEMBER(unknown_w);
	DECLARE_WRITE32_MEMBER(palette_w);
	DECLARE_READ8_MEMBER(regs_r);
	DECLARE_WRITE8_MEMBER(regs_w);
	DECLARE_READ32_MEMBER(rom_r);
	DECLARE_READ32_MEMBER(vram_r);
	DECLARE_WRITE32_MEMBER(vram_w);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) override;

	required_memory_region m_rom;
	std::unique_ptr<uint32_t[]> m_vram;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	uint8_t m_palette_entry;
	uint8_t m_palette_r;
	uint8_t m_palette_g;
	uint8_t m_palette_b;
	uint8_t m_palette_step;
};

// device type definition
DECLARE_DEVICE_TYPE(SBUS_CGTHREE, sbus_cgthree_device)

#endif // MAME_BUS_SBUS_CGTHREE_H
