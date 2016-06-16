// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Midway Zeus2 Video

**************************************************************************/
#ifndef __ZEUS2_H__
#define __ZEUS2_H__

#include "emu.h"
#include "video/poly.h"
#include "video/rgbutil.h"
#include "cpu/tms32031/tms32031.h"

#pragma once

/*************************************
*  Constants
*************************************/
#define MIDZEUS_VIDEO_CLOCK     XTAL_66_6667MHz

#define DUMP_WAVE_RAM       0
#define TRACK_REG_USAGE     0

#define WAVERAM0_WIDTH      1024
#define WAVERAM0_HEIGHT     2048

//#define WAVERAM1_WIDTH      512
#define WAVERAM1_WIDTH      512
#define WAVERAM1_HEIGHT     1024
//#define WAVERAM1_HEIGHT     1024

/*************************************
*  Type definitions
*************************************/

struct zeus2_poly_extra_data
{
	const void *    palbase;
	const void *    texbase;
	UINT16          solidcolor;
	INT16           zoffset;
	UINT16          transcolor;
	UINT16          texwidth;
	UINT16          color;
	UINT32          alpha;
};

/*************************************
*  Macros
*************************************/

#define WAVERAM_BLOCK0(blocknum)                ((void *)((UINT8 *)waveram[0] + 8 * (blocknum)))
#define WAVERAM_BLOCK1(blocknum)                ((void *)((UINT8 *)waveram[1] + 12 * (blocknum)))
#define WAVERAM_BLOCK0_EXT(blocknum)                ((void *)((UINT8 *)m_state.waveram[0] + 8 * (blocknum)))
#define WAVERAM_BLOCK1_EXT(blocknum)                ((void *)((UINT8 *)m_state.waveram[1] + 12 * (blocknum)))

#define WAVERAM_PTR8(base, bytenum)             ((UINT8 *)(base) + BYTE4_XOR_LE(bytenum))
#define WAVERAM_READ8(base, bytenum)            (*WAVERAM_PTR8(base, bytenum))
#define WAVERAM_WRITE8(base, bytenum, data)     do { *WAVERAM_PTR8(base, bytenum) = (data); } while (0)

#define WAVERAM_PTR16(base, wordnum)            ((UINT16 *)(base) + BYTE_XOR_LE(wordnum))
#define WAVERAM_READ16(base, wordnum)           (*WAVERAM_PTR16(base, wordnum))
#define WAVERAM_WRITE16(base, wordnum, data)    do { *WAVERAM_PTR16(base, wordnum) = (data); } while (0)

#define WAVERAM_PTR32(base, dwordnum)           ((UINT32 *)(base) + (dwordnum))
#define WAVERAM_READ32(base, dwordnum)          (*WAVERAM_PTR32(base, dwordnum))
#define WAVERAM_WRITE32(base, dwordnum, data)   do { *WAVERAM_PTR32(base, dwordnum) = (data); } while (0)

#define PIXYX_TO_DWORDNUM(y, x)                 (((((y) & 0x1ff) << 8) | (((x) & 0x1fe) >> 1)) * 3 + ((x) & 1))
#define DEPTHYX_TO_DWORDNUM(y, x)               (PIXYX_TO_DWORDNUM(y, (x) & ~1) + 2)

#define WAVERAM_PTRPIX(base, y, x)              WAVERAM_PTR32(base, PIXYX_TO_DWORDNUM(y, x))
#define WAVERAM_READPIX(base, y, x)             (*WAVERAM_PTRPIX(base, y, x))
#define WAVERAM_WRITEPIX(base, y, x, color)     do { *WAVERAM_PTRPIX(base, y, x) = (color);  } while (0)

#define WAVERAM_PTRDEPTH(base, y, x)            WAVERAM_PTR16(base, DEPTHYX_TO_DWORDNUM(y, x) * 2 + (x & 1))
#define WAVERAM_READDEPTH(base, y, x)           (*WAVERAM_PTRDEPTH(base, y, x))
#define WAVERAM_WRITEDEPTH(base, y, x, color)   do { *WAVERAM_PTRDEPTH(base, y, x) = (color);  } while (0)

/*************************************
*  Polygon renderer
*************************************/
class zeus2_device;

class zeus2_renderer : public poly_manager<float, zeus2_poly_extra_data, 4, 10000>
{
public:
	zeus2_renderer(zeus2_device &state);

	void render_poly_8bit(INT32 scanline, const extent_t& extent, const zeus2_poly_extra_data& object, int threadid);

	void zeus2_draw_quad(const UINT32 *databuffer, UINT32 texoffs, int logit);

private:
	zeus2_device& m_state;
};
typedef zeus2_renderer::vertex_t poly_vertex;
typedef zeus2_renderer::extent_t poly_extent;

/*************************************
*  Zeus2 Video Device
*************************************/
#define MCFG_ZEUS2_VBLANK_CB(_devcb) \
	devcb = &zeus2_device::set_vblank_callback(*device, DEVCB_##_devcb);

#define MCFG_ZEUS2_IRQ_CB(_devcb) \
	devcb = &zeus2_device::set_irq_callback(*device, DEVCB_##_devcb);

class zeus2_device : public device_t
{
public:
	zeus2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	screen_device *m_screen;              /* the screen we are acting on */

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_READ32_MEMBER( zeus2_r );
	DECLARE_WRITE32_MEMBER( zeus2_w );
	TIMER_CALLBACK_MEMBER(display_irq_off);
	TIMER_CALLBACK_MEMBER(display_irq);

	template<class _Object> static devcb_base &set_vblank_callback(device_t &device, _Object object) { return downcast<zeus2_device &>(device).m_vblank.set_callback(object); }
	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<zeus2_device &>(device).m_irq.set_callback(object); }
	devcb_write_line   m_vblank;
	devcb_write_line   m_irq;

	UINT32 m_zeusbase[400];

	zeus2_renderer* poly;

	void *zeus_renderbase;
	rectangle zeus_cliprect;

	float zeus_matrix[3][3];
	float zeus_point[3];
	float zeus_point2[3];
	UINT32 zeus_texbase;
	UINT32 zeus_unknown_40;
	int zeus_quad_size;

	UINT32 *waveram[2];
	emu_timer *int_timer;
	emu_timer *vblank_timer;
	int m_yScale;
	int yoffs;
	int texel_width;
	float zbase;

	/*************************************
	*  Inlines for block addressing
	*************************************/

	inline void *waveram0_ptr_from_expanded_addr(UINT32 addr)
	{
		UINT32 blocknum = (addr % WAVERAM0_WIDTH) + ((addr >> 16) % WAVERAM0_HEIGHT) * WAVERAM0_WIDTH;
		return WAVERAM_BLOCK0(blocknum);
	}

	inline void *waveram1_ptr_from_expanded_addr(UINT32 addr)
	{
		UINT32 blocknum = (addr % WAVERAM1_WIDTH) + ((addr >> 16) % WAVERAM1_HEIGHT) * WAVERAM1_WIDTH;
		return WAVERAM_BLOCK1(blocknum);
	}

#ifdef UNUSED_FUNCTION
	inline void *waveram0_ptr_from_texture_addr(UINT32 addr, int width)
	{
		UINT32 blocknum = ((addr & ~1) * width) / 8;
		return WAVERAM_BLOCK0(blocknum);
	}
#endif

	/*************************************
	*  Inlines for rendering
	*************************************/

#ifdef UNUSED_FUNCTION
	inline void waveram_plot(int y, int x, UINT32 color)
	{
		if (zeus_cliprect.contains(x, y))
			WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
	}
#endif

	inline void waveram_plot_depth(int y, int x, UINT32 color, UINT16 depth)
	{
		if (zeus_cliprect.contains(x, y))
		{
			WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
			WAVERAM_WRITEDEPTH(zeus_renderbase, y, x, depth);
		}
	}

#ifdef UNUSED_FUNCTION
	inline void waveram_plot_check_depth(int y, int x, UINT32 color, UINT16 depth)
	{
		if (zeus_cliprect.contains(x, y))
		{
			UINT16 *depthptr = WAVERAM_PTRDEPTH(zeus_renderbase, y, x);
			if (depth <= *depthptr)
			{
				WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
				*depthptr = depth;
			}
		}
	}
#endif

#ifdef UNUSED_FUNCTION
	inline void waveram_plot_check_depth_nowrite(int y, int x, UINT32 color, UINT16 depth)
	{
		if (zeus_cliprect.contains(x, y))
		{
			UINT16 *depthptr = WAVERAM_PTRDEPTH(zeus_renderbase, y, x);
			if (depth <= *depthptr)
				WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
		}
	}
#endif
	/*************************************
	*  Inlines for texel accesses
	*************************************/
	inline UINT8 get_texel_8bit(const void *base, int y, int x, int width)
	{
		UINT32 byteoffs = (y / 2) * (width * 2) + ((x / 4) << 3) + ((y & 1) << 2) + (x & 3);
		return WAVERAM_READ8(base, byteoffs);
	}

#ifdef UNUSED_FUNCTION
	inline UINT8 get_texel_4bit(const void *base, int y, int x, int width)
	{
		UINT32 byteoffs = (y / 2) * (width * 2) + ((x / 8) << 3) + ((y & 1) << 2) + ((x / 2) & 3);
		return (WAVERAM_READ8(base, byteoffs) >> (4 * (x & 1))) & 0x0f;
	}
#endif

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

private:
	TIMER_CALLBACK_MEMBER(int_timer_callback);
	void zeus2_register32_w(offs_t offset, UINT32 data, int logit);
	void zeus2_register_update(offs_t offset, UINT32 oldval, int logit);
	int zeus2_fifo_process(const UINT32 *data, int numwords);
	void zeus2_pointer_write(UINT8 which, UINT32 value);
	void zeus2_draw_model(UINT32 baseaddr, UINT16 count, int logit);
	void log_fifo_command(const UINT32 *data, int numwords, const char *suffix);
	
	/*************************************
	*  Member variables
	*************************************/


	UINT8 log_fifo;

	UINT32 zeus_fifo[20];
	UINT8 zeus_fifo_words;

#if TRACK_REG_USAGE
	struct reg_info
	{
		struct reg_info *next;
		UINT32 value;
	};

	reg_info *regdata[0x80];
	int regdata_count[0x80];
	int regread_count[0x80];
	int regwrite_count[0x80];
	reg_info *subregdata[0x100];
	int subregdata_count[0x80];
	int subregwrite_count[0x100];
#endif


};

// device type definition
extern const device_type ZEUS2;

#endif
