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
enum
{
	TYPE_VOODOO_1,
	TYPE_VOODOO_2,
	TYPE_VOODOO_BANSHEE,
	TYPE_VOODOO_3
};

// nominal clock values
static constexpr u32 STD_VOODOO_1_CLOCK = 50000000;
static constexpr u32 STD_VOODOO_2_CLOCK = 90000000;
static constexpr u32 STD_VOODOO_BANSHEE_CLOCK = 90000000;
static constexpr u32 STD_VOODOO_3_CLOCK = 132000000;

#include "voodoo_regs.h"
#include "voodoo_render.h"


//**************************************************************************
//  INTERNAL CLASSES
//**************************************************************************

namespace voodoo
{

class fifo_state
{
public:
	// construction
	fifo_state();

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

//private:
	// internal state
	u32 *m_base;   // base of the FIFO
	s32 m_size;    // size of the FIFO
	s32 m_in;      // input pointer
	s32 m_out;     // output pointer
};

}


//**************************************************************************
//  VOODOO DEVICES
//**************************************************************************

class voodoo_device : public device_t
{
public:
	~voodoo_device();

	void set_fbmem(int value) { m_fbmem = value; }
	void set_tmumem(int value1, int value2) { m_tmumem0 = value1; m_tmumem1 = value2; }
	template <typename T> void set_screen_tag(T &&tag) { m_screen_finder.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_cpu_tag(T &&tag) { m_cpu_finder.set_tag(std::forward<T>(tag)); }
	auto vblank_callback() { return m_vblank.bind(); }
	auto stall_callback() { return m_stall.bind(); }
	auto pciint_callback() { return m_pciint.bind(); }

	void set_screen(screen_device &screen) { assert(!m_screen); m_screen = &screen; }
	void set_cpu(cpu_device &cpu) { assert(!m_cpu); m_cpu = &cpu; }

	u32 voodoo_r(offs_t offset);
	void voodoo_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	TIMER_CALLBACK_MEMBER( vblank_off_callback );
	TIMER_CALLBACK_MEMBER( stall_cpu_callback );
	TIMER_CALLBACK_MEMBER( vblank_callback );

	void voodoo_postload();

	int voodoo_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void voodoo_set_init_enable(u32 newval);

protected:
	voodoo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 vdt);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	struct tmu_shared_state;

	struct voodoo_stats
	{
		voodoo_stats()
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


	struct cmdfifo_info
	{
		u8 enable = 0;             // enabled?
		u8 count_holes = 0;        // count holes?
		u32 base = 0;               // base address in framebuffer RAM
		u32 end = 0;                // end address in framebuffer RAM
		u32 rdptr = 0;              // current read pointer
		u32 amin = 0;               // minimum address
		u32 amax = 0;               // maximum address
		u32 depth = 0;              // current depth
		u32 holes = 0;              // number of holes
	};


	struct pci_state
	{
		voodoo::fifo_state fifo;                   // PCI FIFO
		voodoo::reg_init_en init_enable = 0;        // initEnable value
		u8 stall_state = 0;        // state of the system if we're stalled
		u8 op_pending = 0;         // true if an operation is pending
		attotime op_end_time = attotime::zero; // time when the pending operation ends
		emu_timer *continue_timer = nullptr; // timer to use to continue processing
		u32 fifo_mem[64*2];         // memory backing the PCI FIFO
	};


	struct tmu_state
	{
		void init(u8 vdt, tmu_shared_state &share, void *memory, int tmem);
		voodoo::rasterizer_texture &prepare(s32 &lodbase);
		void texture_w(offs_t offset, u32 data, bool seq_8_downld);

		struct ncc_table
		{
			void write(offs_t regnum, u32 data);
			void update();

			u8 dirty = 0;              // is the texel lookup dirty?
			u32 *m_reg = nullptr;          // pointer to our registers
			s32 ir[4], ig[4], ib[4];    // I values for R,G,B
			s32 qr[4], qg[4], qb[4];    // Q values for R,G,B
			s32 y[16];                  // Y values
			rgb_t *palette = nullptr;      // pointer to associated RGB palette
			rgb_t *palettea = nullptr;     // pointer to associated ARGB palette
			rgb_t texel[256];             // texel lookup
		};

		int m_type;
		u8 *m_ram = nullptr;          // pointer to our RAM
		u32 m_mask = 0;               // mask to apply to pointers

		voodoo::voodoo_regs m_reg;          // pointer to our register base
		u32 m_regdirty = 0;           // true if the LOD/mode/base registers have changed

		s64 m_starts = 0, m_startt = 0; // starting S,T (14.18)
		s64 m_startw = 0;             // starting W (2.30)
		s64 m_dsdx = 0, m_dtdx = 0;     // delta S,T per X
		s64 m_dwdx = 0;               // delta W per X
		s64 m_dsdy = 0, m_dtdy = 0;     // delta S,T per Y
		s64 m_dwdy = 0;               // delta W per Y

		voodoo::rasterizer_texture m_raster;

		ncc_table m_ncc[2];                 // two NCC tables

		rgb_t *m_texel[16];              // texel lookups for each format

		rgb_t m_palette[256];           // palette lookup table
		rgb_t m_palettea[256];          // palette+alpha lookup table
	};


	struct tmu_shared_state
	{
		void init();

		rgb_t rgb332[256];            // RGB 3-3-2 lookup table
		rgb_t alpha8[256];            // alpha 8-bit lookup table
		rgb_t int8[256];              // intensity 8-bit lookup table
		rgb_t ai44[256];              // alpha, intensity 4-4 lookup table

		rgb_t * rgb565;                 // RGB 5-6-5 lookup table
		rgb_t argb1555[65536];        // ARGB 1-5-5-5 lookup table
		rgb_t argb4444[65536];        // ARGB 4-4-4-4 lookup table
	};


	struct fbi_state
	{
		struct setup_vertex
		{
			float x, y;                   // X, Y coordinates
			float z, wb;                  // Z and broadcast W values
			float r, g, b, a;             // A, R, G, B values
			float s0, t0, w0;             // W, S, T for TMU 0
			float s1, t1, w1;             // W, S, T for TMU 1
		};

		u8 *ram = nullptr;          // pointer to frame buffer RAM
		u32 mask = 0;               // mask to apply to pointers
		u32 rgboffs[3] = { 0, 0, 0 }; // word offset to 3 RGB buffers
		u32 auxoffs = 0;            // word offset to 1 aux buffer

		u8 frontbuf = 0;           // front buffer index
		u8 backbuf = 0;            // back buffer index
		u8 swaps_pending = 0;      // number of pending swaps
		u8 video_changed = 0;      // did the frontbuffer video change?

		u32 yorigin = 0;            // Y origin subtract value
		u32 lfb_base = 0;           // base of LFB in memory
		u8  lfb_stride = 0;         // stride of LFB accesses in bits

		u32 width = 0;              // width of current frame buffer
		u32 height = 0;             // height of current frame buffer
		u32 xoffs = 0;              // horizontal offset (back porch)
		u32 yoffs = 0;              // vertical offset (back porch)
		u32 vsyncstart = 0;         // vertical sync start scanline
		u32 vsyncstop = 0;          // vertical sync stop
		u32 rowpixels = 0;          // pixels per row
		u32 tile_width = 0;         // width of video tiles
		u32 tile_height = 0;        // height of video tiles
		u32 x_tiles = 0;            // number of tiles in the X direction

		emu_timer *vsync_stop_timer = nullptr; // VBLANK End timer
		emu_timer *vsync_start_timer = nullptr; // VBLANK timer
		u8 vblank = 0;             // VBLANK state
		u8 vblank_count = 0;       // number of VBLANKs since last swap
		u8 vblank_swap_pending = 0;// a swap is pending, waiting for a vblank
		u8 vblank_swap = 0;        // swap when we hit this count
		u8 vblank_dont_swap = 0;   // don't actually swap when we hit this point

		/* triangle setup info */
		s32 sign;                   // triangle sign
		s16 ax, ay;                 // vertex A x,y (12.4)
		s16 bx, by;                 // vertex B x,y (12.4)
		s16 cx, cy;                 // vertex C x,y (12.4)
		s32 startr, startg, startb, starta; // starting R,G,B,A (12.12)
		s32 startz;                 // starting Z (20.12)
		s64 startw;                 // starting W (16.32)
		s32 drdx, dgdx, dbdx, dadx; // delta R,G,B,A per X
		s32 dzdx;                   // delta Z per X
		s64 dwdx;                   // delta W per X
		s32 drdy, dgdy, dbdy, dady; // delta R,G,B,A per Y
		s32 dzdy;                   // delta Z per Y
		s64 dwdy;                   // delta W per Y

		voodoo::thread_stats_block lfb_stats;              // LFB-access statistics

		u8 sverts = 0;             // number of vertices ready */
		setup_vertex svert[3];               // 3 setup vertices */

		voodoo::fifo_state fifo;                   // framebuffer memory fifo */
		cmdfifo_info cmdfifo[2];             // command FIFOs */

		rgb_t pen[65536];             // mapping from pixels to pens */
		rgb_t clut[512];              // clut gamma data */
		u8 clut_dirty = 1;         // do we need to recompute? */
		rgb_t rgb565[65536];          // RGB 5-6-5 lookup table */
	};


	struct dac_state
	{
		void data_w(u8 regum, u8 data);
		void data_r(u8 regnum);

		u8 m_reg[8];                 // 8 registers
		u8 read_result;            // pending read result
	};


	struct banshee_info
	{
		u32 io[0x40];               // I/O registers
		u32 agp[0x80];              // AGP registers
		u8  vga[0x20];              // VGA registers
		u8  crtc[0x27];             // VGA CRTC registers
		u8  seq[0x05];              // VGA sequencer registers
		u8  gc[0x05];               // VGA graphics controller registers
		u8  att[0x15];              // VGA attribute registers
		u8  attff;                  // VGA attribute flip-flop

		u32 blt_regs[0x20];         // 2D Blitter registers
		u32 blt_dst_base = 0;
		u32 blt_dst_x = 0;
		u32 blt_dst_y = 0;
		u32 blt_dst_width = 0;
		u32 blt_dst_height = 0;
		u32 blt_dst_stride = 0;
		u32 blt_dst_bpp = 0;
		u32 blt_cmd = 0;
		u32 blt_src_base = 0;
		u32 blt_src_x = 0;
		u32 blt_src_y = 0;
		u32 blt_src_width = 0;
		u32 blt_src_height = 0;
		u32 blt_src_stride = 0;
		u32 blt_src_bpp = 0;
	};

	void check_stalled_cpu(attotime current_time);
	void flush_fifos(attotime current_time);
	void init_fbi(fbi_state *f, void *memory, int fbmem);
	s32 register_w(offs_t offset, u32 data);
	s32 swapbuffer(u32 data);
	s32 lfb_w(offs_t offset, u32 data, u32 mem_mask);
	u32 lfb_r(offs_t offset, bool lfb_3d);
	s32 texture_w(offs_t offset, u32 data);
	s32 lfb_direct_w(offs_t offset, u32 data, u32 mem_mask);
	s32 banshee_2d_w(offs_t offset, u32 data);
	void stall_cpu(int state, attotime current_time);
	void soft_reset();
	void recompute_video_memory();
	void adjust_vblank_timer();
	s32 fastfill();
	s32 triangle();
	s32 begin_triangle();
	s32 draw_triangle();
	s32 setup_and_draw_triangle();

	void accumulate_statistics(const voodoo::thread_stats_block &block);
	void update_statistics(bool accumulate);
	void reset_counters();

	u32 register_r(offs_t offset);

	void swap_buffers();
	int cmdfifo_compute_expected_depth(cmdfifo_info &f);
	u32 cmdfifo_execute(cmdfifo_info *f);
	s32 cmdfifo_execute_if_ready(cmdfifo_info &f);
	void cmdfifo_w(cmdfifo_info *f, offs_t offset, u32 data);

	void init_save_state();

	void banshee_blit_2d(u32 data);

private:
	// internal helpers
	u16 *draw_buffer(int index) const { return (u16 *)(m_fbi.ram + m_fbi.rgboffs[index]); }
	u16 *front_buffer() const { return draw_buffer(m_fbi.frontbuf); }
	u16 *back_buffer() const { return draw_buffer(m_fbi.backbuf); }
	u16 *aux_buffer() const { return (m_fbi.auxoffs != ~0) ? (u16 *)(m_fbi.ram + m_fbi.auxoffs) : nullptr; }

// FIXME: this stuff should not be public
public:
	voodoo::voodoo_regs m_reg;             // raw registers
	const u8 m_type;                // type of system
	u8 m_alt_regmap;             // enable alternate register map?
	u8 m_chipmask;               // mask for which chips are available
	u8 m_index;                  // index of board

	screen_device *m_screen;               // the screen we are acting on
	cpu_device *m_cpu;                  // the CPU we interact with
	u32 m_freq;                   // operating frequency
	attoseconds_t m_attoseconds_per_cycle;  // attoseconds per cycle
	int m_trigger;                // trigger used for stalling

	const u8 *m_regaccess;              // register access array
	const char *const *m_regnames;               // register names array

	std::unique_ptr<voodoo::voodoo_renderer> m_renderer;              // polygon manager

	voodoo_stats m_stats;                  // internal statistics

	offs_t m_last_status_pc;         // PC of last status description (for logging)
	u32 m_last_status_value;      // value of last status read (for logging)

	bool m_send_config;
	u32 m_tmu_config;

	u8 m_fbmem;
	u8 m_tmumem0;
	u8 m_tmumem1;
	devcb_write_line m_vblank;
	devcb_write_line m_stall;
	// This is for internally generated PCI interrupts in Voodoo3
	devcb_write_line m_pciint;

	pci_state m_pci;                    // PCI state
	dac_state m_dac;                    // DAC state
	fbi_state m_fbi;                    // FBI states
	tmu_state m_tmu[MAX_TMU];           // TMU states
	tmu_shared_state m_tmushare;               // TMU shared state
	banshee_info m_banshee;                // Banshee state

	optional_device<screen_device> m_screen_finder; // the screen we are acting on
	optional_device<cpu_device> m_cpu_finder;   // the CPU we interact with

	std::unique_ptr<u8[]> m_fbmem_alloc;
	std::unique_ptr<u8[]> m_tmumem_alloc[2];
};

class voodoo_1_device : public voodoo_device
{
public:
	voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class voodoo_2_device : public voodoo_device
{
public:
	voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class voodoo_banshee_device : public voodoo_device
{
public:
	voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u32 banshee_r(offs_t offset, u32 mem_mask = ~0);
	void banshee_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_fb_r(offs_t offset);
	void banshee_fb_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_io_r(offs_t offset, u32 mem_mask = ~0);
	void banshee_io_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_rom_r(offs_t offset);
	u8 banshee_vga_r(offs_t offset);
	void banshee_vga_w(offs_t offset, u8 data);

protected:
	voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 vdt);

	// device-level overrides
	u32 banshee_agp_r(offs_t offset);
	void banshee_agp_w(offs_t offset, u32 data, u32 mem_mask = ~0);
};


class voodoo_3_device : public voodoo_banshee_device
{
public:
	voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(VOODOO_1,       voodoo_1_device)
DECLARE_DEVICE_TYPE(VOODOO_2,       voodoo_2_device)
DECLARE_DEVICE_TYPE(VOODOO_BANSHEE, voodoo_banshee_device)
DECLARE_DEVICE_TYPE(VOODOO_3,       voodoo_3_device)

#endif // MAME_VIDEO_VOODOO_H
