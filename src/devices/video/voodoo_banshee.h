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

namespace voodoo
{

// logging
static constexpr bool LOG_BANSHEE_2D = false;

}


//**************************************************************************
//  VOODOO DEVICES
//**************************************************************************

DECLARE_DEVICE_TYPE(VOODOO_BANSHEE, voodoo_banshee_device)

// ======================> voodoo_banshee_device

class voodoo_banshee_device : public voodoo_2_device
{
protected:
	// internal construction
	voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo::voodoo_model model);

public:
	// nominal clock values
	static constexpr u32 NOMINAL_CLOCK = 90'000'000;

	// construction
	voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		voodoo_banshee_device(mconfig, VOODOO_BANSHEE, tag, owner, clock, voodoo::voodoo_model::VOODOO_BANSHEE) { }

	// core address map and read/write helpers
	virtual void core_map(address_map &map) override ATTR_COLD;
	virtual u32 read(offs_t offset, u32 mem_mask = ~0) override;
	virtual void write(offs_t offset, u32 data, u32 mem_mask = ~0) override;

	// LFB address map and read/write helpers
	virtual void lfb_map(address_map &map) ATTR_COLD;
	virtual u32 read_lfb(offs_t offset, u32 mem_mask = ~0);
	virtual void write_lfb(offs_t offset, u32 data, u32 mem_mask = ~0);

	// I/O address map and read/write helpers
	virtual void io_map(address_map &map) ATTR_COLD;
	virtual u32 read_io(offs_t offset, u32 mem_mask = ~0) { return map_io_r(offset, mem_mask); }
	virtual void write_io(offs_t offset, u32 data, u32 mem_mask = ~0) { map_io_w(offset, data, mem_mask); }

	// video update
	virtual int update(bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// system management
	virtual void soft_reset() override;
	virtual void register_save(voodoo::save_proxy &save, u32 total_allocation) override;

	// buffer accessors
	virtual u16 *lfb_buffer_indirect(int index) override;
	virtual u16 *draw_buffer_indirect(int index) override;

	// mapped reads
	u32 map_io_r(offs_t offset, u32 mem_mask);
	u32 map_cmd_agp_r(offs_t offset);
	u32 map_2d_r(offs_t offset);
	u32 map_register_r(offs_t offset);

	// mapped writes
	void map_io_w(offs_t offset, u32 data, u32 mem_mask);
	void map_cmd_agp_w(offs_t offset, u32 data, u32 mem_mask);
	void map_2d_w(offs_t offset, u32 data, u32 mem_mask);
	void map_register_w(offs_t offset, u32 data, u32 mem_mask);
	template<int Which> void map_texture_w(offs_t offset, u32 data, u32 mem_mask);
	void map_yuv_w(offs_t offset, u32 data, u32 mem_mask);
	void map_lfb_w(offs_t offset, u32 data, u32 mem_mask);

	// internal reads and writes
	u32 internal_io_r(offs_t offset, u32 mem_mask = ~0);
	void internal_io_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 internal_cmd_agp_r(offs_t offset);
	void internal_cmd_agp_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	s32 internal_2d_w(offs_t offset, u32 data);
	virtual void internal_texture_w(offs_t offset, u32 data) override;
	void internal_lfb_direct_w(offs_t offset, u32 data, u32 mem_mask);
	u8 internal_vga_r(offs_t offset);
	void internal_vga_w(offs_t offset, u8 data);

	// read/write and FIFO helpers
	virtual u32 execute_fifos() override;

	// register read accessors
	u32 reg_status_r(u32 chipmask, u32 offset);

	// register write accessors
	u32 reg_colbufbase_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_colbufstride_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_auxbufbase_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_auxbufstride_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_swappending_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_overbuffer_w(u32 chipmask, u32 regnum, u32 data);

	// VBLANK timing
	virtual void rotate_buffers() override;

	// video timing and updates
	void recompute_video();

	// command FIFO-specific write handlers
	virtual u32 cmdfifo_register_w(u32 offset, u32 data) override;
	virtual u32 cmdfifo_2d_w(u32 offset, u32 data) override;

	// rendering
	void execute_blit(u32 data);

	// internal state
	u32 m_lfb_base;                              // configured LFB base
	voodoo::command_fifo m_cmdfifo2;             // second command FIFO


	voodoo::banshee_io_regs m_io_regs;           // I/O registers
	voodoo::banshee_cmd_agp_regs m_cmd_agp_regs; // CMD/AGP registers
	voodoo::banshee_vga_regs m_vga_regs;         // VGA registers

	voodoo::banshee_2d_regs m_2d_regs;  // 2D registers
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

	static voodoo::static_register_table_entry<voodoo_banshee_device> const s_register_table[256];
};


// ======================> voodoo_3_device

DECLARE_DEVICE_TYPE(VOODOO_3, voodoo_3_device)

class voodoo_3_device : public voodoo_banshee_device
{
public:
	// nominal clock values
	static constexpr u32 NOMINAL_CLOCK = 132'000'000;

	// construction
	voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

#endif // MAME_VIDEO_VOODOO_BANSHEE_H
