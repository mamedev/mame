// license:BSD-3-Clause
// copyright-holders:Curt Coder, Angelo Salese
/*

References:
- http://www2.odn.ne.jp/~haf09260/Pc80/EnrPc.htm
- http://home1.catvmics.ne.jp/~kanemoto/n80/inside.html
- http://www.geocities.jp/retro_zzz/machines/nec/8001/index.html
- https://oldcrap.org/2024/07/08/nec-pc-8001/

*/

/*

    TODO:

    - uPD3301 attributes;
    - PCG-1000;
    - Intel 8251;
    - cassette;
    - dip-switches;
    - PC-8011 (expansion unit)
    - PC-8021;
    - PC-8031 (mini disk unit, in progress)
	- pc8001mk2: implement 1 layer GVRAM;
    - pc8001mk2sr: verify how much needs to be ported from pc8801.cpp code
      (Has 3 bitplane GVRAM like PC-8801 V1 mode + ALU + few unique modes eventually ditched,
	  mapped at $8000 rather than $c000);
    - waitstates & DMA penalty (some games are suspciously fast);
    - buzzer has pretty ugly aliasing in places;

    Notes:
    - pc8001 v1.01 / v1.02 sports a buggy readout of the expansion ROM at PC=17a1:
      It expects an header read of 0x41-0x42 at offset $6000-6001, but second read at
      PC=0x17aa is just a comparison to $6000 == 0x42, which is impossible at that point
      unless external aid is given. This has been fixed in v1.10;
    - Color Magical (pc8001gp:flop5 option 7) transfers two 8 color screens at
      even/odd frame intervals, effectively boosting the number of available colors to 27.
      This trick is kinda flickery even on real HW, no wonder it looks ugly in MAME,
      can it be improved?

*/

#include "emu.h"
#include "pc88_kbd.h"
#include "pc8001.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "utf8.h"


void pc8001_base_state::crtc_reverse_w(int state)
{
	// rvv acts as a global flip for reverse attribute meaning
	// (does not act on underlying palette)
	// TODO: what happens if RVV changes mid-frame?
	// I suspect monitor resync more likely than rasters.
	m_screen_reverse = state;
}

UPD3301_FETCH_ATTRIBUTE( pc8001_base_state::attr_fetch )
{
	const u8 attr_max_size = 80;
	const bool is_color_mode = gfx_mode == 0x2;
	std::array<u16, attr_max_size> attr_extend_info = m_crtc->default_attr_fetch(attr_row, gfx_mode, y, attr_fifo_size, row_size);

	// further extend the attributes if we are in color mode
	if (is_color_mode)
	{
		// flgworld (pc8001) gameplay sets up:
		// - 0x00 0x00 0x02 0x88 on playfield
		// \- (wanting the default from the first defined color)
		// - 0x00 0x00 0x00 0x48 0x12 0x88 for first row
		// \- (Expecting "FLAG WORLD" wording to be red while the "P"s in green)
		// undermon (pc8001) instruction screen sets up:
		// - 0x00 0x00 0x06 0xb8
		// \- (expecting blue fill up to 0x06)
		// xak2 & cancanb (pc8801) really expects that the color / decoration is implictly
		// latched from previous line (uses semigfx black tiles for masking)
		if (y == 0)
		{
			// TODO: default values for line 0
			m_attr_color = 0xe8;
			m_attr_decoration = 0x00;
		}

		for (int ex = 0; ex < row_size; ex++)
		{
			u16 cur_attr = attr_extend_info[ex];
			if (BIT(cur_attr, 3))
				m_attr_color = cur_attr;
			else
				m_attr_decoration = cur_attr;
			attr_extend_info[ex] = (m_attr_color << 8) | m_attr_decoration;
		}
	}

	return attr_extend_info;
}

UPD3301_DRAW_CHARACTER_MEMBER( pc8001_base_state::draw_text )
{
	// punt if we are in width 40 (discarded on this end)
	if (sx % 2 && !m_width80)
		return;

	const bool is_color_mode = gfx_mode == 0x2;
	u8 tile;
	const u8 tile_width = m_width80 ? 8 : 16;
	const u8 dot_width = (m_width80 ^ 1) + 1;
	const u8 y_double = m_screen_is_24KHz ? 2 : 1;
	const u8 y_height = y_double * 8;

	bool semigfx_tile, reverse, attr_blink, secret;
	bool upperline, lowerline;
	u8 color;

	if (is_color_mode)
	{
		color = (attr & 0xe000) >> 13;
		semigfx_tile = bool(BIT(attr, 12));
		// bit 7 is used by 2001spc and many others, no effect?
	}
	else
	{
		color = 7;
		semigfx_tile = bool(BIT(attr, 7));
	}

	lowerline = bool(BIT(attr, 5));
	// FIXME: ninjakmb (pc8801) sets 0x50 0x50 for all attribute strips
	// bit 6 is undefined in the specs, is it for masking decoration(s)?
	upperline = bool(BIT(attr, 4));
	reverse = bool(BIT(attr, 2));
	attr_blink = bool(BIT(attr, 1));
	secret = bool(BIT(attr, 0));

	if (semigfx_tile)
		tile = cc;
	else
	{
		if (lc > y_height - 1)
			tile = 0;
		else
			tile = m_cgrom->base()[(cc << 3) | (lc >> (y_double-1))];
	}

	// secret blacks out every tile connections,
	// has lower priority over blinking and other attribute decorations
	if (secret)
		tile = 0;

	if (csr)
		tile ^= 0xff;
	else if (attr_blink_on && attr_blink)
		tile = 0;

	// upper/lower line aren't affected by secret and blinking, only reverse
	// TODO: should downshift chars by one
	if (lc == 0 && upperline)
		tile = 0xff;

	if (is_lowestline && lowerline)
		tile = 0xff;

	if (reverse ^ m_screen_reverse)
		tile ^= 0xff;

//  if (m_width80)
	{
		u8 pen_dot;

		for (int xi = 0; xi < tile_width; xi += dot_width)
		{
			int res_x = (sx * 8) + xi;
			if (semigfx_tile)
			{
				u8 mask = (xi & (4 << (dot_width - 1))) ? 0x10 : 0x01;
				mask <<= (lc & (0x3 << y_double)) >> y_double;
				pen_dot = tile & mask;
			}
			else
			{
				pen_dot = tile;
				pen_dot = (pen_dot >> (7 - (xi >> (dot_width - 1)))) & 1;
			}

			if (!pen_dot)
				continue;

			for (int di = 0; di < dot_width; di++)
				bitmap.pix(y, res_x + di) = m_crtc_palette->pen(color);
		}
	}
}

uint32_t pc8001_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	// TODO: superimposing
	// TODO: merging with previous frame for Color Magical (is it driver area?)
	m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}

/*
 * MkII video
 */

void pc8001mk2_state::video_start()
{
	m_screen->register_screen_bitmap(m_text_bitmap);

	save_item(NAME(m_text_layer_mask));
	save_item(NAME(m_bitmap_layer_mask));
}

void pc8001mk2_state::video_reset()
{
	m_text_layer_mask = true;
	m_bitmap_layer_mask = 0x7;
}

void pc8001mk2_state::draw_bitmap_2bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device *palette, std::function<u8(u32 bitmap_offset, int y, int x, int xi)> dot_func)
{
	const uint16_t y_double = 0;
	int32_t y_line_size = y_double + 1;

	for(int y = cliprect.min_y; y <= cliprect.max_y; y += y_line_size)
	{
		for(int x = cliprect.min_x; x <= cliprect.max_x; x += 8)
		{
			u8 x_char = (x >> 3);
			u32 bitmap_offset = (y >> y_double) * 80 + x_char;
			for(int xi = 0; xi < 4; xi ++)
			{
				u8 pen_dot = dot_func(bitmap_offset, y, x_char, 6 - (xi * 2));

				if (pen_dot == 0)
					continue;

				for (int yi = 0; yi < y_line_size; yi ++)
				{
					int res_x = x + xi * 2;
					int res_y = y + yi;
					if (cliprect.contains(res_x, res_y))
						bitmap.pix(res_y, res_x) = palette->pen(pen_dot);
					if (cliprect.contains(res_x + 1, res_y))
						bitmap.pix(res_y, res_x + 1) = palette->pen(pen_dot);
				}
			}
		}
	}
}


uint32_t pc8001mk2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_port31, 3))
	{
		// mkII has no settable palette, use the CRTC one for convenience.
		bitmap.fill(m_crtc_palette->pen(0), cliprect);

		// TODO: backported from mk2sr, see below
		draw_bitmap_2bpp(bitmap, cliprect, m_crtc_palette, [&](u32 bitmap_offset, int y, int x, int xi){
			const u8 color_table[8] = { 0x00, 0x02, 0x04, 0x03, 0x00, 0x02, 0x04, 0x07 };

			u8 res = 0;

			// we have one filled plane only in this implementation
			//for (int plane = 0; plane < 3; plane ++)
			{
				const u8 plane = 0;
				u8 mask = BIT(m_bitmap_layer_mask, plane) * 3;
				res |= (m_gvram[bitmap_offset + plane * 0x4000] >> xi) & mask;
			}

			if (!res)
				return (u8)0;

			return color_table[res | BIT(m_port31, 2) << 2];
			//return m_crtc->is_gfx_color_mode() ? (m_attr_info[y][x] >> 13) & 7 : 7;
		});

	}
	else
		bitmap.fill(0, cliprect);

	if(m_text_layer_mask)
	{
		m_text_bitmap.fill(0, cliprect);
		m_crtc->screen_update(screen, m_text_bitmap, cliprect);
		copybitmap_trans(bitmap, m_text_bitmap, 0, 0, 0, 0, cliprect, 0);
	}

	return 0;
}

/*
 * mkIISR video
 */

void pc8001mk2sr_state::video_start()
{
	pc8001mk2_state::video_start();
	m_screen->register_screen_bitmap(m_graph_bitmap);

	save_item(STRUCT_MEMBER(m_palram, r));
	save_item(STRUCT_MEMBER(m_palram, g));
	save_item(STRUCT_MEMBER(m_palram, b));
}

void pc8001mk2sr_state::video_reset()
{
	pc8001mk2_state::video_reset();
	for (int i = 0; i < 8; i ++)
	{
		m_palram[i].b = i & 1 ? 7 : 0;
		m_palram[i].r = i & 2 ? 7 : 0;
		m_palram[i].g = i & 4 ? 7 : 0;
		m_palette->set_pen_color(i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
	}
}

void pc8001mk2sr_state::draw_bitmap_w80(bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device *palette, std::function<u8(u32 bitmap_offset, int y, int x, int xi)> dot_func)
{
	const uint16_t y_double = 0;
	int32_t y_line_size = y_double + 1;

	for(int y = cliprect.min_y; y <= cliprect.max_y; y += y_line_size)
	{
		for(int x = cliprect.min_x; x <= cliprect.max_x; x += 8)
		{
			u8 x_char = (x >> 3);
			u32 bitmap_offset = (y >> y_double) * 80 + x_char;
			for(int xi = 0; xi < 8; xi++)
			{
				u8 pen_dot = dot_func(bitmap_offset, y, x_char, 7 - xi);

				if (pen_dot == 0)
					continue;

				for (int yi = 0; yi < y_line_size; yi ++)
				{
					int res_x = x + xi;
					int res_y = y + yi;
					if (cliprect.contains(res_x, res_y))
						bitmap.pix(res_y, res_x) = palette->pen(pen_dot);
				}
			}
		}
	}
}

void pc8001mk2sr_state::draw_bitmap_w40(bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device *palette, std::function<u8(int layer_n, u32 bitmap_offset, int y, int x, int xi)> dot_func)
{
	const uint16_t y_double = 0;
	int32_t y_line_size = y_double + 1;
	// n80srbas "demo2/3/4.nsr"
	// normally priority is 0 > 1, bit enabled makes it 0 < 1
	const u8 pr1 = BIT(m_port33, 2);
	const u8 pri0 = 0 ^ pr1;
	const u8 pri1 = 1 ^ pr1;

	for(int y = cliprect.min_y; y <= cliprect.max_y; y += y_line_size)
	{
		for(int x = cliprect.min_x; x <= cliprect.max_x; x += 16)
		{
			u8 x_char = (x >> 4);
			u32 bitmap_offset = (y >> y_double) * 40 + x_char;
			for(int xi = 0; xi < 16; xi++)
			{
				u8 pen_dot[2];
				pen_dot[0] = dot_func(0, bitmap_offset + 0x0000, y, x_char, 7 - (xi >> 1));
				pen_dot[1] = dot_func(1, bitmap_offset + 0x2000, y, x_char, 7 - (xi >> 1));

				for (int yi = 0; yi < y_line_size; yi ++)
				{
					int res_x = x + xi;
					int res_y = y + yi;
					if (cliprect.contains(res_x, res_y))
					{
						if (pen_dot[pri1])
							bitmap.pix(res_y, res_x) = palette->pen(pen_dot[pri1]);
						if (pen_dot[pri0])
							bitmap.pix(res_y, res_x) = palette->pen(pen_dot[pri0]);
					}
				}
			}
		}
	}
}


uint32_t pc8001mk2sr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_graph_bitmap.fill(0, cliprect);

	if (BIT(m_port31, 3))
	{
		// pack2:"Dragon Slayer"
		bitmap.fill(m_palette->pen(0), cliprect);

		// TODO: mkII compatible mono mode
		// 640x200x1bpp, SR GVRAM banks actually OR-ed like pc8801 -> mirror on write access.
		// Also Spectrum-like colors from text attribute (verify if regular mk2 can do it as well).

		if (BIT(m_port31, 5))
		{
			// 2bpp color
			// The goofy one out, OR-es in planar like other 1bpp modes but also draws in packed
			// - pack1 game list
			// - pack1:"Nuts & Milk"
			// - any mk2 SW
			// TODO: enable guessed
			// m_port31 & 0xe0 may be border color really?
			// m_port31 & 0x30 == 0x20 should give mono mode instead
			// n80diskb "mark2" demo plays with this a ton, presumably wants mono color in places

			draw_bitmap_2bpp(m_graph_bitmap, cliprect, m_palette, [&](u32 bitmap_offset, int y, int x, int xi){
				const u8 color_table[8] = { 0x00, 0x02, 0x04, 0x03, 0x00, 0x02, 0x04, 0x07 };

				u8 res = 0;

				for (int plane = 0; plane < 3; plane ++)
				{
					u8 mask = BIT(m_bitmap_layer_mask, plane) * 3;
					res |= (m_gvram[bitmap_offset + plane * 0x4000] >> xi) & mask;
				}

				if (!res)
					return (u8)0;

				return color_table[res | BIT(m_port31, 2) << 2];
				//return m_crtc->is_gfx_color_mode() ? (m_attr_info[y][x] >> 13) & 7 : 7;
			});
		}
		else if (BIT(m_port31, 2))
		{
			// 2 planes width 40

			draw_bitmap_w40(m_graph_bitmap, cliprect, m_palette, [&](int layer_n, u32 bitmap_offset, int y, int x, int xi){
				u8 res = 0;
				if (!BIT(m_bitmap_layer_mask, layer_n))
					return res;

				for (int plane = 0; plane < 3; plane ++)
					res |= ((m_gvram[bitmap_offset + plane * 0x4000] >> xi) & 1) << plane;

				return res;
			});
		}
		else
		{
			// 1 plane width 80

			// NOTE: unlike pc8801 port $53 actually allows disabling the single plane
			if (BIT(m_bitmap_layer_mask, 0))
			{
				draw_bitmap_w80(m_graph_bitmap, cliprect, m_palette, [&](u32 bitmap_offset, int y, int x, int xi){
					u8 res = 0;

					for (int plane = 0; plane < 3; plane ++)
						res |= ((m_gvram[bitmap_offset + plane * 0x4000] >> xi) & 1) << plane;

					return res;
				});
			}
		}
	}
	else
		bitmap.fill(0, cliprect);

	m_text_bitmap.fill(0, cliprect);
	if(m_text_layer_mask)
		m_crtc->screen_update(screen, m_text_bitmap, cliprect);

	// PR2 makes graph to be higher priority than text layer
	// - pack2:"Burger Time" depends on this
	if (BIT(m_port33, 3))
	{
		copybitmap_trans(bitmap, m_text_bitmap, 0, 0, 0, 0, cliprect, 0);
		copybitmap_trans(bitmap, m_graph_bitmap, 0, 0, 0, 0, cliprect, 0);
	}
	else
	{
		copybitmap_trans(bitmap, m_graph_bitmap, 0, 0, 0, 0, cliprect, 0);
		copybitmap_trans(bitmap, m_text_bitmap, 0, 0, 0, 0, cliprect, 0);
	}

	return 0;
}


/* Read/Write Handlers */

/*
 * ---- ---x RTC C0
 * ---- --x- RTC C1
 * ---- -x-- RTC C2
 * ---- x--- RTC DATA IN
 */
void pc8001_base_state::port10_w(uint8_t data)
{
	// RTC
	m_rtc->c0_w(BIT(data, 0));
	m_rtc->c1_w(BIT(data, 1));
	m_rtc->c2_w(BIT(data, 2));
	m_rtc->data_in_w(BIT(data, 3));

	// centronics
	m_cent_data_out->write(data);
}

/*
 * I/O Port $30 (w/o) "System Control Port (1)"
 * N88-BASIC buffer port $e6c0
 *
 * Virtually same between PC-8001 and PC-8801
 *
 * --xx ---- BS2, BS1: USART channel control
 * --00 ----           CMT 600 bps
 * --01 ----           CMT 1200 bps
 * --10 ----           RS-232C async mode
 * --11 ----           RS-232C sync mode
 * ---- x--- MTON: CMT motor control (active high)
 * ---- -x-- CDS: CMT carrier control (1) mark (0) space
 * ---- --x- /COLOR: CRT display mode control (0) color mode (1) monochrome
 * ---- ---x /40: CRT display format control (1) 80 chars per line (0) 40 chars
 *
 */
void pc8001_base_state::port30_w(uint8_t data)
{
	m_width80 = BIT(data, 0);
	m_color = BIT(data, 1);

	m_cassette->change_state(BIT(data, 3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

/*
 * xxx- ---- background color
 * ---x ---- resolution (0=640x200, 1=320x200)
 * ---- x--- graphics enable
 * ---- -x-- color mode (0=attribute, 1=B&W)
 * ---- --x- memory mode (0=ROM, 1=RAM)
 * ---- ---x expansion ROM (0=expansion, 1=N80)
 */
void pc8001mk2_state::port31_w(uint8_t data)
{
	m_port31 = data;
	flush_low_bank();
}

void pc8001mk2_state::flush_low_bank()
{
	// enable 64K work RAM
	if (BIT(m_port31, 1))
		m_exp_view.disable();
	else
		m_exp_view.select(m_port31 & 1);
}

void pc8001_base_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

void pc8001_base_state::write_centronics_ack(int state)
{
	m_centronics_ack = state;
}

uint8_t pc8001_state::port40_r()
{
	/*

	    bit     description

	    0       BUSY
	    1       ACK
	    2       CMT CDIN
	    3       EXP /EXTON
	    4       RTC DATA OUT
	    5       VRTC
	    6
	    7

	*/

	uint8_t data = 0x00;

	data |= m_centronics_busy;
	data |= m_centronics_ack << 1;
	data |= m_rtc->data_out_r() << 4;
	data |= m_crtc->vrtc_r() << 5;
	// TODO: enable line from pc80s31k (bit 3, active_low)

	return data;
}

/*
 * --x- ---- SPEAKER
 * ---- x--- CRT /CLDS CLK
 * ---- -x-- RTC CLK
 * ---- --x- RTC STB
 * ---- ---x Centronics STROBE
 */
void pc8001_state::port40_w(uint8_t data)
{
	m_centronics->write_strobe(BIT(data, 0));

	m_rtc->stb_w(BIT(data, 1));
	m_rtc->clk_w(BIT(data, 2));

	m_beep->set_state(BIT(data, 5));
}

/* i8214 PICU section */

void pc8001_base_state::irq_level_w(uint8_t data)
{
	m_picu->b_sgs_w(~data);
}

/*
 * ---- -x-- /RXMF RXRDY irq mask
 * ---- --x- /VRMF VRTC irq mask
 * ---- ---x /RTMF Real-time clock irq mask
 *
 */
void pc8001_base_state::irq_mask_w(uint8_t data)
{
	m_irq_state.enable &= ~7;
	// mapping reversed to the correlated irq levels
	m_irq_state.enable |= bitswap<3>(data & 7, 0, 1, 2);

	check_irq(RXRDY_IRQ_LEVEL);
	check_irq(VRTC_IRQ_LEVEL);
	check_irq(CLOCK_IRQ_LEVEL);
}

void pc8001_base_state::rxrdy_irq_w(int state)
{
	if (state)
		assert_irq(RXRDY_IRQ_LEVEL);
}

/*
 * 0 RXRDY
 * 1 VRTC
 * 2 CLOCK
 * 3 INT3 (GSX-8800)
 * 4 INT4 (any OPN, external boards included with different irq mask at $aa)
 * 5 INT5
 * 6 FDCINT1
 * 7 FDCINT2
 *
 */
IRQ_CALLBACK_MEMBER(pc8001_base_state::int_ack_cb)
{
	// TODO: schematics sports a μPB8212 too, with DI2-DI4 connected to 8214 A0-A2
	// Seems just an intermediate bridge for translating raw levels to vectors
	// with no access from outside world?
	u8 level = m_picu->a_r();
	m_picu->r_w(level, 1);

	return (7 - level) * 2;
}

void pc8001_base_state::int4_irq_w(int state)
{
	bool irq_state = m_sound_irq_enable & state;

	// remember current setting so that an enable reg variation will pick up
	// particularly needed by PC-88 Telenet games (xzr2, valis2)
	// TODO: understand how exactly the external irq source works out (Sound Board II)
	// has a separate irq mask for secondary OPNA but still sends INT4s,
	// we separate the logic from the others since this exact function needs templatized array for enable and pending anyway
	// (and won't otherwise work for xzr2 anyway).
	m_picu->r_w(7 ^ INT4_IRQ_LEVEL, !irq_state);
	m_sound_irq_pending = state;
}

// FIXME: convert to pure write-line-style member
// Works with 0 -> 1 F/F transitions
TIMER_DEVICE_CALLBACK_MEMBER(pc8001_base_state::clock_irq_w)
{
	// NOTE: pc8801:castlex uses this rather than dedicated OPN INT4 for BGM tempo
	assert_irq(CLOCK_IRQ_LEVEL);
}

void pc8001_base_state::check_irq(u8 level)
{
	u8 mask = 1 << level;

	// pc8801:megamit and pc8801:babylon are particularly fussy if the VRTC irq isn't disabled when requested
	// - megamit jumps to PC=0
	// - babylon has just a ret coded in the VRTC irq, so accepting that will wreck the program flow and hang at title screen with no sound (because it expects INT4s)
	if (!(m_irq_state.enable & mask))
		m_picu->r_w(7 ^ level, 1);
	else if (m_irq_state.enable & m_irq_state.pending & mask)
		assert_irq(level);
}

void pc8001_base_state::assert_irq(u8 level)
{
	u8 mask = 1 << level;

	if (mask & m_irq_state.enable)
	{
		m_irq_state.pending &= ~mask;
		m_picu->r_w(7 ^ level, 0);
	}
	else
		m_irq_state.pending |= mask;
}

void pc8001_base_state::vrtc_irq_w(int state)
{
//  bool irq_state = m_vrtc_irq_enable & state;
	if (state)
	{
		assert_irq(VRTC_IRQ_LEVEL);
	}
}


/* Memory Maps */

void pc8001_state::pc8001_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(pc8001_state::ram_r<0x8000>), FUNC(pc8001_state::ram_w<0x8000>));
	map(0x0000, 0x7fff).view(m_exp_view);
	m_exp_view[0](0x0000, 0x5fff).rom().region("maincpu", 0x0000);
	// TODO: hookup expansion ROM
	m_exp_view[0](0x6000, 0x7fff).lr8(NAME([] (offs_t offset) { return 0xff; }));
	m_exp_view[1](0x0000, 0x5fff).rom().region("maincpu", 0x0000);
	m_exp_view[1](0x6000, 0x7fff).rom().region("maincpu", 0x6000);
	map(0x8000, 0xbfff).rw(FUNC(pc8001_state::ram_r<0x4000>), FUNC(pc8001_state::ram_w<0x4000>));
	map(0xc000, 0xffff).rw(FUNC(pc8001_state::ram_r<0x0000>), FUNC(pc8001_state::ram_w<0x0000>));
}

void pc8001_state::pc8001_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x0f).r("kbd", FUNC(pc8001_kbd_device::read_direct));
	map(0x10, 0x10).mirror(0x0f).w(FUNC(pc8001_state::port10_w));
	map(0x20, 0x21).mirror(0x0e).rw(I8251_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x30, 0x30).mirror(0x0f).w(FUNC(pc8001_state::port30_w));
	map(0x40, 0x40).mirror(0x0f).rw(FUNC(pc8001_state::port40_r), FUNC(pc8001_state::port40_w));
	map(0x50, 0x51).rw(m_crtc, FUNC(upd3301_device::read), FUNC(upd3301_device::write));
	map(0x60, 0x68).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
//  map(0x70, 0x7f) unused
//  map(0x80, 0x80).mirror(0x0f).w(FUNC(pc8001_state::pc8011_ext0_w));
//  map(0x90, 0x90).mirror(0x0f).w(FUNC(pc8001_state::pc8011_ext1_w));
//  map(0xa0, 0xa0).mirror(0x0f).w(FUNC(pc8001_state::pc8011_ext2_w));
//  map(0xb0, 0xb0).r(FUNC(pc8001_state::pc8011_gpio8_r));
//  map(0xb1, 0xb1).w(FUNC(pc8001_state::pc8011_gpio8_w));
//  map(0xb2, 0xb2).r(FUNC(pc8001_state::pc8011_gpio4_r));
//  map(0xb3, 0xb3).w(FUNC(pc8001_state::pc8011_gpio4_w));
//  map(0xc0, 0xc0).rw(PC8011_CH1_I8251_TAG, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
//  map(0xc1, 0xc1).rw(PC8011_CH1_I8251_TAG, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
//  map(0xc2, 0xc2).rw(PC8011_CH2_I8251_TAG, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
//  map(0xc3, 0xc3).rw(PC8011_CH2_I8251_TAG, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
//  map(0xc8, 0xc8) RS-232 output enable?
//  map(0xca, 0xca) RS-232 output disable?
//  map(0xd0, 0xd3).rw(PC8011_IEEE488_I8255A_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
//  map(0xd8, 0xd8).r(FUNC(pc8001_state::pc8011_ieee488_control_signal_input_r));
//  map(0xda, 0xda).r(FUNC(pc8001_state::pc8011_ieee488_bus_address_mode_r));
//  map(0xdc, 0xdc).w(FUNC(pc8001_state::pc8011_ieee488_nrfd_w));
//  map(0xde, 0xde).w(FUNC(pc8001_state::pc8011_ieee488_bus_mode_control_w));
//  map(0xe0, 0xe3).w(FUNC(pc8001_state::expansion_storage_mode_w));
	map(0xe4, 0xe4).w(FUNC(pc8001_state::irq_level_w));
	map(0xe6, 0xe6).w(FUNC(pc8001_state::irq_mask_w));
//  map(0xe7, 0xe7).w(FUNC(pc8001_state::pc8012_memory_mode_w));
//  map(0xe8, 0xfb) unused
	map(0xfc, 0xff).m(m_pc80s31, FUNC(pc80s31_device::host_map));
}

/*
 *
 * mkII (GVRAM)
 *
 */

void pc8001mk2_state::flush_gvram_access()
{
	m_gvram_bank->set_bank(m_vram_sel);
}

uint8_t pc8001mk2_state::gvram_r(offs_t offset)
{
	return m_gvram[offset];
}

void pc8001mk2_state::gvram_w(offs_t offset, uint8_t data)
{
	m_gvram[offset] = data;
}

void pc8001mk2_state::gvram_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(pc8001mk2_state::gvram_r), FUNC(pc8001mk2_state::gvram_w));
	map(0xc000, 0xffff).rw(FUNC(pc8001mk2_state::ram_r<0x4000>), FUNC(pc8001mk2_state::ram_w<0x4000>));
}

void pc8001mk2_state::pc8001mk2_map(address_map &map)
{
	pc8001_map(map);
	map(0x8000, 0xbfff).m(m_gvram_bank, FUNC(address_map_bank_device::amap8));
}

void pc8001mk2_state::pc8001mk2_io(address_map &map)
{
	pc8001_io(map);
	map(0x30, 0x30).portr("DSW1").w(FUNC(pc8001mk2_state::port30_w));
	map(0x31, 0x31).portr("DSW2").w(FUNC(pc8001mk2_state::port31_w));
	map(0x53, 0x53).lw8(
		NAME([this] (u8 data) {
			m_text_layer_mask = !!(BIT(~data, 0));
			// NOTE: more bits vs. pc8801
			m_bitmap_layer_mask = ((data & 0x7e) >> 1) ^ 0x3f;
		})
	);
//  map(0x5c, 0x5c).w(FUNC(pc8001mk2_state::gvram_on_w));
//  map(0x5f, 0x5f).w(FUNC(pc8001mk2_state::gvram_off_w));
	map(0x5c, 0x5c).lw8(NAME([this] (offs_t offset, u8 data) { (void)data; m_vram_sel = 0; flush_gvram_access(); }));
	map(0x5f, 0x5f).lw8(NAME([this] (offs_t offset, u8 data) { (void)data; m_vram_sel = 3; flush_gvram_access(); }));
//  map(0xe8, 0xe8) kanji_address_lo_w, kanji_data_lo_r
//  map(0xe9, 0xe9) kanji_address_hi_w, kanji_data_hi_r
//  map(0xea, 0xea) kanji_readout_start_w
//  map(0xeb, 0xeb) kanji_readout_end_w
//  map(0xf3, 0xf3) DMA type disk unit interface selection port
//  map(0xf4, 0xf4) DMA type 8 inch control
//  map(0xf5, 0xf5) DMA type 8 inch margin control
//  map(0xf6, 0xf6) DMA type 8 inch FDC status
//  map(0xf7, 0xf7) DMA type 8 inch FDC data register
//  map(0xf8, 0xf8) DMA type 5 inch control
//  map(0xf9, 0xf9) DMA type 5 inch margin control
//  map(0xfa, 0xfa) DMA type 5 inch FDC status
//  map(0xfb, 0xfb) DMA type 5 inch FDC data register
}

/*
 *
 * PC-8001mkIISR (ALU, bumped GVRAM)
 *
 */

void pc8001mk2sr_state::gvram_map(address_map &map)
{
	map(0x0000, 0xbfff).rw(FUNC(pc8001mk2sr_state::gvram_r), FUNC(pc8001mk2sr_state::gvram_w));
	map(0xc000, 0xffff).rw(FUNC(pc8001mk2sr_state::ram_r<0x4000>), FUNC(pc8001mk2sr_state::ram_w<0x4000>));
}

void pc8001mk2sr_state::flush_low_bank()
{
	// work RAM enable has priority over any BIOS
	if (BIT(m_port31, 1))
	{
		m_exp_view.disable();
		return;
	}

	// $e2 individual work RAM select works similarly to PC-8801,
	// except it has just one register and can only bank to 64K
	if (m_extram_mode & 0x11)
	{
		m_extram_view.select(m_extram_mode);
	}
	else
		m_extram_view.disable();

	if (BIT(m_port33, 7))
	{
		m_exp_view.select(2 | (m_n80sr_bank & 1));
	}
	else
	{
		m_exp_view.select(0 | (m_port31 & 1));
	}
}

void pc8001mk2sr_state::flush_gvram_access()
{
	// NOTE: different than the equivalent pc88 access ($32 vs. $33)
	//if (BIT(m_misc_ctrl, 6))
	if (BIT(m_port33, 6))
	{
		if (m_alu_gam)
		{
			m_alu_view.select(0);
		}
		else
		{
			m_alu_view.disable();
		}

		// NOTE: ALU enabled wins over GVRAM, to the point of disabling its latch when setting changes
		m_vram_sel = 3;
	}
	else
		m_alu_view.disable();

	pc8001mk2_state::flush_gvram_access();
}

u8 pc8001mk2sr_state::port33_r()
{
	return m_port33;
}

/*
 * x--- ---- N80SR enable SR specific BIOS
 * -x-- ---- GVAM ALU enable
 * ---x ---- HIRA hiragana enable
 * ---- x--- PR2 graphic priority over text
 * ---- -x-- PR1 swap bank 0 and bank 1 in 320x200 graphic mode
 * ---- --x- SINTM sound irq mask
 */
void pc8001mk2sr_state::port33_w(u8 data)
{
	m_port33 = data;

	if (data & 0x10)
		popmessage("pc8001.cpp: port33_w HIRA %02x", data);
	flush_low_bank();
	flush_gvram_access();

	m_sound_irq_enable = !!BIT(~data, 1);

	if (m_sound_irq_enable)
		int4_irq_w(m_sound_irq_pending);

}

void pc8001mk2sr_state::alu_ctrl2_w(u8 data)
{
	m_alu->ctrl2_w(data);
	m_alu_gam = BIT(data, 7);
	flush_gvram_access();
}

u8 pc8001mk2sr_state::port71_r()
{
	return m_n80sr_bank;
}

void pc8001mk2sr_state::port71_w(u8 data)
{
	m_n80sr_bank = data;
	flush_low_bank();
}

void pc8001mk2sr_state::pc8001mk2sr_map(address_map &map)
{
	pc8001mk2_map(map);
	m_exp_view[2](0x0000, 0x5fff).rom().region(N80SR_ROM_TAG, 0x0000);
	m_exp_view[2](0x6000, 0x7fff).rom().region(N80SR_ROM_TAG, 0x8000);
	m_exp_view[3](0x0000, 0x5fff).rom().region(N80SR_ROM_TAG, 0x0000);
	m_exp_view[3](0x6000, 0x7fff).rom().region(N80SR_ROM_TAG, 0x6000);
	map(0x0000, 0x7fff).view(m_extram_view);
	m_extram_view[0x01](0x0000, 0x7fff).r(FUNC(pc8001mk2sr_state::ram_r<0x8000>));
	m_extram_view[0x10](0x0000, 0x7fff).w(FUNC(pc8001mk2sr_state::ram_w<0x8000>));
	m_extram_view[0x11](0x0000, 0x7fff).rw(FUNC(pc8001mk2sr_state::ram_r<0x8000>), FUNC(pc8001mk2sr_state::ram_w<0x8000>));

	map(0x8000, 0xbfff).view(m_alu_view);
	m_alu_view[0](0x8000, 0xbfff).rw(m_alu, FUNC(pc88_alu_device::alu_r), FUNC(pc88_alu_device::alu_w));
}

void pc8001mk2sr_state::pc8001mk2sr_io(address_map &map)
{
	pc8001mk2_io(map);
	// latch for mkIISR (pack2:"Burger Time" cares)
	map(0x32, 0x32).lrw8(
		NAME([this] () { return m_port32; }),
		NAME([this] (u8 data) { m_port32 = data; })
	);
	map(0x33, 0x33).rw(FUNC(pc8001mk2sr_state::port33_r), FUNC(pc8001mk2sr_state::port33_w));
	map(0x34, 0x34).w(m_alu, FUNC(pc88_alu_device::ctrl1_w));
	map(0x35, 0x35).w(FUNC(pc8001mk2sr_state::alu_ctrl2_w));
	map(0x41, 0x4f).unmaprw();
	map(0x44, 0x45).rw(m_opn, FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	// 0x53 in pc8001mk2 (layer disable)
	map(0x54, 0x5b).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_palram[offset].b = data & 1 ? 7 : 0;
			m_palram[offset].r = data & 2 ? 7 : 0;
			m_palram[offset].g = data & 4 ? 7 : 0;

			m_palette->set_pen_color(offset, pal3bit(m_palram[offset].r), pal3bit(m_palram[offset].g), pal3bit(m_palram[offset].b));
		})
	);
	map(0x5c, 0x5c).lr8(NAME([this] () {
		return 0xf8 | ((m_vram_sel == 3) ? 0 : (1 << m_vram_sel));
	}));
	map(0x5c, 0x5f).lw8(NAME([this] (offs_t offset, u8 data) { (void)data; m_vram_sel = offset & 3; flush_gvram_access(); }));

	map(0x70, 0x70).ram(); // latch for mkIISR detection
	map(0x71, 0x71).rw(FUNC(pc8001mk2sr_state::port71_r), FUNC(pc8001mk2sr_state::port71_w));

	map(0xe2, 0xe2).lrw8(
		NAME([this] () {
			return (m_extram_mode ^ 0x11) | 0xee;
		}),
		NAME([this] (u8 data) {
			m_extram_mode = data & 0x11;
			flush_low_bank();
		})
	);
}

/* Input Ports */

static INPUT_PORTS_START( pc8001 )
//  PORT_START("DSW1")
INPUT_PORTS_END

static INPUT_PORTS_START( pc8001mk2 )
	PORT_INCLUDE( pc8001 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Boot Mode" )
	PORT_DIPSETTING(    0x00, "N-BASIC" )
	PORT_DIPSETTING(    0x01, "N80-BASIC" )
	PORT_DIPNAME( 0x02, 0x02, "DSW1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pc8001mk2sr )
	PORT_INCLUDE( pc8001mk2 )

	PORT_MODIFY("DSW1")
	// This is really a tri-state dip on front panel
	// BIOS just expects bit 1 to be off for SR mode
	PORT_DIPNAME( 0x03, 0x01, "Boot Mode" )
	PORT_DIPSETTING(    0x00, "N80SR-BASIC (duplicate)")
	PORT_DIPSETTING(    0x01, "N80SR-BASIC" )
	PORT_DIPSETTING(    0x02, "N-BASIC" )
	PORT_DIPSETTING(    0x03, "N80-BASIC" )
	PORT_DIPNAME( 0x04, 0x04, "DSW1" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* 8257 Interface */

void pc8001_base_state::hrq_w(int state)
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	m_dma->hlda_w(state);
}

uint8_t pc8001_base_state::dma_mem_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	return program.read_byte(offset);
}

/* Machine Initialization */

void pc8001_base_state::machine_start()
{
	m_screen_reverse = false;

	/* initialize RTC */
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	save_item(NAME(m_width80));
	save_item(NAME(m_color));
	save_item(NAME(m_screen_reverse));
	save_item(NAME(m_screen_is_24KHz));

	// PICU init
	save_item(STRUCT_MEMBER(m_irq_state, enable));
	save_item(STRUCT_MEMBER(m_irq_state, pending));
	save_item(NAME(m_sound_irq_enable));
	save_item(NAME(m_sound_irq_pending));
}

void pc8001_state::machine_start()
{
	pc8001_base_state::machine_start();

	/* initialize DMA */
	m_dma->ready_w(1);

	// PC8001 is 15KHz only
	set_screen_frequency(false);
}

void pc8001_base_state::picu_reset()
{
	m_picu->etlg_w(1);
	m_picu->inte_w(1);
	m_irq_state.pending = 0;
	m_irq_state.enable = 0;
	m_sound_irq_enable = false;
	m_sound_irq_pending = false;
}

void pc8001_state::machine_reset()
{
	m_exp_view.select(1);
	picu_reset();
}

void pc8001mk2_state::machine_start()
{
	pc8001_state::machine_start();

	// NOTE: regular mk2 has 0x4000 only of GVRAM
	m_gvram = make_unique_clear<uint8_t[]>(0xc000);

	save_pointer(NAME(m_gvram), 0xc000);
	save_item(NAME(m_vram_sel));
}

void pc8001mk2_state::machine_reset()
{
	pc8001_state::machine_reset();

	m_vram_sel = 3;
	flush_gvram_access();
}


void pc8001mk2sr_state::machine_start()
{
	pc8001mk2_state::machine_start();

	save_item(NAME(m_n80sr_bank));
	save_item(NAME(m_port31));
	save_item(NAME(m_port32));
	save_item(NAME(m_port33));
	save_item(NAME(m_alu_gam));
	save_item(NAME(m_extram_mode));
}

void pc8001mk2sr_state::machine_reset()
{
	pc8001mk2_state::machine_reset();

	m_port31 = 0;
	m_port32 = 0x98;
	// SR BIOS doesn't check DSW1 for non-SR modes
	m_port33 = BIT(m_dsw[0]->read(), 1) ? 0 : 0x80;
	m_n80sr_bank = 1;
	m_extram_mode = 0;
	flush_low_bank();

	m_alu_gam = 0;
	flush_gvram_access();
}

/* Snapquik */

SNAPSHOT_LOAD_MEMBER(pc8001_state::snapshot_cb)
{
	if (m_ram->size() < 0x10000)
		return std::make_pair(image_error::UNSUPPORTED, std::string("Configured RAM size must be 64K"));

	if (image.length() < 0x7f40 || image.length() > 0x8000)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	uint8_t *ram = m_ram->pointer();

	std::vector<u8> snapshot(image.length());
	image.fread(&snapshot[0], image.length());

	std::copy_n(&snapshot[0x0000], 0x4000, &ram[0x4000]);
	const s32 load_size = image.length() - 0x4000;
	if (load_size > 0)
	{
		std::copy_n(&snapshot[0x4000], load_size, &ram[0x0000]);
	}

	m_maincpu->set_state_int(Z80_SP, snapshot[0x7f3e] | (snapshot[0x7f3f] << 8));
	m_maincpu->set_pc(0xff3d);

	//m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	return std::make_pair(std::error_condition(), std::string());
}


/* Machine Drivers */

void pc8001_state::pc8001(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = XTAL(4'000'000);
	constexpr XTAL VIDEO_CLOCK = XTAL(14'318'181);

	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8001_state::pc8001_map);
	m_maincpu->set_addrmap(AS_IO, &pc8001_state::pc8001_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(pc8001_state::int_ack_cb));

	I8214(config, m_picu, MASTER_CLOCK);
	m_picu->int_wr_callback().set_inputline(m_maincpu, 0);
	m_picu->set_int_dis_hack(true);

	PC80S31(config, m_pc80s31, MASTER_CLOCK);
	// TODO: get rid of this
	config.set_perfect_quantum("pc80s31:fdc_cpu");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(VIDEO_CLOCK, 896, 0, 640, 260, 0, 200);
	m_screen->set_screen_update(FUNC(pc8001_state::screen_update));
//  m_screen->set_palette(m_crtc_palette);

	PALETTE(config, m_crtc_palette, palette_device::BRG_3BIT);

	UPD3301(config, m_crtc, VIDEO_CLOCK);
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(pc8001_state::draw_text));
	m_crtc->set_attribute_fetch_callback(FUNC(pc8001_state::attr_fetch));
	m_crtc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));
	m_crtc->rvv_wr_callback().set(FUNC(pc8001_state::crtc_reverse_w));
	m_crtc->vrtc_wr_callback().set(FUNC(pc8001_state::vrtc_irq_w));
	m_crtc->set_screen(m_screen);

	I8257(config, m_dma, MASTER_CLOCK);
	m_dma->out_hrq_cb().set(FUNC(pc8001_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(pc8001_state::dma_mem_r));
	m_dma->out_iow_cb<2>().set(m_crtc, FUNC(upd3301_device::dack_w));

	/* devices */
	TIMER(config, "rtc_timer").configure_periodic(FUNC(pc8001_state::clock_irq_w), attotime::from_hz(600));

	I8251(config, I8251_TAG);

	UPD1990A(config, m_rtc);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(pc8001_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(pc8001_state::write_centronics_busy));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	RAM(config, RAM_TAG).set_default_size("16K").set_extra_options("32K,64K");

	PC8001_KBD(config, "kbd");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	// TODO: unknown clock, is it really a beeper?
	BEEP(config, m_beep, 2400).add_route(ALL_OUTPUTS, "mono", 0.25);

	snapshot_image_device &snapshot(SNAPSHOT(config, "snapshot", "bin,n80", attotime::from_seconds(1)));
	snapshot.set_load_callback(FUNC(pc8001_state::snapshot_cb));

	SOFTWARE_LIST(config, "disk_n_list").set_original("pc8001_flop");
}

void pc8001mk2_state::pc8001mk2(machine_config &config)
{
	pc8001(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8001mk2_state::pc8001mk2_map);
	m_maincpu->set_addrmap(AS_IO, &pc8001mk2_state::pc8001mk2_io);

	ADDRESS_MAP_BANK(config, m_gvram_bank).set_map(&pc8001mk2_state::gvram_map).set_options(ENDIANNESS_LITTLE, 8, 16, 0x4000);

	RAM(config.replace(), RAM_TAG).set_default_size("64K");

	SOFTWARE_LIST(config, "disk_n80_list").set_original("pc8001mk2_flop");
}

void pc8001mk2sr_state::pc8001mk2sr(machine_config &config)
{
	pc8001mk2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8001mk2sr_state::pc8001mk2sr_map);
	m_maincpu->set_addrmap(AS_IO, &pc8001mk2sr_state::pc8001mk2sr_io);

	PC8801_KBD(config.replace(), "kbd");

//	m_gvram_bank->set_map(&pc8001mk2sr_state::gvram_map);
	PALETTE(config, m_palette, palette_device::BLACK, 0x8);

	PC88_ALU(config, m_alu, 0);
	m_alu->gvram_read_cb().set(FUNC(pc8001mk2sr_state::gvram_r));
	m_alu->gvram_write_cb().set(FUNC(pc8001mk2sr_state::gvram_w));

	YM2203(config, m_opn, XTAL(4'000'000));
	m_opn->irq_handler().set(FUNC(pc8001mk2sr_state::int4_irq_w));
	// TODO: pull high for now (pack2:"Dig Dug")
	// OPN/OPNA needs to be moved in a common internal expansion slot
	m_opn->port_a_read_callback().set([] () { return 0xff; });
	m_opn->port_b_read_callback().set([] () { return 0xff; });
//	m_opn->port_b_write_callback().set(FUNC(pc8801mk2sr_state::opn_portb_w));
	m_opn->add_route(ALL_OUTPUTS, "mono", 0.5);

	SOFTWARE_LIST(config, "disk_n80sr_list").set_original("pc8001mk2sr_flop");
}

/* ROMs */

ROM_START( pc8001 )
	ROM_REGION( 0x8000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("v110")
	// PCB pictures shows divided by 3 ROMs (and 4th socket unpopulated)
	// D2364C ROMs from a pc8001b PCB:
	// - p12019-106.ic10 072NBASIC
	// - p11219-105.ic11 073NBASIC
	// - p12029-106.ic12 171NBASIC
	ROM_SYSTEM_BIOS( 0, "v101", "N-BASIC v1.01" )
	ROMX_LOAD( "n80v101.rom", 0x00000, 0x6000, BAD_DUMP CRC(a2cc9f22) SHA1(6d2d838de7fea20ddf6601660d0525d5b17bf8a3), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v102", "N-BASIC v1.02" )
	ROMX_LOAD( "n80v102.rom", 0x00000, 0x6000, BAD_DUMP CRC(ed01ca3f) SHA1(b34a98941499d5baf79e7c0e5578b81dbede4a58), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v110", "N-BASIC v1.10" )
	ROMX_LOAD( "n80v110.rom", 0x00000, 0x6000, BAD_DUMP CRC(1e02d93f) SHA1(4603cdb7a3833e7feb257b29d8052c872369e713), ROM_BIOS(2) )
	// empty socket, cfr. notes in header for usage instructions
	ROM_LOAD( "exprom.ic13", 0x6000, 0x2000, NO_DUMP )

	ROM_REGION( 0x800, CGROM_TAG, 0)
	ROM_LOAD( "font.rom", 0x000, 0x800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )
ROM_END

ROM_START( pc8001mk2 )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	// N-BASIC v1.3
	// N80-BASIC v1.0
	ROM_LOAD( "n80_2.rom", 0x0000, 0x8000, CRC(03cce7b6) SHA1(c12d34e42021110930fed45a8af98db52136f1fb) )

	ROM_REGION( 0x800, CGROM_TAG, 0)
	ROM_LOAD( "font.rom", 0x0000, 0x0800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )

	ROM_REGION( 0x20000, "kanji", 0)
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
ROM_END

ROM_START( pc8001mk2sr )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	// N-BASIC v1.6
	// N80-BASIC v1.2
	ROM_LOAD( "n80_2sr.rom", 0x0000, 0x8000, CRC(dcb71282) SHA1(e8db5dc5eae11da14e48656d324874e59f2e3844) )

	ROM_REGION (0x10000, N80SR_ROM_TAG, ROMREGION_ERASEFF )
	// N80SR-BASIC v1.0
	ROM_LOAD( "n80_3.rom",    0x0000, 0xa000, BAD_DUMP CRC(d99ef247) SHA1(9bfa5009d703cd31caa734d932d2a847d74cbfa6) )
	// TODO: empty socket at ic77

	ROM_REGION( 0x2000, CGROM_TAG, 0)
	ROM_LOAD( "font80sr.rom", 0x000000, 0x001000, CRC(784c0b17) SHA1(565dc8e5e46b1633cb434d12b4d8b3a662546b33) )
	ROM_LOAD( "fonthira.rom", 0x001000, 0x000800, CRC(fe7059d5) SHA1(10c5f85adcce540cbd0a11352e2c38a84c989a26) )
	ROM_LOAD( "fontkata.rom", 0x001800, 0x000800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )

	ROM_REGION( 0x20000, "kanji", 0)
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
ROM_END

/* System Drivers */

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT   CLASS            INIT        COMPANY  FULLNAME       FLAGS
// 1978?, pc8001g, Wirewrapped prototype version
COMP( 1979, pc8001,      0,      0,      pc8001,      pc8001,      pc8001_state,      empty_init, "NEC",   "PC-8001",     MACHINE_NOT_WORKING )
// 1981 pc8001a, US version of PC-8001 with Greek alphabet instead of Kana
COMP( 1983, pc8001mk2,   pc8001, 0,      pc8001mk2,   pc8001mk2,   pc8001mk2_state,   empty_init, "NEC",   "PC-8001mkII", MACHINE_NOT_WORKING )
COMP( 1985, pc8001mk2sr, pc8001, 0,      pc8001mk2sr, pc8001mk2sr, pc8001mk2sr_state, empty_init, "NEC",   "PC-8001mkIISR", MACHINE_NOT_WORKING )
