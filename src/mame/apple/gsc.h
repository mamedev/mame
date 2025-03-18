// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_GSC_H
#define MAME_APPLE_GSC_H

#pragma once

#include "emupal.h"
#include "screen.h"

class gsc_device : public device_t
{
public:
	gsc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~gsc_device() = default;

	void map(address_map &map) ATTR_COLD;

	void set_panel_id(int panel_id);
	void set_pmu_blank(bool blank) { m_pmu_blank_display = blank; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	std::unique_ptr<u32[]> m_vram;

	u8 m_gsc_regs[0x20];
	u8 m_gsc_panel_id;
	bool m_pmu_blank_display;

	u32 screen_update_gsc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 gsc_r(offs_t offset);
	void gsc_w(offs_t offset, u8 data);
	u32 vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask);

	void macgsc_palette(palette_device &palette) const;
};

DECLARE_DEVICE_TYPE(GSC, gsc_device)

#endif  /* MAME_APPLE_GSC_H */
