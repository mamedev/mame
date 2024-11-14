// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef MAME_VIDEO_VOODOO_H
#define MAME_VIDEO_VOODOO_H

#pragma once

#include "screen.h"


// forward declarations
class voodoo_1_device;
namespace voodoo { class save_proxy; }


//**************************************************************************
//  CONSTANTS
//**************************************************************************

namespace voodoo
{

// enumeration specifying which model of Voodoo we are emulating
enum class voodoo_model : u8
{
	VOODOO_1,
	VOODOO_2,
	VOODOO_BANSHEE,
	VOODOO_3
};

// debug
static constexpr bool DEBUG_DEPTH = false;      // ENTER key to view depthbuf
static constexpr bool DEBUG_BACKBUF = false;    // L key to view backbuf
static constexpr bool DEBUG_STATS = false;      // \ key to view stats

// logging
static constexpr bool LOG_VBLANK_SWAP = false;
static constexpr bool LOG_FIFO = false;
static constexpr bool LOG_FIFO_VERBOSE = false;
static constexpr bool LOG_REGISTERS = false;
static constexpr bool LOG_WAITS = false;
static constexpr bool LOG_LFB = false;
static constexpr bool LOG_TEXTURE_RAM = false;
static constexpr bool LOG_CMDFIFO = false;
static constexpr bool LOG_CMDFIFO_VERBOSE = false;

}


//**************************************************************************
//  INTERNAL CLASSES
//**************************************************************************

// include register and render classes
#include "voodoo_regs.h"
#include "voodoo_render.h"


namespace voodoo
{

// ======================> save_proxy

// save_proxy is a helper class to make hierarchical state saving more manageable;
class save_proxy
{
public:
	// constructor
	save_proxy(device_t &device) :
		m_device(device)
	{
	}

	// save an item; append the current prefix and pass through
	template<typename T>
	void save_item(T &&item, char const *name)
	{
		std::string fullname = m_prefix;
		fullname += name;
		m_device.save_item(std::forward<T>(item), fullname.c_str());
	}

	// save a pointer item; append the current prefix and pass through
	template<typename T>
	void save_pointer(T &&item, char const *name, u32 count)
	{
		std::string fullname = m_prefix;
		fullname += name;
		m_device.save_pointer(std::forward<T>(item), fullname.c_str(), count);
	}

	// save a class; update the prefix then call the register_save method on the class
	template<typename T>
	void save_class(T &item, char const *name)
	{
		std::string orig = m_prefix;
		m_prefix += name;
		m_prefix += "/";
		item.register_save(*this);
		m_prefix = orig;
	}

private:
	// internal state
	device_t &m_device;
	std::string m_prefix;
};


// ======================> shared_tables

// shared_tables are global tables that are shared between different components
struct shared_tables
{
	// construction
	shared_tables();

	// texel lookups
	rgb_t *texel[16];       // 16 texture formats

	// 8-bit lookups
	rgb_t rgb332[256];      // RGB 3-3-2 lookup table
	rgb_t alpha8[256];      // alpha 8-bit lookup table
	rgb_t int8[256];        // intensity 8-bit lookup table
	rgb_t ai44[256];        // alpha, intensity 4-4 lookup table

	// 16-bit lookups
	rgb_t rgb565[65536];    // RGB 5-6-5 lookup table
	rgb_t argb1555[65536];  // ARGB 1-5-5-5 lookup table
	rgb_t argb4444[65536];  // ARGB 4-4-4-4 lookup table
};


// ======================> tmu_state

// tmu_state holds TMU-specific register and palette state
class tmu_state
{
public:
	// construction
	tmu_state();

	// initialization
	void init(int index, shared_tables const &share, u8 *ram, u32 size);

	// configuration
	void set_baseaddr_mask_shift(u32 mask, u8 shift) { m_basemask = mask; m_baseshift = shift; }

	// state saving
	void register_save(save_proxy &save);
	void post_load();

	// simple getters
	voodoo_regs &regs() { return m_reg; }
	bool dirty() const { return m_regdirty; }

	// simple setters
	void mark_dirty() { m_regdirty = true; }

	// write to the NCC/palette registers
	void ncc_w(offs_t offset, u32 data);

	// prepare a texture for the renderer
	rasterizer_texture &prepare_texture(voodoo_renderer &renderer);

private:
	// internal state
	int m_index;                         // index of ourself
	u8 *m_ram;                           // pointer to our RAM
	u32 m_mask;                          // mask to apply to pointers
	u32 m_basemask;                      // mask to apply to the texBaseAddr
	u8 m_baseshift;                      // shift to apply to the texBaseAddr

	// register state
	voodoo_regs m_reg;                   // TMU registers
	bool m_regdirty;                     // true if the LOD/mode/base registers have changed

	// lookups
	rgb_t const * const *m_texel_lookup; // texel lookups for each format

	// palettes
	bool m_palette_dirty[4];             // true if palette (0-1) or NCC (2-3) is dirty
	rgb_t m_palette[2][256];             // 2 versions of the palette
};


// ======================> memory_fifo

// memory_fifo is a simple memory access FIFO that is used on the frontend
// PCI bus (64 entries) or within the framebuffer (up to 64k)
class memory_fifo
{
public:
	// FIFO flags, added to offset
	static constexpr offs_t FLAGS_MASK     = 0xf0000000;
	static constexpr offs_t TYPE_MASK      = 0xc0000000;
	static constexpr offs_t TYPE_REGISTER  = 0x00000000;
	static constexpr offs_t TYPE_LFB       = 0x40000000;
	static constexpr offs_t TYPE_TEXTURE   = 0x80000000;
	static constexpr offs_t NO_16_31       = 0x20000000;
	static constexpr offs_t NO_0_15        = 0x10000000;

	// construction
	memory_fifo();

	// configuration
	void configure(u32 *base, u32 size);

	// state saving
	void register_save(save_proxy &save);

	// basic queries
	bool configured() const { return (m_size != 0); }
	u32 peek() const { return m_base[m_out]; }
	bool empty() const { return (m_in == m_out); }
	bool full() const { return ((m_in + 1) == m_out) || (m_in == (m_size - 1) && m_out == 0); }
	s32 items() const { s32 result = m_in - m_out; if (result < 0) result += m_size; return result; }
	s32 space() const { return m_size - 1 - items(); }

	// reset
	void reset() { m_in = m_out = 0; }

	// add/remove items
	void add(u32 data);
	u32 remove();

private:
	// internal state
	u32 *m_base;   // base of the FIFO
	s32 m_size;    // size of the FIFO
	s32 m_in;      // input pointer
	s32 m_out;     // output pointer
};


// ======================> debug_stats

// debug_stats are enabled via DEBUG_STATS and displayed via the backslash key;
// these are independent of the real on-chip stats kept in thread_stats_block
class debug_stats
{
public:
	// construction
	debug_stats();

	// add in states from the emulation
	void add_emulation_stats(thread_stats_block const &block);

	// reset the stats
	void reset();

	// simple getters
	bool displayed() const { return m_display; }
	char const *string() const { return m_string.c_str(); }

	// compute the string to display
	void update_string(rectangle const &visarea, u32 swap_history);

	// based on the current key state, update and return whether stats should be shown
	bool update_display_state(bool key_pressed);

	// public access to the statistics that are hand-updated
	s32 m_swaps;              // total swaps
	s32 m_stalls;             // total stalls
	s32 m_triangles;          // total triangles
	s32 m_lfb_writes;         // LFB writes
	s32 m_lfb_reads;          // LFB reads
	s32 m_reg_writes;         // register writes
	s32 m_reg_reads;          // register reads
	s32 m_tex_writes;         // texture writes
	s32 m_texture_mode[16];   // 16 different texture modes

private:
	// stats that are updated from emulation stats
	s32 m_pixels_in;          // total pixels in
	s32 m_pixels_out;         // total pixels out
	s32 m_chroma_fail;        // total chroma fail
	s32 m_zfunc_fail;         // total z func fail
	s32 m_afunc_fail;         // total a func fail
	s32 m_clipped;            // total clipped
	s32 m_stippled;           // total stippled

	// internal state
	bool m_lastkey;           // last key state
	bool m_display;           // display stats?
	std::string m_string;     // string
};


// ======================> static_register_table_entry

// static_register_table_entry represents a read/write handler pair for Voodoo
// registers, along with a valid mask, access flags, and a string name
template<typename BaseType>
struct static_register_table_entry
{
	static constexpr u32 make_mask(int maskbits)
	{
		if (maskbits == 0)
			return 0;
		if (maskbits == 32)
			return 0xffffffff;
		return (1 << maskbits) - 1;
	}

	using read_handler = u32 (BaseType::*)(u32 chipmask, u32 regnum);
	using write_handler = u32 (BaseType::*)(u32 chipmask, u32 regnum, u32 data);

	u32 m_mask;                // mask to apply to written data
	u32 m_chipmask_flags;      // valid chips, plus flags
	char const *m_name;        // string name
	write_handler m_write;     // write handler
	read_handler m_read;       // read handler
};


// ======================> register_table_entry

// register_table_entry is a live version of static_register_table_entry with
// bound delegates in place of the member function pointers
class register_table_entry
{
public:
	// internal delegates
	using read_handler = delegate<u32 (u32, u32)>;
	using write_handler = delegate<u32 (u32, u32, u32)>;

	// flags for the chipmask
	static constexpr u32 CHIPMASK_NA       = 0x0000;
	static constexpr u32 CHIPMASK_FBI      = 0x0001;
	static constexpr u32 CHIPMASK_TREX     = 0x0006;
	static constexpr u32 CHIPMASK_FBI_TREX = 0x0007;

	// flags for the sync state
	static constexpr u32 SYNC_NA           = 0x0000;
	static constexpr u32 SYNC_NOSYNC       = 0x0000;
	static constexpr u32 SYNC_SYNC         = 0x0100;

	// flags for the FIFO state
	static constexpr u32 FIFO_NA           = 0x0000;
	static constexpr u32 FIFO_NOFIFO       = 0x0000;
	static constexpr u32 FIFO_FIFO         = 0x0200;

	// simple getters
	char const *name() const { return m_name; }
	bool is_sync() const { return ((m_chipmask_flags & SYNC_SYNC) != 0); }
	bool is_fifo() const { return ((m_chipmask_flags & FIFO_FIFO) != 0); }

	// read/write helpers
	u32 read(voodoo_1_device &device, u32 chipmask, u32 regnum) const;
	u32 write(voodoo_1_device &device, u32 chipmask, u32 regnum, u32 data) const;

	// unpack from a static entry
	template<typename BaseType>
	void unpack(static_register_table_entry<BaseType> const &source, BaseType &device)
	{
		m_mask = source.m_mask;
		m_chipmask_flags = source.m_chipmask_flags;
		m_name = source.m_name;
		m_write = write_handler(source.m_write, &device);
		m_read = read_handler(source.m_read, &device);
	}

private:
	// internal state
	u32 m_mask;                // mask to apply to written data
	u32 m_chipmask_flags;      // valid chips, plus flags
	char const *m_name;        // string name
	write_handler m_write;     // write handler
	read_handler m_read;       // read handler
};

}


//**************************************************************************
//  GENERIC VOODOO DEVICE
//**************************************************************************

// ======================> generic_voodoo_device

// generic_voodoo_device is a base class that can be used to represent any of the
// specific voodoo devices below
class generic_voodoo_device : public device_t, public device_video_interface
{
public:
	// configuration
	void set_fbmem(int value) { m_fbmem_in_mb = value; }
	void set_tmumem(int value1, int value2) { m_tmumem0_in_mb = value1; m_tmumem1_in_mb = value2; }
	void set_status_cycles(u32 value) { m_status_cycles = value; }
	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	auto vblank_callback() { return m_vblank_cb.bind(); }
	auto stall_callback() { return m_stall_cb.bind(); }
	auto pciint_callback() { return m_pciint_cb.bind(); }

	// getters
	voodoo::voodoo_model model() const { return m_model; }

	// address map and read/write helpers
	virtual void core_map(address_map &map) = 0;
	virtual u32 read(offs_t offset, u32 mem_mask = ~0) = 0;
	virtual void write(offs_t offset, u32 data, u32 mem_mask = ~0) = 0;

	// external control
	virtual void set_init_enable(u32 newval) = 0;

	// video update
	virtual int update(bitmap_rgb32 &bitmap, const rectangle &cliprect) = 0;

protected:
	// internal construction
	generic_voodoo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo::voodoo_model model);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// configuration
	const voodoo::voodoo_model m_model;      // which voodoo model
	u8 m_fbmem_in_mb;                        // framebuffer memory, in MB
	u8 m_tmumem0_in_mb;                      // TMU0 memory, in MB
	u8 m_tmumem1_in_mb;                      // TMU1 memory, in MB
	u32 m_status_cycles;                     // number of cycles to eat on status reads (optimization)
	required_device<cpu_device> m_cpu;       // the CPU we interact with
	devcb_write_line m_vblank_cb;            // VBLANK callback
	devcb_write_line m_stall_cb;             // stalling callback
	devcb_write_line m_pciint_cb;            // PCI interrupt callback
};


//**************************************************************************
//  VOODOO 1 DEVICE
//**************************************************************************

DECLARE_DEVICE_TYPE(VOODOO_1, voodoo_1_device)

// ======================> voodoo_1_device

// voodoo_1_device represents the original generation of 3dfx Voodoo Graphics devices;
// these devices have independent framebuffer and texture memory, and can be flexibly
// configured with 1 or 2 TMUs
class voodoo_1_device : public generic_voodoo_device
{
	friend class voodoo::register_table_entry;

	// enumeration describing reasons we might be stalled
	enum stall_state : u8
	{
		NOT_STALLED = 0,
		STALLED_UNTIL_FIFO_LWM,
		STALLED_UNTIL_FIFO_EMPTY
	};

	// flags for LFB writes
	static constexpr u8 LFB_RGB_PRESENT_0       = 0x01;
	static constexpr u8 LFB_ALPHA_PRESENT_0     = 0x02;
	static constexpr u8 LFB_DEPTH_PRESENT_0     = 0x04;
	static constexpr u8 LFB_DEPTH_PRESENT_MSW_0 = 0x08;
	static constexpr u8 LFB_PIXEL0_MASK         = 0x0f;

	static constexpr u8 LFB_RGB_PRESENT_1       = 0x10;
	static constexpr u8 LFB_ALPHA_PRESENT_1     = 0x20;
	static constexpr u8 LFB_DEPTH_PRESENT_1     = 0x40;
	static constexpr u8 LFB_PIXEL1_MASK         = 0x70;

protected:
	// number of clocks to set up a triangle (just a guess)
	static constexpr u32 TRIANGLE_SETUP_CLOCKS = 100;

	// internal construction
	voodoo_1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo::voodoo_model model);

public:
	// nominal clock values
	static constexpr u32 NOMINAL_CLOCK = 50'000'000;

	// construction
	voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		voodoo_1_device(mconfig, VOODOO_1, tag, owner, clock, voodoo::voodoo_model::VOODOO_1) { }

	// destruction
	virtual ~voodoo_1_device();

	// address map and read/write helpers
	virtual void core_map(address_map &map) override ATTR_COLD;
	virtual u32 read(offs_t offset, u32 mem_mask = ~0) override;
	virtual void write(offs_t offset, u32 data, u32 mem_mask = ~0) override;

	// external control
	virtual void set_init_enable(u32 newval) override;

	// video update
	virtual int update(bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// system management
	virtual void soft_reset();
	virtual void register_save(voodoo::save_proxy &save, u32 total_allocation);

	// buffer accessors
	virtual u16 *draw_buffer_indirect(int index);
	virtual u16 *lfb_buffer_indirect(int index);
	u16 *draw_buffer(int index) const { return (u16 *)(m_fbram + m_rgboffs[index]); }
	u16 *front_buffer() const { return draw_buffer(m_frontbuf); }
	u16 *back_buffer() const { return draw_buffer(m_backbuf); }
	u16 *aux_buffer() const { return (m_auxoffs != ~0) ? (u16 *)(m_fbram + m_auxoffs) : nullptr; }
	u16 *ram_end() const { return (u16 *)(m_fbram + m_fbmask + 1); }

	// read/write and FIFO helpers
	void prepare_for_read();
	bool prepare_for_write();
	void recompute_fbmem_fifo();
	void add_to_fifo(u32 offset, u32 data, u32 mem_mask);
	void flush_fifos(attotime current_time);
	virtual u32 execute_fifos();

	// mapped reads
	u32 map_register_r(offs_t offset);
	u32 map_lfb_r(offs_t offset);

	// mapped writes
	void map_register_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void map_lfb_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void map_texture_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	// internal reads and writes
	u32 internal_lfb_r(offs_t offset);
	void internal_lfb_w(offs_t offset, u32 data, u32 mem_mask);
	u32 expand_lfb_data(voodoo::reg_lfb_mode const lfbmode, u32 data, rgb_t src_color[2], u16 src_depth[2]);
	virtual void internal_texture_w(offs_t offset, u32 data);

	// register read accessors
	u32 reg_invalid_r(u32 chipmask, u32 regnum);
	u32 reg_passive_r(u32 chipmask, u32 regnum);
	u32 reg_status_r(u32 chipmask, u32 regnum);
	u32 reg_fbiinit2_r(u32 chipmask, u32 regnum);
	u32 reg_vretrace_r(u32 chipmask, u32 regnum);
	u32 reg_stats_r(u32 chipmask, u32 regnum);

	// register write accessors
	u32 reg_invalid_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_unimplemented_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_passive_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fpassive_4_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fpassive_12_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_starts_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_startt_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_dsdx_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_dtdx_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_dsdy_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_dtdy_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fstarts_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fstartt_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fdsdx_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fdtdx_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fdsdy_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fdtdy_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_startw_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_dwdx_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_dwdy_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fstartw_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fdwdx_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fdwdy_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_triangle_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_nop_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fastfill_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_swapbuffer_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fogtable_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_fbiinit_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_video_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_clut_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_dac_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_texture_w(u32 chipmask, u32 regnum, u32 data);
	u32 reg_palette_w(u32 chipmask, u32 regnum, u32 data);

	// VBLANK timing
	void adjust_vblank_start_timer();
	virtual void vblank_start(s32 param);
	virtual void vblank_stop(s32 param);
	void swap_buffers();
	virtual void rotate_buffers();

	// video timing and updates
	int update_common(bitmap_rgb32 &bitmap, const rectangle &cliprect, rgb_t const *pens);
	void recompute_video_timing(u32 hsyncon, u32 hsyncoff, u32 hvis, u32 hbp, u32 vsyncon, u32 vsyncoff, u32 vvis, u32 vbp);
	virtual void recompute_video_memory();
	void recompute_video_memory_common(u32 config, u32 rowpixels);

	// rendering
	voodoo::voodoo_renderer &renderer() { return *m_renderer; }
	s32 triangle();

	// statistics
	void accumulate_statistics(const voodoo::thread_stats_block &block);
	void update_statistics(bool accumulate);
	void reset_counters();

	// stall management
	bool operation_pending() const { return !m_operation_end.is_zero(); }
	void clear_pending_operation() { m_operation_end = attotime::zero; }
	void check_stalled_cpu(attotime current_time);
	void stall_cpu(stall_state state);
	void stall_resume_callback(s32 param);

	// misc helpers
	u32 chipmask_from_offset(u32 offset)
	{
		u32 chipmask = BIT(offset, 8, 4);
		if (chipmask == 0)
			chipmask = 0xf;
		return chipmask & m_chipmask;
	}

	// configuration
	u8 m_chipmask;                           // mask for which chips are available

	// PCI state/FIFOs
	voodoo::reg_init_en m_init_enable;       // initEnable value (set externally)
	stall_state m_stall_state;               // state of the system if we're stalled
	int m_stall_trigger;                     // trigger used for stalling
	attotime m_operation_end;                // time when the pending operation ends
	voodoo::memory_fifo m_pci_fifo;          // PCI FIFO
	voodoo::memory_fifo m_fbmem_fifo;        // framebuffer memory fifo
	bool m_flush_flag;                       // true if we are currently flushing FIFOs

	// allocated memory
	u8 *m_fbram;                             // pointer to aligned framebuffer
	u32 m_fbmask;                            // mask to apply to pointers
	std::unique_ptr<u8[]> m_memory;          // allocated framebuffer/texture memory
	std::unique_ptr<voodoo::shared_tables> m_shared; // shared tables
	std::unique_ptr<voodoo::voodoo_renderer> m_renderer; // rendering helper

	// video buffer configuration
	u32 m_rgboffs[3];                        // word offset to 3 RGB buffers
	u32 m_auxoffs;                           // word offset to 1 aux buffer
	u8 m_frontbuf;                           // front buffer index
	u8 m_backbuf;                            // back buffer index
	bool m_video_changed;                    // did the frontbuffer video change?

	// linear frame buffer access configuration
	u8 m_lfb_stride;                         // stride of LFB accesses in bits

	// video configuration
	u32 m_width;                             // width of current frame buffer
	u32 m_height;                            // height of current frame buffer
	u32 m_xoffs;                             // horizontal offset (back porch)
	u32 m_yoffs;                             // vertical offset (back porch)
	u32 m_vsyncstart;                        // vertical sync start scanline
	u32 m_vsyncstop;                         // vertical sync stop

	// VBLANK/swapping state
	u8 m_swaps_pending;                      // number of pending swaps
	u8 m_vblank;                             // VBLANK state
	u8 m_vblank_count;                       // number of VBLANKs since last swap
	u8 m_vblank_swap_pending;                // a swap is pending, waiting for a vblank
	u8 m_vblank_swap;                        // swap when we hit this count
	u8 m_vblank_dont_swap;                   // don't actually swap when we hit this point

	// register state
	voodoo::voodoo_regs m_reg;               // FBI registers
	voodoo::register_table_entry m_regtable[256]; // generated register table
	voodoo::tmu_state m_tmu[2];              // TMU states
	u8 m_dac_reg[32];                        // up to 32 DAC registers
	u8 m_dac_read_result;                    // pending DAC read result

	// timers
	emu_timer *m_vsync_start_timer;          // VBLANK timer
	emu_timer *m_vsync_stop_timer;           // VBLANK end timer
	emu_timer *m_stall_resume_timer;         // timer to resume processing after stall

	// statistics
	voodoo::debug_stats m_stats;             // internal statistics
	voodoo::thread_stats_block m_lfb_stats;  // LFB access statistics

	// tracking for logging
	offs_t m_last_status_pc;                 // PC of last status description (for logging)
	u32 m_last_status_value;                 // value of last status read (for logging)

	// memory for PCI FIFO
	u32 m_pci_fifo_mem[64*2];                // memory backing the PCI FIFO

	// pens and CLUT
	bool m_clut_dirty;                       // do we need to recompute?
	std::vector<rgb_t> m_clut;               // clut gamma data
	std::vector<rgb_t> m_pen;                // mapping from pixels to pens

	// register table
	static voodoo::static_register_table_entry<voodoo_1_device> const s_register_table[256];
};


//**************************************************************************
//  INLINE FUNCITIONS
//**************************************************************************

//-------------------------------------------------
//  write - handle a write through a register table
//  entry, performing any necessary logging
//-------------------------------------------------

inline u32 voodoo::register_table_entry::write(voodoo_1_device &device, u32 chipmask, u32 regnum, u32 data) const
{
	// statistics if enabled
	if (DEBUG_STATS)
		device.m_stats.m_reg_writes++;

	// log if enabled
	if (LOG_REGISTERS)
	{
		if (regnum < voodoo_regs::reg_fvertexAx || regnum > voodoo_regs::reg_fdWdY)
			device.logerror("VOODOO.REG:%s(%d) write = %08X\n", m_name, chipmask, data);
		else
			device.logerror("VOODOO.REG:%s(%d) write = %f\n", m_name, chipmask, double(u2f(data)));
	}
	return m_write(chipmask & m_chipmask_flags, regnum, data & m_mask);
}


//-------------------------------------------------
//  read - handle a read through a register table
//  entry, performing any necessary logging
//-------------------------------------------------

inline u32 voodoo::register_table_entry::read(voodoo_1_device &device, u32 chipmask, u32 regnum) const
{
	// statistics if enabled
	if (DEBUG_STATS)
		device.m_stats.m_reg_reads++;

	// get the result
	u32 result = m_read(chipmask & m_chipmask_flags, regnum) & m_mask;

	// log if enabled
	if (LOG_REGISTERS)
	{
		// don't log multiple identical status reads from the same address
		bool logit = true;
		if (regnum == voodoo_regs::reg_vdstatus)
		{
			offs_t pc = device.m_cpu->pc();
			if (pc == device.m_last_status_pc && result == device.m_last_status_value)
				logit = false;
			device.m_last_status_pc = pc;
			device.m_last_status_value = result;
		}
		if (regnum == voodoo_regs::reg_cmdFifoRdPtr)
			logit = false;

		if (logit)
			device.logerror("VOODOO.REG:%s read = %08X\n", m_name, result);
	}
	return result;
}

#endif // MAME_VIDEO_VOODOO_H
