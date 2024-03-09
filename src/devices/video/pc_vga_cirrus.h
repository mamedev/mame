// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Cirrus Logic GD542x/3x video chipsets

*/
#ifndef MAME_VIDEO_PC_VGA_CIRRUS_H
#define MAME_VIDEO_PC_VGA_CIRRUS_H

#pragma once

#include "video/pc_vga.h"

class cirrus_gd5428_device :  public svga_device
{
public:
	// construction/destruction
	cirrus_gd5428_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	cirrus_gd5428_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint16_t offset() override;

	virtual void io_3cx_map(address_map &map) override;

	u8 ramdac_hidden_mask_r(offs_t offset);
	void ramdac_hidden_mask_w(offs_t offset, u8 data);
	u8 ramdac_overlay_r(offs_t offset);
	void ramdac_overlay_w(offs_t offset, u8 data);
	u8 m_hidden_dac_mode = 0;
	u8 m_hidden_dac_phase = 0;
	uint8_t m_chip_id;

	uint8_t gc_mode_ext;
	uint8_t gc_bank_0;
	uint8_t gc_bank_1;
	bool gc_locked;
	uint8_t m_lock_reg;
	uint8_t m_gr10;  // high byte of background colour (in 15/16bpp)
	uint8_t m_gr11;  // high byte of foreground colour (in 15/16bpp)

	uint8_t m_cr19;
	uint8_t m_cr1a;
	uint8_t m_cr1b;

	// hardware cursor
	uint16_t m_cursor_x;
	uint16_t m_cursor_y;
	uint16_t m_cursor_addr;
	uint8_t m_cursor_attr;
	bool m_ext_palette_enabled;
	struct { uint8_t red, green, blue; } m_ext_palette[16];  // extra palette, colour 0 is cursor background, colour 15 is cursor foreground, colour 2 is overscan border colour

	// BitBLT engine
	uint8_t m_blt_status;
	uint8_t m_blt_rop;
	uint8_t m_blt_mode;
	uint32_t m_blt_source;
	uint32_t m_blt_dest;
	uint16_t m_blt_source_pitch;
	uint16_t m_blt_dest_pitch;
	uint16_t m_blt_height;
	uint16_t m_blt_width;
	uint32_t m_blt_source_current;
	uint32_t m_blt_dest_current;
	uint16_t m_blt_trans_colour;
	uint16_t m_blt_trans_colour_mask;

	bool m_blt_system_transfer;  // blit from system memory
	uint8_t m_blt_system_count;
	uint32_t m_blt_system_buffer;
	uint16_t m_blt_pixel_count;
	uint16_t m_blt_scan_count;

	uint8_t m_scratchpad1;
	uint8_t m_scratchpad2;
	uint8_t m_scratchpad3;
	uint8_t m_vclk_num[4];
	uint8_t m_vclk_denom[4];

	virtual uint8_t vga_latch_write(int offs, uint8_t data) override;

	virtual void crtc_map(address_map &map) override;
	virtual void gc_map(address_map &map) override;
	virtual void sequencer_map(address_map &map) override;

	virtual bool get_interlace_mode() override { return BIT(m_cr1a, 0); }

private:
	void cirrus_define_video_mode();

	void start_bitblt();
	void start_reverse_bitblt();
	void start_system_bitblt();
	void blit_dword();
	void blit_byte();  // used for colour expanded system-to-vram bitblts
	void copy_pixel(uint8_t src, uint8_t dst);
};

class cirrus_gd5430_device :  public cirrus_gd5428_device
{
public:
	cirrus_gd5430_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};

class cirrus_gd5446_device :  public cirrus_gd5428_device
{
public:
	cirrus_gd5446_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// device type definition
DECLARE_DEVICE_TYPE(CIRRUS_GD5428, cirrus_gd5428_device)
DECLARE_DEVICE_TYPE(CIRRUS_GD5430, cirrus_gd5430_device)
DECLARE_DEVICE_TYPE(CIRRUS_GD5446, cirrus_gd5446_device)

#endif // MAME_VIDEO_PC_VGA_CIRRUS_H
