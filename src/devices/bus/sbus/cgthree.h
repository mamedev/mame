// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun cgthree color video controller

***************************************************************************/

#ifndef MAME_BUS_SBUS_CGTHREE_H
#define MAME_BUS_SBUS_CGTHREE_H

#pragma once

#include "sbus.h"
#include "video/bt45x.h"

class sbus_cgthree_device : public device_t, public device_sbus_card_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	sbus_cgthree_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sbus_slot_interface overrides
	virtual void install_device() override;

	uint32_t unknown_r(offs_t offset, uint32_t mem_mask = ~0);
	void unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t regs_r(offs_t offset);
	void regs_w(offs_t offset, uint8_t data);
	uint32_t rom_r(offs_t offset);
	uint32_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) override;

	required_memory_region m_rom;
	std::unique_ptr<uint32_t[]> m_vram;
	required_device<screen_device> m_screen;
	required_device<bt458_device> m_ramdac;
};


DECLARE_DEVICE_TYPE(SBUS_CGTHREE, sbus_cgthree_device)

#endif // MAME_BUS_SBUS_CGTHREE_H
