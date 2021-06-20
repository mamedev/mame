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

}


//**************************************************************************
//  VOODOO DEVICES
//**************************************************************************

class voodoo_device : public device_t
{
public:
	// destruction
	virtual ~voodoo_device();

	// configuration
	void set_fbmem(int value) { m_fbmem_in_mb = value; }
	void set_tmumem(int value1, int value2) { m_tmumem0_in_mb = value1; m_tmumem1_in_mb = value2; }
	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	auto vblank_callback() { return m_vblank.bind(); }
	auto stall_callback() { return m_stall.bind(); }
	auto pciint_callback() { return m_pciint.bind(); }

	// getters
	voodoo_model model() const { return m_model; }

	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0);

	TIMER_CALLBACK_MEMBER( vblank_off_callback );
	TIMER_CALLBACK_MEMBER( stall_cpu_callback );
	TIMER_CALLBACK_MEMBER( vblank_callback );

	virtual int update(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void set_init_enable(u32 newval);

	voodoo::voodoo_renderer &renderer() { return *m_renderer; }

protected:
	// construction
	voodoo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

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
		tmu_state(voodoo_device &device) : m_device(device), m_reg(device.model()) { }

		void init(int index, voodoo::shared_tables &share, std::vector<u8> &memory);
		voodoo::rasterizer_texture &prepare_texture();
		s32 compute_lodbase();
		void texture_w(offs_t offset, u32 data, bool seq_8_downld);
		void ncc_w(offs_t offset, u32 data);

		voodoo_device &m_device;        // reference back to our owner
		int m_index;                    // index of ourself
		u8 *m_ram = nullptr;            // pointer to our RAM
		u32 m_mask = 0;                 // mask to apply to pointers

		voodoo::voodoo_regs m_reg;      // TMU registers
		bool m_regdirty;                // true if the LOD/mode/base registers have changed

		s64 m_starts = 0, m_startt = 0; // starting S,T (14.18)
		s64 m_startw = 0;               // starting W (2.30)
		s64 m_dsdx = 0, m_dtdx = 0;     // delta S,T per X
		s64 m_dwdx = 0;                 // delta W per X
		s64 m_dsdy = 0, m_dtdy = 0;     // delta S,T per Y
		s64 m_dwdy = 0;                 // delta W per Y

		rgb_t *m_texel[16];             // texel lookups for each format

		bool m_palette_dirty[4];        // true if palette (0-1) or NCC (2-3) is dirty
		rgb_t m_palette[2][256];        // 2 versions of the palette
	};


	struct fbi_state
	{
		void init(voodoo_model model, std::vector<u8> &memory);

		struct setup_vertex
		{
			float x, y;                   // X, Y coordinates
			float z, wb;                  // Z and broadcast W values
			float r, g, b, a;             // A, R, G, B values
			float s0, t0, w0;             // W, S, T for TMU 0
			float s1, t1, w1;             // W, S, T for TMU 1
		};

		u8 *m_ram;          // pointer to frame buffer RAM
		u32 m_mask;               // mask to apply to pointers
		u32 m_rgboffs[3]; // word offset to 3 RGB buffers
		u32 m_auxoffs;            // word offset to 1 aux buffer

		u8 m_frontbuf;           // front buffer index
		u8 m_backbuf;            // back buffer index
		u8 m_swaps_pending;      // number of pending swaps
		bool m_video_changed;      // did the frontbuffer video change?

		u32 m_yorigin = 0;            // Y origin subtract value
		u32 m_lfb_base = 0;           // base of LFB in memory
		u8  m_lfb_stride = 0;         // stride of LFB accesses in bits

		u32 m_width = 0;              // width of current frame buffer
		u32 m_height = 0;             // height of current frame buffer
		u32 m_xoffs = 0;              // horizontal offset (back porch)
		u32 m_yoffs = 0;              // vertical offset (back porch)
		u32 m_vsyncstart = 0;         // vertical sync start scanline
		u32 m_vsyncstop = 0;          // vertical sync stop
		u32 m_rowpixels = 0;          // pixels per row

		u8 m_vblank = 0;             // VBLANK state
		u8 m_vblank_count = 0;       // number of VBLANKs since last swap
		u8 m_vblank_swap_pending = 0;// a swap is pending, waiting for a vblank
		u8 m_vblank_swap = 0;        // swap when we hit this count
		u8 m_vblank_dont_swap = 0;   // don't actually swap when we hit this point

		/* triangle setup info */
		s32 m_sign;                   // triangle sign
		s16 m_ax, m_ay;                 // vertex A x,y (12.4)
		s16 m_bx, m_by;                 // vertex B x,y (12.4)
		s16 m_cx, m_cy;                 // vertex C x,y (12.4)
		s32 m_startr, m_startg, m_startb, m_starta; // starting R,G,B,A (12.12)
		s32 m_startz;                 // starting Z (20.12)
		s64 m_startw;                 // starting W (16.32)
		s32 m_drdx, m_dgdx, m_dbdx, m_dadx; // delta R,G,B,A per X
		s32 m_dzdx;                   // delta Z per X
		s64 m_dwdx;                   // delta W per X
		s32 m_drdy, m_dgdy, m_dbdy, m_dady; // delta R,G,B,A per Y
		s32 m_dzdy;                   // delta Z per Y
		s64 m_dwdy;                   // delta W per Y

		voodoo::thread_stats_block m_lfb_stats;              // LFB-access statistics

		u8 m_sverts = 0;             // number of vertices ready */
		setup_vertex m_svert[3];               // 3 setup vertices */

		voodoo::fifo_state m_fifo;                   // framebuffer memory fifo */
		cmdfifo_info m_cmdfifo[2];             // command FIFOs */

		rgb_t m_pen[65536];             // mapping from pixels to pens */
		rgb_t m_clut[512];              // clut gamma data */
		u8 m_clut_dirty = 1;         // do we need to recompute? */

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
	};


	struct dac_state
	{
		void data_w(u8 regum, u8 data);
		void data_r(u8 regnum);

		u8 m_reg[8];                 // 8 registers
		u8 read_result;            // pending read result
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
	void stall_cpu(int state, attotime current_time);
	void soft_reset();
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

protected:
	// internal helpers
	void register_save();
	virtual s32 banshee_2d_w(offs_t offset, u32 data);
	int update_common(bitmap_rgb32 &bitmap, const rectangle &cliprect, rgb_t const *pens);

	// internal state
	const voodoo_model m_model;              // which voodoo model
	u8 m_chipmask;                           // mask for which chips are available
	u8 m_fbmem_in_mb;                        // framebuffer memory, in MB
	u8 m_tmumem0_in_mb;                      // TMU0 memory, in MB
	u8 m_tmumem1_in_mb;                      // TMU1 memory, in MB

	std::unique_ptr<voodoo::voodoo_renderer> m_renderer; // rendering class
	voodoo::voodoo_regs m_reg;               // raw registers

	int m_trigger;                           // trigger used for stalling

	offs_t m_last_status_pc;                 // PC of last status description (for logging)
	u32 m_last_status_value;                 // value of last status read (for logging)

	devcb_write_line m_vblank;               // VBLANK callback
	devcb_write_line m_stall;                // stalling callback
	devcb_write_line m_pciint;               // PCI interrupt callback

	pci_state m_pci;                         // PCI state
	dac_state m_dac;                         // DAC state
	fbi_state m_fbi;                         // FBI states
	tmu_state m_tmu[MAX_TMU];                // TMU states

	required_device<screen_device> m_screen; // the screen we are acting on
	required_device<cpu_device> m_cpu;       // the CPU we interact with

	emu_timer *m_vsync_stop_timer = nullptr; // VBLANK end timer
	emu_timer *m_vsync_start_timer = nullptr;// VBLANK timer

	voodoo::debug_stats m_stats;             // internal statistics

	std::vector<u8> m_fbmem;                 // allocated framebuffer
	std::vector<u8> m_tmumem[2];             // allocated texture memory
	std::unique_ptr<voodoo::shared_tables> m_shared; // shared tables
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

	virtual s32 banshee_2d_w(offs_t offset, u32 data) override;
	virtual int update(bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	// construction
	voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model);

	// device-level overrides
	virtual void device_start() override;

	// device-level overrides
	u32 banshee_agp_r(offs_t offset);
	void banshee_agp_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void banshee_blit_2d(u32 data);

	// internal state
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
	banshee_info m_banshee;                  // Banshee state
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
