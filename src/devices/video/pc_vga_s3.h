// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#ifndef MAME_VIDEO_PC_VGA_S3_H
#define MAME_VIDEO_PC_VGA_S3_H

#pragma once

#include "video/pc_vga.h"

#include "screen.h"

class s3vision864_vga_device : public svga_device
{
public:
	// construction/destruction
	s3vision864_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	ibm8514a_device* get_8514() { return m_8514; }

protected:
	s3vision864_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void sequencer_map(address_map &map) override ATTR_COLD;
	virtual uint32_t latch_start_addr() override;

	virtual u16 line_compare_mask() override;

	// TODO: remove this leaky abstraction
	struct
	{
		uint8_t memory_config;
		uint8_t ext_misc_ctrl_2;
		uint8_t crt_reg_lock;
		uint8_t reg_lock1;
		uint8_t reg_lock2;
		uint8_t enable_8514;
		uint8_t enable_s3d;
		uint8_t cr3a;
		uint8_t cr42;
		uint8_t cr43;
		uint8_t cr51;
		uint8_t cr53;
		uint8_t id_high;
		uint8_t id_low;
		uint8_t revision;
		uint8_t id_cr30;
		uint32_t strapping;  // power-on strapping bits
		uint8_t sr10;   // MCLK PLL
		uint8_t sr11;   // MCLK PLL
		uint8_t sr12;   // DCLK PLL
		uint8_t sr13;   // DCLK PLL
		uint8_t sr15;   // CLKSYN control 2
		uint8_t sr17;   // CLKSYN test
		uint8_t clk_pll_r;  // individual DCLK PLL values
		uint8_t clk_pll_m;
		uint8_t clk_pll_n;

		// data for memory-mapped I/O
		uint16_t mmio_9ae8;
		uint16_t mmio_bee8;
		uint16_t mmio_96e8;

		// hardware graphics cursor
		uint8_t cursor_mode;
		uint16_t cursor_x;
		uint16_t cursor_y;
		uint16_t cursor_start_addr;
		uint8_t cursor_pattern_x;  // cursor pattern origin
		uint8_t cursor_pattern_y;
		uint8_t cursor_fg[4];
		uint8_t cursor_bg[4];
		uint8_t cursor_fg_ptr;
		uint8_t cursor_bg_ptr;
		uint8_t extended_dac_ctrl;
	} s3;
	virtual uint16_t offset() override;

	virtual void s3_define_video_mode(void);
	virtual bool get_interlace_mode() override { return BIT(s3.cr42, 5); }

private:
	ibm8514a_device* m_8514;
	void refresh_pitch_offset();
};

class s3vision964_vga_device : public s3vision864_vga_device
{
public:
	s3vision964_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	s3vision964_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
};

class s3vision968_vga_device : public s3vision964_vga_device
{
public:
	s3vision968_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	s3vision968_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
};

class s3trio64_vga_device : public s3vision968_vga_device
{
public:
	s3trio64_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	s3trio64_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(S3_VISION864_VGA, s3vision864_vga_device)
DECLARE_DEVICE_TYPE(S3_VISION964_VGA, s3vision964_vga_device)
DECLARE_DEVICE_TYPE(S3_VISION968_VGA, s3vision968_vga_device)
DECLARE_DEVICE_TYPE(S3_TRIO64_VGA, s3trio64_vga_device)

#endif // MAME_VIDEO_PC_VGA_S3_H
