// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_PLATINUM_H
#define MAME_APPLE_PLATINUM_H

#pragma once

#include "dafb.h"

#include "machine/ram.h"

class platinum_device : public dafb_base
{
public:
	platinum_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void map(address_map &map) ATTR_COLD;

	auto irq_cb() { return dafb_irq(); }

	void set_maincpu_space(address_space *space) { m_maincpu_space = space; }
	template <typename... T> void set_ram_tag(T &&... args) { m_ram.set_tag(std::forward<T>(args)...); }


	u32 platinum_r(offs_t offset);
	void platinum_w(offs_t offset, u32 data);
	virtual u32 dafb_r(offs_t offset) override;
	virtual void dafb_w(offs_t offset, u32 data) override;
	u32 dacula_r(offs_t offset, u32 mem_mask);
	void dacula_w(offs_t offset, u32 data, u32 mem_mask);
	u32 accel_r(offs_t offset, u32 mem_mask);
	void accel_w(offs_t offset, u32 data, u32 mem_mask);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual int clock_divider() const override;

private:
	enum
	{
		ACCEL_FGCOLOR = 0,      // Foreground Color
		ACCEL_BGCOLOR,          // Background Color
		ACCEL_SRCBASE,          // Source Starting Address
		ACCEL_DSTBASE,          // Destination Starting Address
		ACCEL_ROWBYTES,         // Destination/Source Row Bytes
		ACCEL_DSTSIZE,          // Destination Vertical/Horizontal Size
		ACCEL_CNTL,             // Control Register 1
		ACCEL_CMD,              // Control Register 2
		ACCEL_STATUS,           // Command/Status Register
		ACCEL_PTR,              // Command List Pointer (not implemented in Platinum)
		ACCEL_SYSCONFIG,        // Iridium System Configuration
		ACCEL_SLEEP,            // Sleep Mode

		ACCEL_MAX_REG
	};

	enum
	{
		DAC_ADDR_LATCH = 0,
		DAC_CURSOR_CLUT,
		DAC_CONTROL_LATCH,
		DAC_DATA_LATCH,

		DAC_CURSOR_POS_H = 0x10,
		DAC_CURSOR_POS_L,

		DAC_CONTROL = 0x20,
		DAC_BUFFER_CONTROL,
		DAC_TEST_CONTROL,
		DAC_PLL_CONTROL,
		DAC_VCLK_M_A,
		DAC_VCLK_PN_A,
		DAC_UNUSED_VCLK,
		DAC_VCLK_M_B,
		DAC_VCLK_PN_B,
		DAC_UNKNOWN,

		DAC_ID = 0x40,

		DAC_MAX_REG = DAC_ID + 1
	};

	emu_timer *m_accel_timer;

	u32 m_mem_regs[0x10];
	u32 m_dacula_regs[DAC_MAX_REG];

	address_space *m_maincpu_space;
	required_device<ram_device> m_ram;
	u32 m_accel_regs[ACCEL_MAX_REG];
	u32 m_accel_status;

	u32 m_cursor_clut[8];
	u8 m_cursor_rgb[3];

	void recalc_clock();
	void draw_hw_cursor(bitmap_rgb32 &bitmap, u32 fb_base, u32 stride);

	void accel_execute();
	u32 accel_read_pixel(u32 addr, int bpp);
	void accel_write_pixel(u32 addr, int bpp, u32 data);
	u32 accel_apply_rop(u32 rop, bool colorize, int depth_sel, u32 src, u32 dst, u32 fg, u32 bg);
	u32 accel_apply_rop_expand(u32 rop, u32 src, u32 dst, u32 fg, u32 bg);

	TIMER_CALLBACK_MEMBER(accel_done);
};

DECLARE_DEVICE_TYPE(PLATINUM, platinum_device)

#endif  /* MAME_APPLE_PLATINUM_H */
