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

#include "voodoo_regs.h"
#include "voodoo_render.h"


namespace voodoo
{

//**************************************************************************
//  DEBUGGING
//**************************************************************************

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


//**************************************************************************
//  INTERNAL CLASSES
//**************************************************************************

class voodoo_device_base;

// ======================> memory_fifo

// memory_fifo is a simple memory access FIFO that is used on the frontend
// PCI bus (64 entries) or within the framebuffer (up to 64k)
class memory_fifo
{
public:
	// construction
	memory_fifo();

	// configuration
	void configure(u32 *base, u32 size) { m_base = base; m_size = size; reset(); }

	// basic queries
	u32 peek() { return m_base[m_out]; }
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

// command_fifo is a more intelligent FIFO that was introduced with the
// Voodoo-2
class command_fifo
{
public:
	// construction
	command_fifo(voodoo_device_base &device);

	// initialization
	void init(std::vector<u8> &ram) { m_ram = reinterpret_cast<u32 *>(&ram[0]); m_mask = (ram.size() / 4) - 1; }

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
	voodoo_device_base &m_device;  // reference to our device
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

struct shared_tables
{
	// constructor
	shared_tables();

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

struct setup_vertex
{
	float x, y;               // X, Y coordinates
	float z, wb;              // Z and broadcast W values
	float r, g, b, a;         // A, R, G, B values
	float s0, t0, w0;         // W, S, T for TMU 0
	float s1, t1, w1;         // W, S, T for TMU 1
};

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

class voodoo_device_base : public device_t
{
	friend class voodoo::command_fifo;

	// enumeration describing reasons we might be stalled
	enum stall_state
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
	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	auto vblank_callback() { return m_vblank_cb.bind(); }
	auto stall_callback() { return m_stall_cb.bind(); }
	auto pciint_callback() { return m_pciint_cb.bind(); }

	// getters
	voodoo_model model() const { return m_model; }

	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0);

	TIMER_CALLBACK_MEMBER( vblank_off_callback );
	TIMER_CALLBACK_MEMBER( stall_resume_callback );
	TIMER_CALLBACK_MEMBER( vblank_callback );

	virtual int update(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void set_init_enable(u32 newval);

	voodoo::voodoo_renderer &renderer() { return *m_renderer; }

protected:
	// construction
	voodoo_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	struct tmu_state
	{
		tmu_state(voodoo_device_base &device) : m_device(device), m_reg(device.model()) { }

		void init(int index, voodoo::shared_tables &share, std::vector<u8> &memory);
		voodoo::rasterizer_texture &prepare_texture();
		s32 compute_lodbase();
		void texture_w(offs_t offset, u32 data, bool seq_8_downld);
		void ncc_w(offs_t offset, u32 data);

		voodoo_device_base &m_device;   // reference back to our owner
		int m_index;                    // index of ourself
		u8 *m_ram = nullptr;            // pointer to our RAM
		u32 m_mask = 0;                 // mask to apply to pointers

		voodoo::voodoo_regs m_reg;      // TMU registers
		bool m_regdirty;                // true if the LOD/mode/base registers have changed

		s64 m_starts, m_startt;         // starting S,T (14.18)
		s64 m_startw;                   // starting W (2.30)
		s64 m_dsdx, m_dtdx;             // delta S,T per X
		s64 m_dwdx;                     // delta W per X
		s64 m_dsdy, m_dtdy;             // delta S,T per Y
		s64 m_dwdy;                     // delta W per Y

		rgb_t *m_texel[16];             // texel lookups for each format
		bool m_palette_dirty[4];        // true if palette (0-1) or NCC (2-3) is dirty
		rgb_t m_palette[2][256];        // 2 versions of the palette
	};


	struct fbi_state
	{
		fbi_state(voodoo_device_base &device) : m_cmdfifo{ device, device } { }
		void init(voodoo_model model, std::vector<u8> &memory);

		void recompute_screen_params(voodoo::voodoo_regs &regs, screen_device &screen);
		void recompute_video_memory(voodoo::voodoo_regs &regs);
		void recompute_fifo_layout(voodoo::voodoo_regs &regs);
		bool copy_scanline(u32 *dst, int drawbuf, s32 y, s32 xstart, s32 xstop, rgb_t const *pens)
		{
			if (y < m_yoffs)
				return false;
			u16 const *const src = draw_buffer(drawbuf) + (y - m_yoffs) * m_rowpixels - m_xoffs;
			for (s32 x = xstart; x < xstop; x++)
				dst[x] = pens[src[x]];
			return true;
		}

		// internal helpers
		u16 *draw_buffer(int index) const { return (u16 *)(m_ram + m_rgboffs[index]); }
		u16 *front_buffer() const { return draw_buffer(m_frontbuf); }
		u16 *back_buffer() const { return draw_buffer(m_backbuf); }
		u16 *aux_buffer() const { return (m_auxoffs != ~0) ? (u16 *)(m_ram + m_auxoffs) : nullptr; }
		u16 *ram_end() const { return (u16 *)(m_ram + m_mask + 1); }

		u8 *m_ram;                    // pointer to frame buffer RAM
		u32 m_mask;                   // mask to apply to pointers
		u32 m_rgboffs[3];             // word offset to 3 RGB buffers
		u32 m_auxoffs;                // word offset to 1 aux buffer

		u8 m_frontbuf;                // front buffer index
		u8 m_backbuf;                 // back buffer index
		u8 m_swaps_pending;           // number of pending swaps
		bool m_video_changed;         // did the frontbuffer video change?

		u32 m_yorigin;                // Y origin subtract value
		u32 m_lfb_base;               // base of LFB in memory
		u8 m_lfb_stride;              // stride of LFB accesses in bits

		u32 m_width;                  // width of current frame buffer
		u32 m_height;                 // height of current frame buffer
		u32 m_xoffs;                  // horizontal offset (back porch)
		u32 m_yoffs;                  // vertical offset (back porch)
		u32 m_vsyncstart;             // vertical sync start scanline
		u32 m_vsyncstop;              // vertical sync stop
		u32 m_rowpixels;              // pixels per row

		u8 m_vblank;                  // VBLANK state
		u8 m_vblank_count;            // number of VBLANKs since last swap
		u8 m_vblank_swap_pending;     // a swap is pending, waiting for a vblank
		u8 m_vblank_swap;             // swap when we hit this count
		u8 m_vblank_dont_swap;        // don't actually swap when we hit this point

		// triangle setup info
		s32 m_sign;                   // triangle sign
		s16 m_ax, m_ay;               // vertex A x,y (12.4)
		s16 m_bx, m_by;               // vertex B x,y (12.4)
		s16 m_cx, m_cy;               // vertex C x,y (12.4)
		s32 m_startr, m_startg, m_startb, m_starta; // starting R,G,B,A (12.12)
		s32 m_startz;                 // starting Z (20.12)
		s64 m_startw;                 // starting W (16.32)
		s32 m_drdx, m_dgdx, m_dbdx, m_dadx; // delta R,G,B,A per X
		s32 m_dzdx;                   // delta Z per X
		s64 m_dwdx;                   // delta W per X
		s32 m_drdy, m_dgdy, m_dbdy, m_dady; // delta R,G,B,A per Y
		s32 m_dzdy;                   // delta Z per Y
		s64 m_dwdy;                   // delta W per Y

		voodoo::thread_stats_block m_lfb_stats; // LFB access statistics

		u8 m_sverts = 0;              // number of vertices ready
		voodoo::setup_vertex m_svert[3];      // 3 setup vertices

		voodoo::memory_fifo m_fifo;   // framebuffer memory fifo
		voodoo::command_fifo m_cmdfifo[2];    // command FIFOs

		rgb_t m_pen[65536];           // mapping from pixels to pens
		rgb_t m_clut[512];            // clut gamma data
		bool m_clut_dirty;            // do we need to recompute?
	};


	struct dac_state
	{
		void data_w(u8 regum, u8 data);
		void data_r(u8 regnum);

		u8 m_reg[8];                 // 8 registers
		u8 read_result;            // pending read result
	};


	void check_stalled_cpu(attotime current_time);
	void flush_fifos(attotime const &current_time);
	void init_fbi(fbi_state *f, void *memory, int fbmem);
	s32 register_w(offs_t offset, u32 data);
	s32 swapbuffer(u32 data);
	s32 lfb_w(offs_t offset, u32 data, u32 mem_mask);
	u32 lfb_r(offs_t offset, bool lfb_3d);
	s32 texture_w(offs_t offset, u32 data);
	void stall_cpu(stall_state state);
	void soft_reset();
	void adjust_vblank_timer();
	s32 fastfill();
	s32 triangle();

	void accumulate_statistics(const voodoo::thread_stats_block &block);
	void update_statistics(bool accumulate);
	void reset_counters();

	u32 register_r(offs_t offset);

	void swap_buffers();

protected:
	// internal helpers
	bool operation_pending() const { return !m_operation_end.is_zero(); }
	void clear_pending_operation() { m_operation_end = attotime::zero; }
	void register_save();
	virtual s32 banshee_2d_w(offs_t offset, u32 data);
	int update_common(bitmap_rgb32 &bitmap, const rectangle &cliprect, rgb_t const *pens);

	// setup engine
	s32 begin_triangle();
	s32 draw_triangle();
	void populate_setup_vertex(voodoo::setup_vertex &vertex);
	s32 setup_and_draw_triangle();

	// configuration
	const voodoo_model m_model;              // which voodoo model
	u8 m_chipmask;                           // mask for which chips are available
	u8 m_fbmem_in_mb;                        // framebuffer memory, in MB
	u8 m_tmumem0_in_mb;                      // TMU0 memory, in MB
	u8 m_tmumem1_in_mb;                      // TMU1 memory, in MB
	required_device<screen_device> m_screen; // the screen we are acting on
	required_device<cpu_device> m_cpu;       // the CPU we interact with
	devcb_write_line m_vblank_cb;            // VBLANK callback
	devcb_write_line m_stall_cb;             // stalling callback
	devcb_write_line m_pciint_cb;            // PCI interrupt callback

	std::unique_ptr<voodoo::voodoo_renderer> m_renderer; // rendering helper
	voodoo::voodoo_regs m_reg;               // FBI registers

	int m_trigger;                           // trigger used for stalling
	bool m_flush_flag;                       // true if we are currently flushing FIFOs

	offs_t m_last_status_pc;                 // PC of last status description (for logging)
	u32 m_last_status_value;                 // value of last status read (for logging)

	// PCI interface
	voodoo::reg_init_en m_init_enable;       // initEnable value (set externally)
	stall_state m_stall_state;               // state of the system if we're stalled
	attotime m_operation_end;                // time when the pending operation ends
	voodoo::memory_fifo m_pci_fifo;          // PCI FIFO
	u32 m_pci_fifo_mem[64*2];                // memory backing the PCI FIFO

	dac_state m_dac;                         // DAC state
	fbi_state m_fbi;                         // FBI states
	tmu_state m_tmu[MAX_TMU];                // TMU states

	emu_timer *m_vsync_stop_timer;           // VBLANK end timer
	emu_timer *m_vsync_start_timer;          // VBLANK timer
	emu_timer *m_stall_resume_timer;         // timer to resume processing after stall

	voodoo::debug_stats m_stats;             // internal statistics

	// allocated memory
	std::vector<u8> m_fbmem;                 // allocated framebuffer
	std::vector<u8> m_tmumem[2];             // allocated texture memory
	std::unique_ptr<voodoo::shared_tables> m_shared; // shared tables
};

}


using voodoo::voodoo_device_base;

class voodoo_1_device : public voodoo_device_base
{
public:
	voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class voodoo_2_device : public voodoo_device_base
{
public:
	voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};



DECLARE_DEVICE_TYPE(VOODOO_1,       voodoo_1_device)
DECLARE_DEVICE_TYPE(VOODOO_2,       voodoo_2_device)

#endif // MAME_VIDEO_VOODOO_H
