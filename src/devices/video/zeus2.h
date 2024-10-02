// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Midway Zeus2 Video

**************************************************************************/
#ifndef MAME_VIDEO_ZEUS2_H
#define MAME_VIDEO_ZEUS2_H

#pragma once

#include "video/poly.h"
#include "video/rgbutil.h"
#include "cpu/tms32031/tms32031.h"

/*************************************
*  Constants
*************************************/
#define ZEUS2_VIDEO_CLOCK     XTAL(66'666'700)

#define DUMP_WAVE_RAM       0
#define TRACK_REG_USAGE     0
#define PRINT_TEX_INFO      0

#define WAVERAM0_WIDTH      1024
#define WAVERAM0_HEIGHT     2048

#define WAVERAM1_WIDTH      512
#define WAVERAM1_HEIGHT     1024

/*************************************
*  Type definitions
*************************************/

struct zeus2_poly_extra_data
{
	const void *    palbase;
	const void *    texbase;
	uint16_t          solidcolor;
	uint16_t          transcolor;
	uint16_t          texwidth;
	uint16_t          color;
	uint32_t          srcAlpha;
	uint32_t          dstAlpha;
	uint32_t          ctrl_word;
	uint32_t          ucode_src;
	uint32_t          tex_src;
	bool            texture_alpha;
	bool            texture_rgb555;
	bool            blend_enable;
	int32_t         zbuf_min;
	bool            depth_min_enable;
	bool            depth_test_enable;
	bool            depth_write_enable;
	bool            depth_clear_enable;

	uint8_t(*get_texel)(const void *, int, int, int);
	uint8_t(*get_alpha)(const void *, int, int, int);
};

/*************************************
*  Macros
*************************************/

#define WAVERAM_BLOCK0(blocknum)                ((void *)((uint8_t *)m_waveram.get() + 8 * (blocknum)))
#define WAVERAM_BLOCK0_EXT(blocknum)            ((void *)((uint8_t *)m_state->m_waveram.get() + 8 * (blocknum)))

#define WAVERAM_PTR8(base, bytenum)             ((uint8_t *)(base) + BYTE4_XOR_LE(bytenum))
#define WAVERAM_READ8(base, bytenum)            (*WAVERAM_PTR8(base, bytenum))
#define WAVERAM_WRITE8(base, bytenum, data)     do { *WAVERAM_PTR8(base, bytenum) = (data); } while (0)

#define WAVERAM_PTR16(base, wordnum)            ((uint16_t *)(base) + BYTE_XOR_LE(wordnum))
#define WAVERAM_READ16(base, wordnum)           (*WAVERAM_PTR16(base, wordnum))
#define WAVERAM_WRITE16(base, wordnum, data)    do { *WAVERAM_PTR16(base, wordnum) = (data); } while (0)

#define WAVERAM_PTR32(base, dwordnum)           ((uint32_t *)(base) + (dwordnum))
#define WAVERAM_READ32(base, dwordnum)          (*WAVERAM_PTR32(base, dwordnum))
#define WAVERAM_WRITE32(base, dwordnum, data)   do { *WAVERAM_PTR32(base, dwordnum) = (data); } while (0)

/*************************************
*  Polygon renderer
*************************************/
class zeus2_device;

class zeus2_renderer : public poly_manager<float, zeus2_poly_extra_data, 4>
{
public:
	zeus2_renderer(zeus2_device *state);

	void render_poly_8bit(int32_t scanline, const extent_t& extent, const zeus2_poly_extra_data& object, int threadid);

	void zeus2_draw_quad(const uint32_t *databuffer, uint32_t texdata, int logit);

private:
	zeus2_device* m_state;
};
typedef zeus2_renderer::vertex_t z2_poly_vertex;
typedef zeus2_renderer::extent_t z2_poly_extent;

/*************************************
*  Zeus2 Video Device
*************************************/
class zeus2_device : public device_t, public device_video_interface
{
public:
	zeus2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t zeus2_r(offs_t offset);
	void zeus2_w(offs_t offset, uint32_t data);
	TIMER_CALLBACK_MEMBER(display_irq_off);
	TIMER_CALLBACK_MEMBER(display_irq);

	auto vblank_callback() { return m_vblank.bind(); }
	auto irq_callback() { return m_irq.bind(); }

	devcb_write_line   m_vblank;
	devcb_write_line   m_irq;

	void set_float_mode(int mode) { m_atlantis = mode; }
	int m_atlantis; // Used to switch to using IEEE754 floating point format for atlantis

	uint32_t m_zeusbase[0x80];
	uint32_t m_renderRegs[0x50];

	std::unique_ptr<zeus2_renderer> poly;

	rectangle zeus_cliprect;

	int m_palSize;
	float zeus_matrix[3][3];
	float zeus_trans[4];
	float zeus_light[3];
	uint32_t zeus_texbase;
	int zeus_quad_size;
	bool m_useZOffset;

	std::unique_ptr<uint32_t[]> m_waveram;
	std::unique_ptr<uint32_t[]> m_frameColor;
	std::unique_ptr<int32_t[]> m_frameDepth;
	uint32_t m_pal_table[0x100];
	uint32_t m_ucode[0x200];
	uint32_t m_curUCodeSrc;
	uint32_t m_curPalTableSrc;
	uint32_t m_texmodeReg;

	emu_timer *int_timer;
	emu_timer *vblank_timer;
	emu_timer *vblank_off_timer;
	int yoffs;
	int texel_width;
	float zbase;

	enum { THEGRID, CRUSNEXO, MWSKINS };
	int m_system;
#if PRINT_TEX_INFO
	void check_tex(uint32_t &texmode, float &zObj, float &zMat, float &zOff);
	std::string tex_info(void);
#endif

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(int_timer_callback);
	void zeus2_register32_w(offs_t offset, uint32_t data, int logit);
	void zeus2_register_update(offs_t offset, uint32_t oldval, int logit);
	bool zeus2_fifo_process(const uint32_t *data, int numwords);
	void zeus2_pointer_write(uint8_t which, uint32_t value, int logit);
	void load_pal_table(void *wavePtr, uint32_t ctrl, int type, int logit);
	void load_ucode(void *wavePtr, uint32_t ctrl, int logit);
	void zeus2_draw_model(uint32_t baseaddr, uint16_t count, int logit);
	void log_fifo_command(const uint32_t *data, int numwords, const char *suffix);
	void print_fifo_command(const uint32_t *data, int numwords, const char *suffix);
	void log_render_info(uint32_t texdata);

	/*************************************
	*  Member variables
	*************************************/
	uint8_t log_fifo;

	uint32_t zeus_fifo[20];
	uint8_t zeus_fifo_words;

	uint32_t m_fill_color;
	int32_t m_fill_depth;

	int m_yScale;

#if PRINT_TEX_INFO
	std::map<uint32_t, std::string> tex_map;
#endif

#if TRACK_REG_USAGE
	struct reg_info
	{
		struct reg_info *next;
		uint32_t value;
	};

	reg_info *regdata[0x80];
	int regdata_count[0x80];
	int regread_count[0x80];
	int regwrite_count[0x80];
	reg_info *subregdata[0x100];
	int subregdata_count[0x80];
	int subregwrite_count[0x100];
#endif
public:
	/*************************************
	*  Inlines for block addressing
	*************************************/
	inline float convert_float(uint32_t val)
	{
		if (m_atlantis) {
			return reinterpret_cast<float&>(val);
		}
		else
			return tms3203x_device::fp_to_float(val);
	}

	inline uint32_t frame_addr_from_xy(uint32_t x, uint32_t y, bool render)
	{
		uint32_t addr;
		if (render) {
			// Rendering is y location
			addr = m_renderRegs[0x4] << (9 + m_yScale);
		}
		else {
			// y.16:x.16 row/col
			// Ignore col for now
			addr = m_zeusbase[0x38] >> (16 - 9 - 2 * m_yScale);
		}
		//uint32_t addr = render ? frame_addr_from_phys_addr(m_renderRegs[0x4] << (15 + m_yScale))
		//  : frame_addr_from_phys_addr((m_zeusbase[0x38] >> 1) << (m_yScale << 1));
		addr += (y << (9 + m_yScale)) + x;
		return addr;
	}

	// Convert 0xRRRRCCCC to frame buffer address
	//inline uint32_t frame_addr_from_expanded_addr(uint32_t addr)
	//{
	//  return (((addr & 0x3ff0000) >> (16 - 9 + 1)) | (addr & 0x1ff)) << 1;
	//}

	// Convert Physical 0xRRRRCCCC to frame buffer address
	// Based on address reg 51 (no scaling)
	inline uint32_t frame_addr_from_phys_addr(uint32_t physAddr)
	{
		uint32_t addr = (((physAddr & 0x3ff0000) >> (16 - 9)) | (physAddr & 0x1ff)) << 1;
		return addr;
	}

	// Read from frame buffer
	inline void frame_read()
	{
		uint32_t addr = frame_addr_from_phys_addr(m_zeusbase[0x51]);
		m_zeusbase[0x58] = m_frameColor[addr];
		m_zeusbase[0x59] = m_frameColor[addr + 1];
		m_zeusbase[0x5a] = *(uint32_t*)&m_frameDepth[addr];
		if (m_zeusbase[0x5e] & 0x40)
		{
			m_zeusbase[0x51]++;
			m_zeusbase[0x51] += (m_zeusbase[0x51] & 0x200) << 7;
			m_zeusbase[0x51] &= ~0xfe00;
		}
	}

	// Write to frame buffer
	inline void frame_write()
	{
		uint32_t addr = frame_addr_from_phys_addr(m_zeusbase[0x51]);
		if (m_zeusbase[0x57] & 0x1)
			m_frameColor[addr] = m_zeusbase[0x58];
		if (m_zeusbase[0x5e] & 0x20) {
			if (m_zeusbase[0x57] & 0x4)
				m_frameColor[addr + 1] = m_zeusbase[0x5a];
		} else
		{
			if (m_zeusbase[0x57] & 0x4)
				m_frameColor[addr + 1] = m_zeusbase[0x59];
			if (m_zeusbase[0x57] & 0x10)
				*(uint32_t*)&m_frameDepth[addr] = m_zeusbase[0x5a];
		}

		if (m_zeusbase[0x5e] & 0x40)
		{
			m_zeusbase[0x51]++;
			m_zeusbase[0x51] += (m_zeusbase[0x51] & 0x200) << 7;
			m_zeusbase[0x51] &= ~0xfe00;
		}
	}

	inline void *waveram0_ptr_from_expanded_addr(uint32_t addr)
	{
		uint32_t blocknum = (addr % WAVERAM0_WIDTH) + ((addr >> 16) % WAVERAM0_HEIGHT) * WAVERAM0_WIDTH;
		return WAVERAM_BLOCK0(blocknum);
	}

	[[maybe_unused]] inline void *waveram0_ptr_from_texture_addr(uint32_t addr, int width)
	{
		uint32_t blocknum = ((addr & ~1) * width) / 8;
		return WAVERAM_BLOCK0(blocknum);
	}

	/*************************************
	*  Inlines for rendering
	*************************************/
	inline uint32_t conv_rgb555_to_rgb32(uint16_t color)
	{
		return ((color & 0x7c00) << 9) | ((color & 0x3e0) << 6) | ((color & 0x1f) << 3);
	}

	inline uint32_t conv_rgb565_to_rgb32(uint16_t color)
	{
		return ((color & 0x7c00) << 9) | ((color & 0x3e0) << 6) | ((color & 0x8000) >> 5) | ((color & 0x1f) << 3);
	}
	inline uint32_t conv_rgb332_to_rgb32(uint8_t color)
	{
		uint32_t result;
		result =  ((((color) >> 0) & 0xe0) | (((color) >> 3) & 0x1c) | (((color) >> 6) & 0x03)) << 16;
		result |= ((((color) << 3) & 0xe0) | (((color) >> 0) & 0x1c) | (((color) >> 3) & 0x03)) << 8;
		result |= ((((color) << 6) & 0xc0) | (((color) << 4) & 0x30) | (((color) << 2) & 0x0c) | (((color) << 0) & 0x03)) << 0;
		return result;
	}

	[[maybe_unused]] inline void WAVERAM_plot(int y, int x, uint32_t color)
	{
		if (zeus_cliprect.contains(x, y))
		{
			uint32_t addr = frame_addr_from_xy(x, y, true);
			m_frameColor[addr] = color;
		}
	}

	[[maybe_unused]] inline void waveram_plot_depth(int y, int x, uint32_t color, int32_t depth)
	{
		if (zeus_cliprect.contains(x, y))
		{
			uint32_t addr = frame_addr_from_xy(x, y, true);
			m_frameColor[addr] = color;
			m_frameDepth[addr] = depth;
		}
	}

	[[maybe_unused]] inline void waveram_plot_check_depth(int y, int x, uint32_t color, int32_t depth)
	{
		if (zeus_cliprect.contains(x, y))
		{
			uint32_t addr = frame_addr_from_xy(x, y, true);
			int32_t *depthptr = &m_frameDepth[addr];
			if (depth <= *depthptr)
			{
				m_frameColor[addr] = color;
				*depthptr = depth;
			}
		}
	}

	[[maybe_unused]] inline void waveram_plot_check_depth_nowrite(int y, int x, uint32_t color, int32_t depth)
	{
		if (zeus_cliprect.contains(x, y))
		{
			uint32_t addr = frame_addr_from_xy(x, y, true);
			int32_t *depthptr = &m_frameDepth[addr];
			if (depth <= *depthptr)
				m_frameColor[addr] = color;
		}
	}

	/*************************************
	*  Inlines for texel accesses
	*************************************/
	// 4x2 block size
	static inline uint8_t get_texel_4bit_4x2(const void *base, int y, int x, int width)
	{
		uint32_t byteoffs = (y / 2) * (width * 2) + ((x / 8) << 3) + ((y & 1) << 2) + ((x / 2) & 3);
		return (WAVERAM_READ8(base, byteoffs) >> (4 * (x & 1))) & 0x0f;
	}

	static inline uint8_t get_texel_8bit_4x2(const void *base, int y, int x, int width)
	{
		uint32_t byteoffs = (y / 2) * (width * 2) + ((x / 4) << 3) + ((y & 1) << 2) + (x & 3);
		return WAVERAM_READ8(base, byteoffs);
	}

	// 2x2 block size within 32 bits, 2 2x2 blocks stacked in y in 64 bits
	static inline uint8_t get_texel_4bit_2x2(const void *base, int y, int x, int width)
	{
		uint32_t byteoffs = (y / 4) * (width * 4) + ((x / 4) << 3) + ((y & 3) << 1) + ((x / 2) & 1);
		return (WAVERAM_READ8(base, byteoffs) >> (4 * (x & 1))) & 0x0f;
	}

	static inline uint8_t get_texel_8bit_2x2(const void *base, int y, int x, int width)
	{
		uint32_t byteoffs = (y / 4) * (width * 4) + ((x / 2) << 3) + ((y & 3) << 1) + (x & 1);
		return WAVERAM_READ8(base, byteoffs);
	}
	// 2x2 block size of texel, alpha in 64 bits
	// 8 Bit texel, 8 bit alpha
	static inline uint8_t get_texel_8bit_2x2_alpha(const void *base, int y, int x, int width)
	{
		uint32_t byteoffs = (y / 2) * (width * 2) + ((x / 2) << 2) + ((y & 1) << 1) + (x & 1);
		// Only grab RGB value for now
		byteoffs <<= 1;
		return WAVERAM_READ8(base, byteoffs + 0);
	}
	static inline uint8_t get_alpha_8bit_2x2_alpha(const void *base, int y, int x, int width)
	{
		uint32_t byteoffs = (y / 2) * (width * 2) + ((x / 2) << 2) + ((y & 1) << 1) + (x & 1);
		// Only grab Alpha value for now
		byteoffs <<= 1;
		return WAVERAM_READ8(base, byteoffs + 1);
	}
	// 2x2 block size of r5g5r5 in 64 bits
	static inline uint32_t get_rgb555(const void *base, int y, int x, int width)
	{
		uint32_t wordoffs = (y / 2) * (width * 2) + ((x / 2) << 2) + ((y & 1) << 1) + (x & 1);
		uint16_t color = WAVERAM_READ16(base, wordoffs);
		return ((color & 0x7c00) << 9) | ((color & 0x3e0) << 6) | ((color & 0x1f) << 3);
	}

};

// device type definition
DECLARE_DEVICE_TYPE(ZEUS2, zeus2_device)

#endif // MAME_VIDEO_ZEUS2
