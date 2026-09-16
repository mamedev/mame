// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_2.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef MAME_VIDEO_VOODOO_2_H
#define MAME_VIDEO_VOODOO_2_H

#pragma once

#include "voodoo.h"


// forward declarations
class voodoo_2_device;


//**************************************************************************
//  INTERNAL CLASSES
//**************************************************************************

namespace voodoo
{

// ======================> command_fifo

// command_fifo is a more intelligent FIFO that was introduced with the Voodoo-2
class command_fifo
{
public:
	// construction
	command_fifo(voodoo_2_device &device);

	// initialization
	void init(u8 *ram, u32 size) { m_ram = (u32 *)ram; m_mask = (size / 4) - 1; }

	// state saving
	void register_save(save_proxy &save);

	// getters
	bool enabled() const { return m_enable; }
	u32 address_min() const { return m_address_min; }
	u32 address_max() const { return m_address_max; }
	u32 read_pointer() const { return m_read_index * 4; }
	u32 depth() const { return m_depth; }
	u32 holes() const { return m_holes; }

	// setters
	void set_enable(bool enabled) { m_enable = enabled; }
	void set_count_holes(bool count) { m_count_holes = count; }
	void set_base(u32 base) { m_ram_base = base; }
	void set_end(u32 end) { m_ram_end = end; }
	void set_size(u32 size) { m_ram_end = m_ram_base + size; }
	void set_read_pointer(u32 ptr) { m_read_index = ptr / 4; }
	void set_address_min(u32 addr) { m_address_min = addr; }
	void set_address_max(u32 addr) { m_address_max = addr; }
	void set_depth(u32 depth) { m_depth = depth; }
	void set_holes(u32 holes) { m_holes = holes; }

	// operations
	u32 execute_if_ready();

	// write to the FIFO if within the address range
	bool write_if_in_range(offs_t addr, u32 data)
	{
		if (m_enable && addr >= m_ram_base && addr < m_ram_end)
		{
			write(addr, data);
			return true;
		}
		return false;
	}

	// write directly to the FIFO, relative to the base
	void write_direct(offs_t offset, u32 data)
	{
		write(m_ram_base + offset * 4, data);
	}

private:
	// internal helpers
	void consume(u32 words) { m_read_index += words; m_depth -= words; }
	u32 peek_next() { return m_ram[m_read_index & m_mask]; }
	u32 read_next() { u32 result = peek_next(); consume(1); return result; }
	float read_next_float() { float result = *(float *)&m_ram[m_read_index & m_mask]; consume(1); return result; }

	// internal operations
	u32 words_needed(u32 command);
	void write(offs_t addr, u32 data);

	// packet handlers
	using packet_handler = u32 (command_fifo::*)(u32);
	u32 packet_type_0(u32 command);
	u32 packet_type_1(u32 command);
	u32 packet_type_2(u32 command);
	u32 packet_type_3(u32 command);
	u32 packet_type_4(u32 command);
	u32 packet_type_5(u32 command);
	u32 packet_type_unknown(u32 command);

	// internal state
	voodoo_2_device &m_device;     // reference to our device
	u32 *m_ram;                    // base of RAM
	u32 m_mask;                    // mask for RAM accesses
	bool m_enable;                 // enabled?
	bool m_count_holes;            // count holes?
	u32 m_ram_base;                // base address in framebuffer RAM
	u32 m_ram_end;                 // end address in framebuffer RAM
	u32 m_read_index;              // current read index into 32-bit RAM
	u32 m_address_min;             // minimum address
	u32 m_address_max;             // maximum address
	u32 m_depth;                   // current depth
	u32 m_holes;                   // number of holes

	static packet_handler s_packet_handler[8];
};


// ======================> setup_vertex

// setup_vertex a set of coordinates managed by the triangle setup engine
// that was introduced with the Voodoo-2
struct setup_vertex
{
	// state saving
	void register_save(save_proxy &save)
	{
		save.save_item(NAME(x));
		save.save_item(NAME(y));
		save.save_item(NAME(z));
		save.save_item(NAME(wb));
		save.save_item(NAME(r));
		save.save_item(NAME(g));
		save.save_item(NAME(b));
		save.save_item(NAME(a));
		save.save_item(NAME(s0));
		save.save_item(NAME(t0));
		save.save_item(NAME(w0));
		save.save_item(NAME(s1));
		save.save_item(NAME(t1));
		save.save_item(NAME(w1));
	}

	float x, y;               // X, Y coordinates
	float z, wb;              // Z and broadcast W values
	float r, g, b, a;         // A, R, G, B values
	float s0, t0, w0;         // W, S, T for TMU 0
	float s1, t1, w1;         // W, S, T for TMU 1
};

}


//**************************************************************************
//  VOODOO 2 DEVICE
//**************************************************************************

DECLARE_DEVICE_TYPE(VOODOO_2, voodoo_2_device)

// ======================> voodoo_2_device

// voodoo_2_device represents the 2nd generation of 3dfx Voodoo Graphics devices;
// these are pretty similar in architecture to the first generation, with the
// addition of command FIFOs and several other smaller features
class voodoo_2_device : public voodoo_1_device
{
	friend class voodoo::command_fifo;

protected:
	// internal construction
	voodoo_2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo::voodoo_model model);

public:
	// nominal clock value
	static constexpr u32 NOMINAL_CLOCK = 90'000'000;

	// construction
	voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		voodoo_2_device(mconfig, VOODOO_2, tag, owner, clock, voodoo::voodoo_model::VOODOO_2) { }

	// address map and read/write helpers
	virtual void core_map(address_map &map) override ATTR_COLD;
	virtual u32 read(offs_t offset, u32 mem_mask = ~0) override;
	virtual void write(offs_t offset, u32 data, u32 mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// system management
	virtual void soft_reset() override;
	virtual void register_save(voodoo::save_proxy &save, u32 total_allocation) override;

	// mapped writes
	void map_register_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	// read/write and FIFO helpers
	virtual u32 execute_fifos() override;

	// register read accessors
	u32 reg_hvretrace_r(u32 chipmask, u32 regnum);
	u32 reg_cmdfifoptr_r(u32 chipmask, u32 regnum);
	u32 reg_cmdfifodepth_r(u32 chipmask, u32 regnum);
	u32 reg_cmdfifoholes_r(u32 chipmask, u32 regnum);

	// register write accessors
	u32 reg_intrctrl_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_video2_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_sargb_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_userintr_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_cmdfifo_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_cmdfifoptr_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_cmdfifodepth_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_cmdfifoholes_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fbiinit5_7_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_draw_tri_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_begin_tri_w(u32 chipmask, u32 regnum, u32 data);

	// command FIFO-specific write handlers
	virtual u32 cmdfifo_register_w(u32 offset, u32 data);
	virtual u32 cmdfifo_2d_w(u32 offset, u32 data);

	// VBLANK timing
	virtual void vblank_start(s32 param) override;
	virtual void vblank_stop(s32 param) override;

	// video timing and updates
	virtual void recompute_video_memory() override;

	// rendering
	s32 begin_triangle();
	s32 draw_triangle();
	s32 setup_and_draw_triangle();

	// internal state
	u8 m_sverts = 0;                         // number of vertices ready
	voodoo::setup_vertex m_svert[3];         // 3 setup vertices
	voodoo::command_fifo m_cmdfifo;          // command FIFO

	// register table
	static voodoo::static_register_table_entry<voodoo_2_device> const s_register_table[256];
};

#endif // MAME_VIDEO_VOODOO_2_H
