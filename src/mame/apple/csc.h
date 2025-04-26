// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_CSC_H
#define MAME_APPLE_CSC_H

#pragma once

#include "emupal.h"
#include "screen.h"

class csc_device : public device_t
{
public:
	csc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~csc_device() = default;

	auto write_irq() { return m_irq.bind(); }

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
	devcb_write_line m_irq;

	std::unique_ptr<u32[]> m_vram;

	u8 m_csc_regs[0x50];
	u8 m_csc_panel_id;
	int m_pal_idx;
	bool m_pmu_blank_display;

	u32 screen_update_csc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 csc_r(offs_t offset);
	void csc_w(offs_t offset, u8 data);
	void csc_irq_w(int state);

	u32 vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask);
};

DECLARE_DEVICE_TYPE(CSC, csc_device)

#endif  /* MAME_APPLE_CSC_H */
