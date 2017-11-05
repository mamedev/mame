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
	virtual READ8_MEMBER(port_03c0_r) override;
	virtual WRITE8_MEMBER(port_03c0_w) override;
	virtual READ8_MEMBER(port_03d0_r) override;
	virtual WRITE8_MEMBER(port_03d0_w) override;
	DECLARE_READ8_MEMBER(port_83c6_r);
	DECLARE_WRITE8_MEMBER(port_83c6_w);
	DECLARE_READ8_MEMBER(port_43c6_r);
	DECLARE_WRITE8_MEMBER(port_43c6_w);
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	virtual READ8_MEMBER(mem_r) override;
	virtual WRITE8_MEMBER(mem_w) override;
	virtual uint16_t offset() override;

	DECLARE_READ8_MEMBER(accel_r);
	DECLARE_WRITE8_MEMBER(accel_w);

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
		uint16_t accel_opermode;
		uint8_t accel_command;
		uint8_t accel_fmix;
		uint32_t accel_drawflags;
		uint32_t accel_fgcolour;
		uint32_t accel_bgcolour;
		uint16_t accel_pattern_loc;
		int16_t accel_source_x;
		int16_t accel_source_y;
		int16_t accel_dest_x;
		int16_t accel_dest_y;
		int16_t accel_dim_x;
		int16_t accel_dim_y;
		uint32_t accel_style;
		uint32_t accel_ckey;
		int16_t accel_source_x_clip;
		int16_t accel_source_y_clip;
		int16_t accel_dest_x_clip;
		int16_t accel_dest_y_clip;
		uint32_t accel_fg_pattern_colour;
		uint32_t accel_bg_pattern_colour;
		uint8_t accel_pattern[0x80];
		bool accel_busy;
		bool accel_memwrite_active;  // true when writing to VRAM will push data to an ongoing command (SRCMONO/PATMONO)
		int16_t accel_mem_x;
		int16_t accel_mem_y;
		uint32_t accel_transfer;
	} tri;
	uint8_t m_version;
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
	void old_mmio_w(address_space& space, uint32_t offset, uint8_t data);
	uint8_t old_mmio_r(address_space& space, uint32_t offset);

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
