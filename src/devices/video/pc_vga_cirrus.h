// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Cirrus Logic GD542x/3x video chipsets

*/
#ifndef MAME_VIDEO_PC_VGA_CIRRUS_H
#define MAME_VIDEO_PC_VGA_CIRRUS_H

#pragma once

#include "video/pc_vga.h"

class cirrus_gd5428_vga_device :  public svga_device
{
public:
	// construction/destruction
	cirrus_gd5428_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	cirrus_gd5428_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint16_t offset() override;
	virtual uint32_t latch_start_addr() override;

	virtual void io_3cx_map(address_map &map) override ATTR_COLD;

	u8 ramdac_hidden_mask_r(offs_t offset);
	void ramdac_hidden_mask_w(offs_t offset, u8 data);
	u8 ramdac_overlay_r(offs_t offset);
	void ramdac_overlay_w(offs_t offset, u8 data);
	u8 m_hidden_dac_mode = 0;
	u8 m_hidden_dac_phase = 0;
	uint8_t m_chip_id;

	uint8_t gc_mode_ext = 0;
	uint8_t gc_bank[2]{};
	bool gc_locked = false;
	uint8_t m_lock_reg = 0;
	uint8_t m_gr10 = 0;  // high byte of background colour (in 15/16bpp)
	uint8_t m_gr11 = 0;  // high byte of foreground colour (in 15/16bpp)

	uint8_t m_cr19 = 0;
	uint8_t m_cr1a = 0;
	uint8_t m_cr1b = 0;

	// hardware cursor
	uint16_t m_cursor_x = 0;
	uint16_t m_cursor_y = 0;
	uint16_t m_cursor_addr = 0;
	uint8_t m_cursor_attr = 0;
	bool m_ext_palette_enabled = false;
	struct { uint8_t red, green, blue; } m_ext_palette[16];  // extra palette, colour 0 is cursor background, colour 15 is cursor foreground, colour 2 is overscan border colour

	// BitBLT engine
	uint8_t m_blt_status = 0;
	uint8_t m_blt_rop = 0;
	uint8_t m_blt_mode = 0;
	uint32_t m_blt_source = 0;
	uint32_t m_blt_dest = 0;
	uint16_t m_blt_source_pitch = 0;
	uint16_t m_blt_dest_pitch = 0;
	uint16_t m_blt_height = 0;
	uint16_t m_blt_width = 0;
	uint32_t m_blt_source_current = 0;
	uint32_t m_blt_dest_current = 0;
	uint16_t m_blt_trans_colour = 0;
	uint16_t m_blt_trans_colour_mask = 0;

	bool m_blt_system_transfer = false;  // blit from system memory
	uint8_t m_blt_system_count = 0;
	uint32_t m_blt_system_buffer = 0;
	uint16_t m_blt_pixel_count = 0;
	uint16_t m_blt_scan_count = 0;

	uint8_t m_scratchpad1 = 0;
	uint8_t m_scratchpad2 = 0;
	uint8_t m_scratchpad3 = 0;
	uint8_t m_vclk_num[4]{};
	uint8_t m_vclk_denom[4]{};

	virtual uint8_t vga_latch_write(int offs, uint8_t data) override;

	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void gc_map(address_map &map) override ATTR_COLD;
	virtual void sequencer_map(address_map &map) override ATTR_COLD;

	virtual bool get_interlace_mode() override { return BIT(m_cr1a, 0); }

	uint8_t offset_select(offs_t offset);

private:
	void cirrus_define_video_mode();

	void start_bitblt();
	void start_reverse_bitblt();
	void start_system_bitblt();
	void blit_dword();
	void blit_byte();  // used for colour expanded system-to-vram bitblts
	void copy_pixel(uint8_t src, uint8_t dst);
};

class cirrus_gd5430_vga_device :  public cirrus_gd5428_vga_device
{
public:
	cirrus_gd5430_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	cirrus_gd5430_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void gc_map(address_map &map) override ATTR_COLD;
	virtual void sequencer_map(address_map &map) override ATTR_COLD;
private:
	uint8_t m_cr1d = 0;
};

class cirrus_gd5446_vga_device :  public cirrus_gd5430_vga_device
{
public:
	cirrus_gd5446_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void gc_map(address_map &map) override ATTR_COLD;
	virtual void sequencer_map(address_map &map) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(CIRRUS_GD5428_VGA, cirrus_gd5428_vga_device)
DECLARE_DEVICE_TYPE(CIRRUS_GD5430_VGA, cirrus_gd5430_vga_device)
DECLARE_DEVICE_TYPE(CIRRUS_GD5446_VGA, cirrus_gd5446_vga_device)

#endif // MAME_VIDEO_PC_VGA_CIRRUS_H
