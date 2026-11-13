// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_CIVIC_H
#define MAME_APPLE_CIVIC_H

#pragma once

#include "emupal.h"
#include "screen.h"

#include "machine/icd2053b.h"

class civic_device : public device_t
{
public:
	civic_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
	civic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual ~civic_device() = default;

	void map(address_map &map) ATTR_COLD;

	u32 vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask);

	void use_icd_clockgen() { m_is_clifton = true; }

	auto vblank_irq() { return m_irq.bind(); }

	u32 civic_r(offs_t offset);
	void civic_w(offs_t offset, u32 data);
	virtual u32 ramdac_r(offs_t offset);
	virtual void ramdac_w(offs_t offset, u32 data);
	virtual u8 clockgen_r(offs_t offset);
	virtual void clockgen_w(offs_t offset, u8 data);

	void clock_select_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void recalc_ints();
	void recalc_mode();
	void pclock_w(u32 new_clock);

	u32 m_vram_size;
	u32 m_pixel_clock;

	bool m_is_clifton;

private:
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<icd2053b_device> m_clockgen;
	required_ioport m_monitor_config;
	devcb_write_line m_irq;

	u32 m_register_base[0x700/4];
	u32 m_register_shift[0x700/4];

	std::unique_ptr<u32[]> m_vram;
	emu_timer *m_vbl_timer;
	u8 m_monitor_id;
	u8 m_pal_address, m_pal_idx, m_sebastian_ctrl;
	u32 m_base, m_stride;
	s32 m_int_status;
	u32 m_hres, m_vres, m_htotal, m_vtotal;
	u32 m_regs[0x40];

	// Endeavor clock synthesizer parameters (Q840AV)
	u32 m_M, m_N, m_CLK;
	int m_clocksel;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(vbl_tick);
};

DECLARE_DEVICE_TYPE(CIVIC, civic_device)

#endif  /* MAME_APPLE_CIVIC_H */
