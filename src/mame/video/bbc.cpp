// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/******************************************************************************
    BBC Model B

    MESS Driver By:

    Gordon Jefferyes
    mess_bbc@romvault.com

    This is the first go around at converting the BBC code over to using
    mames built in mc6845, there are a number of features now incorrect
    or missing in this build:

    BBC split modes no longer work (Like is used in Elite.)

******************************************************************************/

#include "emu.h"
#include "includes/bbc.h"
#include "video/saa5050.h"
#include "video/mc6845.h"
#include "screen.h"

/************************************************************************
 * C0 and C1 along with MA12 output from the 6845 drive 4 NAND gates in ICs 27,36 and 40
 * the outputs from these NAND gates (B1 to B4) along with MA8 to MA11 from the 6845 (A1 to B4) are added together
 * in IC39 74LS283 4 bit adder to form (S1 to S4) the logic is used to loop the screen memory for hardware scrolling.
 * when MA13 from the 6845 is low the latches IC8 and IC9 are enabled
 * they control the memory addressing for the Hi-Res modes.
 * when MA13 from the 6845 is high the latches IC10 and IC11 are enabled
 * they control the memory addressing for the Teletext mode.
 * IC 8 or IC10 drives the row select in the memory (the lower 7 bits in the memory address) and
 * IC 9 or IC11 drives the column select in the memory (the next 7 bits in the memory address) this
 * gives control of the bottom 14 bits of the memory, in a 32K model B 15 bits are needed to access
 * all the RAM, so S4 for the adder drives the CAS0 and CAS1 to access the top bit, in a 16K model A
 * the output of S4 is linked out to a 0v supply by link S25 to just access the 16K memory area.
 ************************************************************************/

uint16_t bbc_state::calculate_video_address(uint16_t ma, uint8_t ra)
{
	/* output from IC32 74LS259 bits 4 and 5 */
	int c0 = m_latch->q4_r();
	int c1 = m_latch->q5_r();

	/* the 4 bit input port b on IC39 are produced by 4 NAND gates. These NAND gates take their inputs
	   from c0 and c1 (from IC32) and ma12 (from the 6845) */

	/* get bit m12 from the 6845 */
	int ma12 = BIT(ma, 12);

	/* 4 bit input B on IC39 74LS283 (4 bit adder) */
	/* 3 input NAND part of IC 36 */
	int b1 = (~(c1 & c0 & ma12)) & 1;
	/* 2 input NAND part of IC40 (b3 is calculated before b2 and b4 because b3 feed back into b2 and b4) */
	int b3 = (~(c0 & ma12)) & 1;
	/* 3 input NAND part of IC 36 */
	int b2 = (~(c1 & b3 & ma12)) & 1;
	/* 2 input NAND part of IC 27 */
	int b4 = (~(b3 & ma12)) & 1;

	/* inputs port b to IC39 are taken from the NAND gates b1 to b4 */
	int b = (b1<<0) | (b2<<1) | (b3<<2) | (b4<<3);

	/* inputs port a to IC39 are MA8 to MA11 from the 6845 */
	int a = (ma>>8) & 0xf;

	/* IC39 performs the 4 bit add with the carry input set high */
	int s = (a+b+1) & 0xf;

	/* if MA13 (TTXVDU) is low then IC8 and IC9 are used to calculate
	   the memory location required for the hi res video.
	   if MA13 is high then IC10 and IC11 are used to calculate the memory location for the teletext chip */
	uint16_t m;
	if (BIT(ma, 13))
		m = ((ma & 0x3ff) | 0x3c00) | ((s & 0x8)<<11);
	else
		m = ((ma & 0xff)<<3) | (s<<11) | (ra & 0x7);

	if (m_ram->size() == 16 * 1024) m &= 0x3fff;

	return m;
}

/***************************************************************************
 * Palette
 ***************************************************************************/

static const rgb_t bbc_palette[8] =
{
	rgb_t(0x0ff, 0x0ff, 0x0ff),
	rgb_t(0x000, 0x0ff, 0x0ff),
	rgb_t(0x0ff, 0x000, 0x0ff),
	rgb_t(0x000, 0x000, 0x0ff),
	rgb_t(0x0ff, 0x0ff, 0x000),
	rgb_t(0x000, 0x0ff, 0x000),
	rgb_t(0x0ff, 0x000, 0x000),
	rgb_t(0x000, 0x000, 0x000)
};

inline rgb_t bbc_state::out_rgb(rgb_t entry)
{
	float luma = float(entry.r()) * 0.299 + float(entry.g()) * 0.587 + float(entry.b()) * 0.114;
	switch (m_monitortype)
	{
	case monitor_type_t::BLACKWHITE:
		return rgb_t(luma, luma, luma);

	case monitor_type_t::GREEN:
		return rgb_t(0.2 * luma, 0.9 * luma, 0.1 * luma);

	case monitor_type_t::AMBER:
		return rgb_t(1.0 * luma, 0.8 * luma, 0.1 * luma);

	default:
		return entry;
	}
}

void bbc_state::bbc_colours(palette_device &palette) const
{
	palette.set_pen_colors(0, bbc_palette);
}

/************************************************************************
 * VideoULA
 ************************************************************************/

static const int pixels_per_byte_set[8]={ 2,4,8,16,1,2,4,8 };

static const int width_of_cursor_set[8]={ 0,0,1,2,1,0,2,4 };

/* this is a quick lookup array that puts bits 0,2,4,6 into bits 0,1,2,3
   this is used by the palette lookup in the video ULA */
void bbc_state::set_pixel_lookup()
{
	for (int i=0; i<256; i++)
	{
		m_pixel_bits[i] = (BIT(i,7)<<3) | (BIT(i,5)<<2) | (BIT(i,3)<<1) | (BIT(i,1)<<0);
	}
}


WRITE8_MEMBER(bbc_state::video_ula_w)
{
	// Make sure vpos is never <0
	int vpos = m_screen->vpos();
	if (vpos == 0)
		m_screen->update_partial(vpos);
	else
		m_screen->update_partial(vpos - 1);

	logerror("setting videoULA %.4x to:%.4x   at :%d \n", data, offset, vpos);

	switch (offset & 0x01)
	{
	// Set the control register in the Video ULA
	case 0:
		m_video_ula.master_cursor_size     = BIT(data, 7);
		m_video_ula.width_of_cursor        = (data >> 5) & 0x03;
		m_video_ula.clock_rate_6845        = BIT(data, 4);
		m_video_ula.characters_per_line    = (data >> 2) & 0x03;
		m_video_ula.teletext_normal_select = BIT(data, 1);
		m_video_ula.flash_colour_select    = BIT(data, 0);

		m_videoULA_palette_lookup = m_video_ula.flash_colour_select ? m_videoULA_palette0 : m_videoULA_palette1;

		m_cursor_size = width_of_cursor_set[m_video_ula.width_of_cursor | (m_video_ula.master_cursor_size << 2)];

		// this is the number of BBC pixels held in each byte
		if (m_video_ula.teletext_normal_select)
			m_pixels_per_byte = 12;
		else
			m_pixels_per_byte = pixels_per_byte_set[m_video_ula.characters_per_line | (m_video_ula.clock_rate_6845 << 2)];

		m_hd6845->set_hpixels_per_column(m_pixels_per_byte);
		if (m_video_ula.clock_rate_6845)
			m_hd6845->set_unscaled_clock(16_MHz_XTAL / 8);
		else
			m_hd6845->set_unscaled_clock(16_MHz_XTAL / 16);

		// FIXME: double clock for MODE7 until interlace is implemented
		if (m_video_ula.teletext_normal_select)
			m_hd6845->set_unscaled_clock(16_MHz_XTAL / 8);
		break;
	// Set a palette register in the Video ULA
	case 1:
		uint8_t tpal = (data >> 4) & 0x0f;
		uint8_t tcol = data & 0x0f;
		m_videoULA_palette0[tpal] = tcol;
		m_videoULA_palette1[tpal] = tcol<8 ? tcol : tcol ^ 7;
		break;
	}
}

/************************************************************************
 * BBC circuits controlled by 6845 Outputs
 ************************************************************************/

MC6845_UPDATE_ROW( bbc_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	if (m_video_ula.teletext_normal_select)
	{
		m_trom->lose_w(1);
		m_trom->lose_w(0);

		for (int x_pos = 0; x_pos < x_count; x_pos++)
		{
			/* Teletext Latch bits 0 to 5 go to bits 0 to 5 on the Teletext chip
			   Teletext Latch bit 6 is only passed onto bits 6 on the Teletext chip if DE is true
			   Teletext Latch bit 7 goes to LOSE on the Teletext chip */

			if (BIT(ma, 13) == 0)
				m_teletext_latch = 0;
			else
				m_teletext_latch = (m_video_ram[calculate_video_address(ma + x_pos, ra)] & 0x7f) | (m_teletext_latch & 0x80);

			m_trom->write((m_teletext_latch & 0x3f) | (m_teletext_latch & 0x40) | (de ? 0x00 : 0x40));

			m_trom->f1_w(1);
			m_trom->f1_w(0);

			for (int pixelno = 0; pixelno < 12; pixelno++)
			{
				m_trom->tr6_w(1);
				m_trom->tr6_w(0);

				int col = m_trom->get_rgb() ^ ((x_pos == cursor_x) ? 7 : 0);

				int r = BIT(col, 0) * 0xff;
				int g = BIT(col, 1) * 0xff;
				int b = BIT(col, 2) * 0xff;

				rgb_t rgb = out_rgb(rgb_t(r, g, b));

				bitmap.pix32(y, (x_pos*m_pixels_per_byte) + pixelno) = de ? rgb : rgb_t::black();
			}
		}
	}
	else
	{
		for (int x_pos = 0; x_pos<x_count; x_pos++)
		{
			uint8_t i = m_video_ram[calculate_video_address(ma + x_pos, ra)];

			for (int pixelno = 0; pixelno < m_pixels_per_byte; pixelno++)
			{
				int col = !(ra & 0x08) ? m_videoULA_palette_lookup[m_pixel_bits[i]] : 7;

				col ^= ((cursor_x != -1 && x_pos >= cursor_x && x_pos < (cursor_x + m_cursor_size)) ? 7 : 0);

				bitmap.pix32(y, (x_pos*m_pixels_per_byte) + pixelno) = de ? out_rgb(palette[col]) : rgb_t::black();
				i = (i << 1) | 1;
			}
		}
	}
}

WRITE_LINE_MEMBER(bbc_state::bbc_hsync_changed)
{
	m_hsync = state;
}

WRITE_LINE_MEMBER(bbc_state::bbc_vsync_changed)
{
	m_vsync = state;
	m_via6522_0->write_ca1(state); // screen refresh interrupts
	m_trom->dew_w(state);
}

WRITE_LINE_MEMBER(bbc_state::bbc_de_changed)
{
	m_video_ula.de = !(BIT(m_hd6845->get_ra(), 3)) && state;

	if (!state)
		m_teletext_latch |= 0x80;
	else
		m_teletext_latch &= ~0x80;
}

/**** BBC B+/Master Shadow Ram change ****/

void bbc_state::setvideoshadow(int vdusel)
{
	// LYNNE lives at 0xb000 in our map, but the offset we use here is 0x8000
	// as the video circuitry will already be looking at 0x3000 or so above
	// the offset.
	if (vdusel)
		m_video_ram = m_ram->pointer() + 0x8000;
	else
		m_video_ram = m_ram->pointer();
}

/************************************************************************
 * Initialize the BBC video emulation
 ************************************************************************/

void bbc_state::video_start()
{
	m_cursor_size = 1;

	set_pixel_lookup();

	m_video_ram = m_ram->pointer();
}
