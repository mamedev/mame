// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun bwtwo monochrome video controller

***************************************************************************/

#ifndef MAME_BUS_SBUS_BWTWO_H
#define MAME_BUS_SBUS_BWTWO_H

#pragma once

#include "sbus.h"


class sbus_bwtwo_device : public device_t, public device_sbus_card_interface
{
public:
	// construction/destruction
	sbus_bwtwo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_sbus_slot_interface overrides
	virtual void install_device() override;

	uint8_t regs_r(offs_t offset);
	void regs_w(offs_t offset, uint8_t data);
	uint32_t rom_r(offs_t offset);
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) override;

	required_memory_region m_rom;
	std::unique_ptr<uint8_t[]> m_vram;
	required_device<screen_device> m_screen;
	uint32_t m_mono_lut[256][8];
};


DECLARE_DEVICE_TYPE(SBUS_BWTWO, sbus_bwtwo_device)

#endif // MAME_BUS_SBUS_BWTWO_H
