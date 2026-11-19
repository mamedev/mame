// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/***************************************************************************

    NEC / VideoLogic PowerVR 250 (PMX1, sold on cards as Neon 250)

***************************************************************************/

#ifndef MAME_VIDEO_POWERVR250_H
#define MAME_VIDEO_POWERVR250_H

#pragma once

#include "cpu/pmx1/pmx1core.h"


class powervr250_device : public device_t, public device_video_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// the clock is the reference input of the PLLs, usually a 14.31818MHz crystal
	powervr250_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~powervr250_device();

	// configuration
	void set_vram_size(u32 bytes) { m_vram_size = bytes; }
	auto irq_callback() { return m_irq_cb.bind(); }

	// host bus: immediate registers, PCI configuration space and frame buffer
	void map(address_map &map) ATTR_COLD;
	u32 config_r(offs_t offset);
	void config_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 fb_r(offs_t offset);
	void fb_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	// implicit registers
	enum : u16
	{
		REG_THREAD_ENABLE   = 0x000,
		REG_THREAD_RECOVER  = 0x008,
		REG_RESET_VECTOR_LO = 0x010,
		REG_RESET_VECTOR_HI = 0x011,
		REG_SOFT_INTERRUPT  = 0x012,
		REG_VIDEO_CLOCK_DIV = 0x013,
		REG_DAC_FLOW        = 0x0a0,
		REG_DAC_HS_FRONT    = 0x0a3,
		REG_DAC_HS_BACK     = 0x0a4,
		REG_DAC_HS_WIDTH    = 0x0a5,
		REG_DAC_SYNC        = 0x0a6,
		REG_DAC_CONTROL     = 0x0a9,
		REG_THREAD_KICKER   = 0x100,
		REG_INDEX_ADDRESS   = 0x104,
		REG_INDEX_DATA      = 0x105,
		REG_IIC_A           = 0x118,
		REG_IIC_B           = 0x119,
		REG_DAC_PLL0        = 0x120,
		REG_DAC_PLL1        = 0x121,
		REG_DAC_LINE_COMPARE = 0x128,
		REG_DAC_LINE_COUNT  = 0x129,
		REG_INTERRUPT_STATUS = 0x14c,
		REG_INTERRUPT_MASK  = 0x14d,
		REG_VGA_PAGE        = 0x168,
		REG_SOFTWARE_RESET  = 0x169,
		REG_PCI_DEVICE_ID   = 0x258,
		REG_PCI_VENDOR_ID   = 0x259,
		REG_SDRAM_CONFIG0   = 0x280,
		REG_3D_ADDRESS_BASE = 0x286,
		REG_FB_OFFSET       = 0x287
	};

	// interrupt status and mask
	enum : u16
	{
		IRQ_DAC_LINE = 1U << 0,
		IRQ_DAC_VS   = 1U << 2,
		IRQ_PVR      = 1U << 7,
		IRQ_C1_0     = 1U << 8,
		IRQ_MASTER   = 1U << 15
	};

	// indexed registers of the 3D core
	enum : u16
	{
		REG3D_START_RENDER      = 0x014,
		REG3D_OBJECT_BASE       = 0x020,
		REG3D_REGION_BASE       = 0x02c,
		REG3D_INDEX_REF         = 0x040,
		REG3D_WRITE_CONTROL1    = 0x048,
		REG3D_WRITE_STRIDE      = 0x04c,
		REG3D_INDEX_FB_OFFSET   = 0x054,
		REG3D_TEXTURE_REF       = 0x05c,
		REG3D_TEXTURE_FB_OFFSET = 0x064,
		REG3D_WRITE_X_CLIP      = 0x068,
		REG3D_WRITE_Y_CLIP      = 0x06c,
		REG3D_SHADOW_SCALE      = 0x074,
		REG3D_CULL_VALUE        = 0x078,
		REG3D_HALF_OFFSET       = 0x080,
		REG3D_BACKGROUND_DEPTH  = 0x088,
		REG3D_BACKGROUND_TAG    = 0x08c,
		REG3D_SORT_CONTROL      = 0x098,
		REG3D_WRITE_CONTROL2    = 0x0a4,
		REG3D_FOG_TABLE_COLOR   = 0x0b0,
		REG3D_FOG_VERTEX_COLOR  = 0x0b4,
		REG3D_FOG_DENSITY       = 0x0b8,
		REG3D_FOG_CLAMP_MAX     = 0x0bc,
		REG3D_FOG_CLAMP_MIN     = 0x0c0,
		REG3D_TEXTURE_CONTROL   = 0x0e4,
		REG3D_SCALER_CONTROL    = 0x0f4,
		REG3D_FOG_TABLE         = 0x200
	};

	// tile accelerator: the mode bits of instruction 2 and the flags of a status word
	enum : u32
	{
		TA_MTILE_OLAP = 1U << 0,
		TA_AA_X       = 1U << 1,
		TA_HALFCULL   = 1U << 2,
		TA_CULL       = 1U << 3,
		TA_Z          = 1U << 4,
		TA_COPY       = 1U << 5,
		TA_SHAD       = 1U << 6,
		TA_AA_Y       = 1U << 7,

		TA_CO         = 1U << 30,
		TA_EOL        = 1U << 31
	};

	// a strip has up to eight vertices, each of up to 18 words
	static constexpr u32 TA_STRIP_WORDS = 8 * 18;

	static constexpr int TILE_WIDTH = 32;
	static constexpr int TILE_HEIGHT = 16;

	struct tile_pass;
	class renderer;

	// a rectangle of tiles, or none when its left is past its right
	struct ta_box
	{
		u8 left = 1;
		u8 right = 0;
		u8 top = 1;
		u8 bottom = 0;
	};

	struct dac_span
	{
		u32 address = 0;    // of the first pixel in the frame buffer
		u16 line = 0;       // first line, from the start of frame
		u16 lines = 0;
		u16 pixels = 0;
		u8 format = 0;
		bool blank = false;
	};

	static constexpr u32 MAX_SPANS = 64;
	static constexpr u32 PAGE_SHIFT = 12;

	// 2D core: registers that the primary core writes through its port registers
	enum : u8
	{
		P2D_SRC_POS = 0x24,         // source x << 16 | y
		P2D_LINE_START = 0x31,      // first pixel of a line, x << 16 | y
		P2D_LINE_DIAGONAL = 0x32,   // step of a line along both axes, x << 16 | y
		P2D_LINE_STEP = 0x33,       // step of a line along its main axis, x << 16 | y
		P2D_DEST_POS = 0x34,        // destination x << 16 | y
		P2D_START = 0x38,           // (width in 64-bit words - 1) << 16 | lines, or for a line pixels << 16 | error term
		P2D_LINE_ERROR = 0x3b,      // what a diagonal step of a line adds to its error term
		P2D_SRC_A = 0x3e,           // source of a copy, background of a blend
		P2D_SRC_B = 0x3f,           // foreground of a blend
		P2D_PITCH_A = 0x41,
		P2D_PITCH_B = 0x42,
		P2D_OPERATION = 0x4b,
		P2D_FILL_COLOR = 0x4d,
		P2D_LINE_COLOR = 0x4f,
		P2D_CLIP = 0x52,            // height << 16 | width
		P2D_SIZE = 0x53,            // width << 16 | x
		P2D_DEST = 0x54,
		P2D_DEST_PITCH = 0x55
	};

	template <u16 Reg> u16 immediate_r(offs_t offset);
	template <u16 Reg> void immediate_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void write_access_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void host_implicit_w(offs_t offset, u16 data, u16 mem_mask);
	TIMER_CALLBACK_MEMBER(host_implicit_sync);
	u16 implicit_r(offs_t offset);
	void implicit_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void index_data_w(u16 data);
	void ta_instruction_w(u64 data);
	u64 ta_status_r();
	void ta_next_status();
	bool ta_next_strip();
	bool ta_list_word(u32 &data);
	u32 ta_find_tile();
	void dac_feed_w(u64 data);
	int dac_busy_r();

	u32 port_r(offs_t offset);
	void port_w(offs_t offset, u32 data);
	void draw_2d();

	void update_irq();
	void set_irq(u16 bits);
	void map_frame_buffer();
	u32 pll_frequency(u32 m, u32 n, u32 p) const;
	u32 core_frequency() const;
	u32 pixel_frequency() const;
	u64 vram_qword(u32 address) const;
	u32 *memory(u32 address, u32 words) const;
	void written(u32 offset, u32 bytes);
	void settle(u32 offset, u32 bytes);
	u32 reg3d(u16 index) const { return m_regs3d[index >> 2]; }
	TIMER_CALLBACK_MEMBER(dac_span_done);
	TIMER_CALLBACK_MEMBER(render_done);

	required_device<pmx1core_device> m_core;
	devcb_write_line m_irq_cb;

	u32 m_vram_size;
	std::unique_ptr<u32 []> m_vram;
	std::unique_ptr<renderer> m_renderer;

	// which render was the last one started when a page of the memory was last written; not saved, it only spares the renderer work
	std::unique_ptr<u32 []> m_page_written;
	u32 m_render_serial;

	// pages that a render still being drawn reads or writes
	std::unique_ptr<u8 []> m_page_in_use;
	memory_passthrough_handler m_core_write_tap;

	emu_timer *m_dac_timer;
	emu_timer *m_render_timer;

	u32 m_config[0x40];
	u8 m_core_pll[4];
	u16 m_implicit[0x400];
	u32 m_war;
	u32 m_rar[2];
	u32 m_regs3d[0x200];
	u16 m_index_low;            // first half of a value that comes through the index data register in two
	bool m_index_low_held;
	bool m_irq_level;           // of the line to the host
	u8 m_fb_mapped;

	// tile accelerator: what the instructions gave it
	u32 m_ta_list_page[4];
	u64 m_ta_setup;             // instruction 2
	u64 m_ta_clip;              // instruction 3
	u32 m_ta_copy_dest;
	u32 m_ta_copy_limit;        // in words
	u32 m_ta_copy_words;        // copied since the destination was given

	// where it is in the control list
	bool m_ta_tiling;
	u16 m_ta_list_words;
	u16 m_ta_list_position;
	u32 m_ta_vertex_page[2];    // the page the strips start in and the one after it
	u32 m_ta_header[5];         // ISP/TSP words that the strips share
	u8 m_ta_header_words;

	// the strip in hand
	u32 m_ta_pointer;           // its object pointer as it goes out, without the triangles
	ta_box m_ta_triangle[6];    // the tiles each triangle is in
	ta_box m_ta_strip;          // those of all of them together
	u8 m_ta_tile_x;             // the tile to go on from
	u8 m_ta_tile_y;
	u64 m_ta_status;            // what the next read gets

	u32 m_dac_reg[4];
	u32 m_dac_pll;              // the setting of the pixel clock PLL that is in effect
	bool m_dac_busy;
	u16 m_dac_line;
	u16 m_dac_htotal;
	dac_span m_dac_feeding[MAX_SPANS];
	dac_span m_dac_showing[MAX_SPANS];
	u8 m_dac_feeding_count;
	bool m_dac_after_blank;     // the span fed before was blank

	u32 m_2d[256];              // 2D core registers
	u8 m_dac_showing_count;
};

DECLARE_DEVICE_TYPE(POWERVR250, powervr250_device)

#endif // MAME_VIDEO_POWERVR250_H
