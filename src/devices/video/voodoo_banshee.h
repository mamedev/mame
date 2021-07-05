// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_banshee.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef MAME_VIDEO_VOODOO_BANSHEE_H
#define MAME_VIDEO_VOODOO_BANSHEE_H

#pragma once

#include "voodoo_2.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// nominal clock values
static constexpr u32 STD_VOODOO_BANSHEE_CLOCK = 90000000;
static constexpr u32 STD_VOODOO_3_CLOCK = 132000000;


//**************************************************************************
//  VOODOO DEVICES
//**************************************************************************

class voodoo_banshee_device_base : public voodoo_2_device
{
public:
	voodoo_banshee_device_base(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u16 *lfb_buffer_indirect(int index) override;
	virtual u16 *draw_buffer_indirect(int index) override;

	virtual u32 read(offs_t offset, u32 mem_mask = ~0) override;
	virtual void write(offs_t offset, u32 data, u32 mem_mask = ~0) override;

	u32 banshee_r(offs_t offset, u32 mem_mask = ~0) { return read(offset, mem_mask); }
	void banshee_w(offs_t offset, u32 data, u32 mem_mask = ~0) { write(offset, data, mem_mask); }
	u32 banshee_fb_r(offs_t offset);
	void banshee_fb_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_io_r(offs_t offset, u32 mem_mask = ~0);
	void banshee_io_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_rom_r(offs_t offset);
	u8 banshee_vga_r(offs_t offset);
	void banshee_vga_w(offs_t offset, u8 data);

	virtual int update(bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	virtual void core_map(address_map &map) override;
	virtual void lfb_map(address_map &map);
	virtual void io_map(address_map &map);

	u32 map_io_r(offs_t offset, u32 mem_mask);
	u32 map_cmd_agp_r(offs_t offset);
	u32 map_2d_r(offs_t offset);
	u32 map_register_r(offs_t offset);

	void map_io_w(offs_t offset, u32 data, u32 mem_mask);
	void map_cmd_agp_w(offs_t offset, u32 data, u32 mem_mask);
	void map_2d_w(offs_t offset, u32 data, u32 mem_mask);
	void map_register_w(offs_t offset, u32 data, u32 mem_mask);
	template<int Which> void map_texture_w(offs_t offset, u32 data, u32 mem_mask);
	void map_yuv_w(offs_t offset, u32 data, u32 mem_mask);
	void map_lfb_w(offs_t offset, u32 data, u32 mem_mask);

protected:
	// construction
	voodoo_banshee_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model);

	// device-level overrides
	virtual void device_start() override;

	virtual u32 execute_fifos() override;

	virtual void rotate_buffers() override;

	virtual void register_save(voodoo::save_proxy &save, u32 total_allocation) override;

	virtual void texture_w(offs_t offset, u32 data) override;

	void recompute_video();

	// device-level overrides
	u32 banshee_agp_r(offs_t offset);
	void banshee_agp_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	s32 banshee_2d_w(offs_t offset, u32 data);
	void banshee_blit_2d(u32 data);
	s32 lfb_direct_w(offs_t offset, u32 data, u32 mem_mask);

	u32 reg_status_r(u32 chipmask, u32 offset);
	u32 reg_colbufbase_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_colbufstride_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_auxbufbase_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_auxbufstride_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_swappending_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_overbuffer_w(u32 chipmask, u32 regnum, u32 data);

	// command FIFO-specific write handlers
	virtual u32 cmdfifo_register_w(u32 offset, u32 data) override;
	virtual u32 cmdfifo_2d_w(u32 offset, u32 data) override;

	// internal state
	voodoo::command_fifo m_cmdfifo2;         // second command FIFO

	u32 m_lfb_base;

	voodoo::banshee_io_regs m_io_regs;               // I/O registers
	u32 m_reg_agp[0x80];              // AGP registers
	u8 m_reg_vga[0x20];              // VGA registers
	u8 m_reg_vga_crtc[0x27];             // VGA CRTC registers
	u8 m_reg_vga_seq[0x05];              // VGA sequencer registers
	u8 m_reg_vga_gc[0x05];               // VGA graphics controller registers
	u8 m_reg_vga_att[0x15];              // VGA attribute registers
	u8 m_reg_vga_att_ff = 0;                  // VGA attribute flip-flop

	u32 m_blt_regs[0x20];         // 2D Blitter registers
	u32 m_blt_dst_base = 0;
	u32 m_blt_dst_x = 0;
	u32 m_blt_dst_y = 0;
	u32 m_blt_dst_width = 0;
	u32 m_blt_dst_height = 0;
	u32 m_blt_dst_stride = 0;
	u32 m_blt_dst_bpp = 0;
	u32 m_blt_cmd = 0;
	u32 m_blt_src_base = 0;
	u32 m_blt_src_x = 0;
	u32 m_blt_src_y = 0;
	u32 m_blt_src_width = 0;
	u32 m_blt_src_height = 0;
	u32 m_blt_src_stride = 0;
	u32 m_blt_src_bpp = 0;

	static voodoo::static_register_table_entry<voodoo_banshee_device_base> const s_register_table[256];
};


class voodoo_banshee_device : public voodoo_banshee_device_base
{
public:
	voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class voodoo_3_device : public voodoo_banshee_device_base
{
public:
	voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

};


DECLARE_DEVICE_TYPE(VOODOO_BANSHEE, voodoo_banshee_device)
DECLARE_DEVICE_TYPE(VOODOO_3,       voodoo_3_device)

#endif // MAME_VIDEO_VOODOO_H
