// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intelligent Designs NICK emulation

**********************************************************************/

#ifndef MAME_ENTERPRISE_NICK_H
#define MAME_ENTERPRISE_NICK_H

#pragma once

#include "machine/rescap.h"
#include "video/resnet.h"
#include "screen.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

/* there are 64us per line, although in reality
 about 50 are visible. */
#define ENTERPRISE_SCREEN_WIDTH (50*16)

/* there are 312 lines per screen, although in reality
 about 35*8 are visible */
#define ENTERPRISE_SCREEN_HEIGHT    (35*8)


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

/* Nick executes a Display list, in the form of a list of Line Parameter
 Tables, this is the form of the data */
struct LPT_ENTRY
{
	uint8_t SC;       /* scanlines in this modeline (two's complement) */
	uint8_t MB;       /* the MODEBYTE (defines video display mode) */
	uint8_t LM;       /* left margin etc */
	uint8_t RM;       /* right margin etc */
	uint8_t LD1L;     /* (a7..a0) of line data pointer LD1 */
	uint8_t LD1H;     /* (a8..a15) of line data pointer LD1 */
	uint8_t LD2L;     /* (a7..a0) of line data pointer LD2 */
	uint8_t LD2H;     /* (a8..a15) of line data pointer LD2 */
	uint8_t COL[8];   /* COL0..COL7 */
};


// ======================> nick_device

class nick_device :  public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	template <typename T>
	nick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag) :
		nick_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	nick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto virq_wr_callback() { return m_write_virq.bind(); }

	virtual void vram_map(address_map &map) ATTR_COLD;
	virtual void vio_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void nick_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	TIMER_CALLBACK_MEMBER(scanline_tick);

	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	void fixbias_w(uint8_t data);
	void border_w(uint8_t data);
	void lpl_w(uint8_t data);
	void lph_w(uint8_t data);

	address_space_config m_space_config;

private:
	devcb_write_line m_write_virq;

	void initialize_palette();

	void write_pixel(int ci);
	void calc_visible_clocks(int width);
	void write_border(int clocks);
	void do_left_margin();
	void do_right_margin();

	int get_color_index(int pen_index);
	void write_pixels2color(uint8_t pen0, uint8_t pen1, uint8_t data_byte);
	void write_pixels2color_lpixel(uint8_t pen0, uint8_t pen1, uint8_t data_byte);
	void write_pixels(uint8_t data_byte, uint8_t char_idx);
	void write_pixels_lpixel(uint8_t data_byte, uint8_t char_idx);

	void do_pixel(int clocks_visible);
	void do_lpixel(int clocks_visible);
	void do_attr(int clocks_visible);
	void do_ch256(int clocks_visible);
	void do_ch128(int clocks_visible);
	void do_ch64(int clocks_visible);
	void do_display();
	void update_lpt();
	void reload_lpt();
	void do_line();

	/* current scanline within LPT */
	uint8_t m_scanline_count;

	uint8_t m_FIXBIAS;
	uint8_t m_BORDER;
	uint8_t m_LPL;
	uint8_t m_LPH;

	uint16_t m_LD1;
	uint16_t m_LD2;

	LPT_ENTRY   m_LPT;

	uint32_t *m_dest = nullptr;
	int m_dest_pos = 0;
	int m_dest_max_pos = 0;

	uint8_t m_reg[4];

	/* first clock visible on left hand side */
	int m_first_visible_clock = 0;
	/* first clock visible on right hand side */
	int m_last_visible_clock = 0;

	/* given a bit pattern, this will get the pen index */
	uint8_t m_pen_idx_4col[256];
	/* given a bit pattern, this will get the pen index */
	uint8_t m_pen_idx_16col[256];

	int m_virq;

	bitmap_rgb32 m_bitmap;
	rgb_t m_palette[256];

	emu_timer *m_timer_scanline = nullptr;
};


// device type definition
DECLARE_DEVICE_TYPE(NICK, nick_device)

#endif // MAME_ENTERPRISE_NICK_H
