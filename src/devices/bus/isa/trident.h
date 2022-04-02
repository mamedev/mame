// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * trident.h
 *
 */

#ifndef MAME_BUS_ISA_TRIDENT_H
#define MAME_BUS_ISA_TRIDENT_H

#pragma once

#include "video/pc_vga.h"

// ======================> trident_vga_device

class trident_vga_device :  public svga_device
{
public:
	virtual uint8_t port_03c0_r(offs_t offset) override;
	virtual void port_03c0_w(offs_t offset, uint8_t data) override;
	virtual uint8_t port_03d0_r(offs_t offset) override;
	virtual void port_03d0_w(offs_t offset, uint8_t data) override;
	uint8_t port_83c6_r(offs_t offset);
	void port_83c6_w(offs_t offset, uint8_t data);
	uint8_t port_43c6_r(offs_t offset);
	void port_43c6_w(offs_t offset, uint8_t data);
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;
	virtual uint16_t offset() override;

	uint8_t accel_r(offs_t offset);
	void accel_w(offs_t offset, uint8_t data);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	// construction/destruction
	trident_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	struct
	{
		uint8_t sr0c;
		uint8_t sr0d_old;
		uint8_t sr0d_new;
		uint8_t sr0e_old;
		uint8_t sr0e_new;
		uint8_t sr0f;
		uint8_t gc0e;
		uint8_t gc0f;
		uint8_t gc2f;
		uint8_t cr1e;
		uint8_t cr1f;
		uint8_t cr20;
		uint8_t cr21;
		uint8_t cr29;
		uint8_t cr2a;
		uint8_t cr39;
		uint8_t dac;
		uint8_t lutdac_reg[0x100];
		uint8_t lutdac_index;
		bool new_mode;
		bool port_3c3;
		uint8_t port_3db;
		uint8_t clock;
		uint8_t pixel_depth;
		uint8_t revision;
		bool dac_active;
		uint8_t dac_count;
		uint32_t linear_address;
		bool linear_active;
		bool mmio_active;
		uint16_t mem_clock;  // I/O 0x43c6
		uint16_t vid_clock;  // I/O 0x43c8
		uint16_t cursor_x;
		uint16_t cursor_y;
		uint16_t cursor_loc;
		uint8_t cursor_x_off;
		uint8_t cursor_y_off;
		uint32_t cursor_fg;  // colour
		uint32_t cursor_bg;  // colour
		uint8_t cursor_ctrl;

		// 2D acceleration
		uint16_t accel_opermode = 0;
		uint8_t accel_command = 0;
		uint8_t accel_fmix = 0;
		uint32_t accel_drawflags = 0;
		uint32_t accel_fgcolour = 0;
		uint32_t accel_bgcolour = 0;
		uint16_t accel_pattern_loc = 0;
		int16_t accel_source_x = 0;
		int16_t accel_source_y = 0;
		int16_t accel_dest_x = 0;
		int16_t accel_dest_y = 0;
		int16_t accel_dim_x = 0;
		int16_t accel_dim_y = 0;
		uint32_t accel_style = 0;
		uint32_t accel_ckey = 0;
		int16_t accel_source_x_clip = 0;
		int16_t accel_source_y_clip = 0;
		int16_t accel_dest_x_clip = 0;
		int16_t accel_dest_y_clip = 0;
		uint32_t accel_fg_pattern_colour = 0;
		uint32_t accel_bg_pattern_colour = 0;
		uint8_t accel_pattern[0x80]{};
		bool accel_busy = false;
		bool accel_memwrite_active = false;  // true when writing to VRAM will push data to an ongoing command (SRCMONO/PATMONO)
		int16_t accel_mem_x = 0;
		int16_t accel_mem_y = 0;
		uint32_t accel_transfer = 0;
	} tri;
	uint8_t m_version = 0;
private:
	uint8_t trident_seq_reg_read(uint8_t index);
	void trident_seq_reg_write(uint8_t index, uint8_t data);
	void trident_define_video_mode();
	uint8_t trident_crtc_reg_read(uint8_t index);
	void trident_crtc_reg_write(uint8_t index, uint8_t data);
	uint8_t trident_gc_reg_read(uint8_t index);
	void trident_gc_reg_write(uint8_t index, uint8_t data);

	int calculate_clock();

	// old style MMIO (0xBFF00)
	void old_mmio_w(offs_t offset, uint8_t data);
	uint8_t old_mmio_r(offs_t offset);

	// 2D acceleration
	void accel_command();
	void accel_bitblt();
	void accel_line();
	void accel_data_write(uint32_t data);
	uint8_t READPIXEL8(int16_t x, int16_t y);
	uint16_t READPIXEL15(int16_t x, int16_t y);
	uint16_t READPIXEL16(int16_t x, int16_t y);
	uint32_t READPIXEL32(int16_t x, int16_t y);
	void WRITEPIXEL8(int16_t x, int16_t y, uint8_t data);
	void WRITEPIXEL15(int16_t x, int16_t y, uint16_t data);
	void WRITEPIXEL16(int16_t x, int16_t y, uint16_t data);
	void WRITEPIXEL32(int16_t x, int16_t y, uint32_t data);
	uint32_t READPIXEL(int16_t x,int16_t y);
	void WRITEPIXEL(int16_t x,int16_t y, uint32_t data);
	uint32_t handle_rop(uint32_t src, uint32_t dst);
};

class tgui9860_device : public trident_vga_device
{
public:
	tgui9860_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class tvga9000_device : public trident_vga_device
{
public:
	tvga9000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(TRIDENT_VGA,  tgui9860_device)
DECLARE_DEVICE_TYPE(TVGA9000_VGA, tvga9000_device)

#endif // MAME_BUS_ISA_TRIDENT_H
