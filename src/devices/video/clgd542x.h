// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Cirrus Logic GD542x/3x video chipsets

*/
#ifndef MAME_VIDEO_CLGD542X_H
#define MAME_VIDEO_CLGD542X_H

#pragma once

#include "video/pc_vga.h"

class cirrus_gd5428_device :  public svga_device
{
public:
	// construction/destruction
	cirrus_gd5428_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual READ8_MEMBER(port_03c0_r) override;
	virtual WRITE8_MEMBER(port_03c0_w) override;
	virtual READ8_MEMBER(port_03b0_r) override;
	virtual WRITE8_MEMBER(port_03b0_w) override;
	virtual READ8_MEMBER(port_03d0_r) override;
	virtual WRITE8_MEMBER(port_03d0_w) override;
	virtual READ8_MEMBER(mem_r) override;
	virtual WRITE8_MEMBER(mem_w) override;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	cirrus_gd5428_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint16_t offset() override;

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

	inline uint8_t cirrus_vga_latch_write(int offs, uint8_t data);

	void pcvideo_cirrus_gd5428(machine_config &config);
	void pcvideo_cirrus_gd5430(machine_config &config);

private:
	void cirrus_define_video_mode();
	uint8_t cirrus_seq_reg_read(uint8_t index);
	void cirrus_seq_reg_write(uint8_t index, uint8_t data);
	uint8_t cirrus_gc_reg_read(uint8_t index);
	void cirrus_gc_reg_write(uint8_t index, uint8_t data);
	uint8_t cirrus_crtc_reg_read(uint8_t index);
	void cirrus_crtc_reg_write(uint8_t index, uint8_t data);

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

#endif // MAME_VIDEO_CLGD542X_H
