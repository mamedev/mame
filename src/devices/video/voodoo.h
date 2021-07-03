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


#define USE_MEMORY_VIEWS (0)

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// maximum number of TMUs
static constexpr int MAX_TMU = 2;

// enumeration specifying which model of Voodoo we are emulating
enum voodoo_model : u8
{
	MODEL_VOODOO_1,
	MODEL_VOODOO_2,
	MODEL_VOODOO_BANSHEE,
	MODEL_VOODOO_3
};

// nominal clock values
static constexpr u32 STD_VOODOO_1_CLOCK = 50000000;
static constexpr u32 STD_VOODOO_2_CLOCK = 90000000;


//**************************************************************************
//  DEBUGGING
//**************************************************************************

namespace voodoo
{

// debug
static constexpr bool DEBUG_DEPTH = false;		// ENTER key to view depthbuf
static constexpr bool DEBUG_BACKBUF = false;	// L key to view backbuf
static constexpr bool DEBUG_STATS = false;		// \ key to view stats

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
static constexpr bool LOG_BANSHEE_2D = false;

// Need to turn off cycle eating when debugging MIPS drc
// otherwise timer interrupts won't match nodrc debug mode.
static constexpr bool EAT_CYCLES = true;

}


//**************************************************************************
//  INTERNAL CLASSES
//**************************************************************************

namespace voodoo
{
class voodoo_device_base;
class voodoo_2_device_base;
class save_proxy;
}

// include register and render classes
#include "voodoo_regs.h"
#include "voodoo_render.h"

namespace voodoo
{

// ======================> save_proxy

// save_proxy is a helper class to make hierarchical state saving more manageable
class save_proxy
{
public:
	save_proxy(device_t &device) :
		m_device(device)
	{
	}

	template<typename T>
	void save_item(T &&item, char const *name)
	{
		std::string fullname = m_prefix;
		fullname += name;
		m_device.save_item(std::forward<T>(item), fullname.c_str());
	}

	template<typename T>
	void save_pointer(T &&item, char const *name, u32 count)
	{
		std::string fullname = m_prefix;
		fullname += name;
		m_device.save_pointer(std::forward<T>(item), fullname.c_str(), count);
	}

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
	device_t &m_device;
	std::string m_prefix;
};


// ======================> shared_tables

// shared_tables are global tables that are shared between different components
struct shared_tables
{
	// constructor
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
	tmu_state(voodoo_model model) : m_reg(model) { }

	// initialization
	void init(int index, shared_tables const &share, u8 *ram, u32 size);

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
	int m_index;                    // index of ourself
	u8 *m_ram = nullptr;            // pointer to our RAM
	u32 m_mask = 0;                 // mask to apply to pointers

	// register state
	voodoo_regs m_reg;              // TMU registers
	bool m_regdirty;                // true if the LOD/mode/base registers have changed

	// lookups
	rgb_t const * const *m_texel_lookup; // texel lookups for each format

	// palettes
	bool m_palette_dirty[4];        // true if palette (0-1) or NCC (2-3) is dirty
	rgb_t m_palette[2][256];        // 2 versions of the palette
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
	void configure(u32 *base, u32 size) { m_base = base; m_size = size; reset(); }

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


// ======================> command_fifo

// command_fifo is a more intelligent FIFO that was introduced with the Voodoo-2
class command_fifo
{
public:
	// construction
	command_fifo(voodoo_2_device_base &device);

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
	s32 execute_if_ready();

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
	voodoo_2_device_base &m_device;// reference to our device
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


// ======================> debug_stats

// debug_stats are enabled via DEBUG_STATS and displayed via the backslash key
class debug_stats
{
public:
	debug_stats()
	{
		std::fill(std::begin(texture_mode), std::end(texture_mode), 0);
		buffer[0] = 0;
	}

	u8 lastkey = 0;            // last key state
	u8 display = 0;            // display stats?
	s32 swaps = 0;              // total swaps
	s32 stalls = 0;             // total stalls
	s32 total_triangles = 0;    // total triangles
	s32 total_pixels_in = 0;    // total pixels in
	s32 total_pixels_out = 0;   // total pixels out
	s32 total_chroma_fail = 0;  // total chroma fail
	s32 total_zfunc_fail = 0;   // total z func fail
	s32 total_afunc_fail = 0;   // total a func fail
	s32 total_clipped = 0;      // total clipped
	s32 total_stippled = 0;     // total stippled
	s32 lfb_writes = 0;         // LFB writes
	s32 lfb_reads = 0;          // LFB reads
	s32 reg_writes = 0;         // register writes
	s32 reg_reads = 0;          // register reads
	s32 tex_writes = 0;         // texture writes
	s32 texture_mode[16];       // 16 different texture modes
	u8 render_override = 0;    // render override
	char buffer[1024];           // string
};



//**************************************************************************
//  VOODOO DEVICES
//**************************************************************************

class voodoo_device_base : public device_t, public device_video_interface
{
	friend class voodoo::register_table_entry;

	// enumeration describing reasons we might be stalled
	enum stall_state : u8
	{
		NOT_STALLED = 0,
		STALLED_UNTIL_FIFO_LWM,
		STALLED_UNTIL_FIFO_EMPTY
	};

public:
	// destruction
	virtual ~voodoo_device_base();

	// configuration
	void set_fbmem(int value) { m_fbmem_in_mb = value; }
	void set_tmumem(int value1, int value2) { m_tmumem0_in_mb = value1; m_tmumem1_in_mb = value2; }
	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	auto vblank_callback() { return m_vblank_cb.bind(); }
	auto stall_callback() { return m_stall_cb.bind(); }
	auto pciint_callback() { return m_pciint_cb.bind(); }

	// getters
	voodoo_model model() const { return m_model; }

	// address map and read/write helpers
	virtual void core_map(address_map &map) = 0;
	virtual u32 read(offs_t offset, u32 mem_mask = ~0) = 0;
	virtual void write(offs_t offset, u32 data, u32 mem_mask = ~0) = 0;

	// external control
	void set_init_enable(u32 newval);

	// video update
	virtual int update(bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// construction
	voodoo_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	// system management
	void soft_reset();
	virtual void register_save(save_proxy &save, u32 total_allocation);

	// VBLANK timing
	void adjust_vblank_start_timer();
	virtual void vblank_start(void *ptr, s32 param);
	virtual void vblank_stop(void *ptr, s32 param);
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

	// buffer accessors
	virtual u16 *draw_buffer_indirect(int index, bool depth_allowed);
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
	virtual void flush_fifos(attotime current_time);

	// stall management
	bool operation_pending() const { return !m_operation_end.is_zero(); }
	void clear_pending_operation() { m_operation_end = attotime::zero; }
	void check_stalled_cpu(attotime current_time);
	void stall_cpu(stall_state state);
	TIMER_CALLBACK_MEMBER( stall_resume_callback );

	// reads and writes
	u32 lfb_r(offs_t offset, bool lfb_3d);
	void lfb_w(offs_t offset, u32 data, u32 mem_mask);
	virtual void texture_w(offs_t offset, u32 data);

	u32 chipmask_from_offset(u32 offset)
	{
		u32 chipmask = BIT(offset, 8, 4);
		if (chipmask == 0)
			chipmask = 0xf;
		return chipmask & m_chipmask;
	}

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

	// configuration
	const voodoo_model m_model;              // which voodoo model
	u8 m_chipmask;                           // mask for which chips are available
	u8 m_fbmem_in_mb;                        // framebuffer memory, in MB
	u8 m_tmumem0_in_mb;                      // TMU0 memory, in MB
	u8 m_tmumem1_in_mb;                      // TMU1 memory, in MB
	required_device<cpu_device> m_cpu;       // the CPU we interact with
	devcb_write_line m_vblank_cb;            // VBLANK callback
	devcb_write_line m_stall_cb;             // stalling callback
	devcb_write_line m_pciint_cb;            // PCI interrupt callback

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
	u32 m_lfb_base;                          // base of LFB in memory
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
	register_table_entry m_regtable[256];    // generated register table
	tmu_state m_tmu[MAX_TMU];                // TMU states
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
	rgb_t m_clut[512];                       // clut gamma data
	bool m_clut_dirty;                       // do we need to recompute?
	rgb_t m_pen[65536];                      // mapping from pixels to pens
};



class voodoo_2_device_base : public voodoo_device_base
{
	friend class command_fifo;

public:

protected:
	// construction
	voodoo_2_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model);

	// device-level overrides
	virtual void device_start() override;
//	virtual void device_reset() override;
//	virtual void device_post_load() override;

	// system management
//	void soft_reset();
	virtual void register_save(save_proxy &save, u32 total_allocation) override;

	virtual void flush_fifos(attotime current_time) override;

	virtual void recompute_video_memory() override;

	virtual void vblank_start(void *ptr, s32 param) override;
	virtual void vblank_stop(void *ptr, s32 param) override;

	virtual s32 banshee_2d_w(offs_t offset, u32 data);

	// Voodoo-2 specific read handlers
	u32 reg_hvretrace_r(u32 chipmask, u32 regnum);
	u32 reg_cmdfifoptr_r(u32 chipmask, u32 regnum);
	u32 reg_cmdfifodepth_r(u32 chipmask, u32 regnum);
	u32 reg_cmdfifoholes_r(u32 chipmask, u32 regnum);

	// Voodoo-2 specific write handlers
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

	// Voodoo-2 specific helpers
	s32 begin_triangle();
	s32 draw_triangle();
	s32 setup_and_draw_triangle();
	virtual void update_register_view() { }

	// Voodoo 2 stuff
	u8 m_sverts = 0;                         // number of vertices ready
	voodoo::setup_vertex m_svert[3];         // 3 setup vertices
	voodoo::command_fifo m_cmdfifo[2];       // command FIFOs
#if USE_MEMORY_VIEWS
	memory_view m_regview;                   // switchable register view
#endif
};

}


using voodoo::voodoo_device_base;
using voodoo::voodoo_2_device_base;

class voodoo_1_device : public voodoo_device_base
{
public:
	voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u32 read(offs_t offset, u32 mem_mask = ~0) override;
	virtual void write(offs_t offset, u32 data, u32 mem_mask = ~0) override;

	u32 map_register_r(offs_t offset);
	u32 map_lfb_r(offs_t offset);

	void map_register_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void map_lfb_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void map_texture_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void core_map(address_map &map) override;

	// register accessors
	u32 reg_status_w(u32 chipmask, u32 offset, u32 data);

private:
	static voodoo::static_register_table_entry<voodoo_1_device> const s_register_table[256];
};


class voodoo_2_device : public voodoo_2_device_base
{
public:
	voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u32 read(offs_t offset, u32 mem_mask = ~0) override;
	virtual void write(offs_t offset, u32 data, u32 mem_mask = ~0) override;

	u32 map_register_r(offs_t offset);
	u32 map_lfb_r(offs_t offset);

	void map_register_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void map_lfb_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void map_texture_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void core_map(address_map &map) override;

protected:
#if USE_MEMORY_VIEWS
	virtual void update_register_view() override;
#endif

private:
	static voodoo::static_register_table_entry<voodoo_2_device> const s_register_table[256];
};




DECLARE_DEVICE_TYPE(VOODOO_1,       voodoo_1_device)
DECLARE_DEVICE_TYPE(VOODOO_2,       voodoo_2_device)

#endif // MAME_VIDEO_VOODOO_H
