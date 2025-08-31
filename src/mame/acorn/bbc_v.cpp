// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/******************************************************************************

    BBC Micro and Master series video code

    TODO:
    - move palette handling to a VIDPROC device.
    - improve 6845 video emulation, fix show border area for VSYNC timing.

    Benchmark games: (+ working, - not working)
    + Alien 8 (and probably other Ultimate games) - FCxx/FDxx must return FF if no hardware present
    - Boffin - overscan
    + By Fair Means or Foul - palette change timing
    - Crazee Rider (Master)- correct handling of undocumented sound chip writes (drums will sound wrong if this is not correct)
    + Dr Who: The First Adventure - non-standard keyboard handling
    + Doctor Who and the Mines of Terror - ROM/B+ sideways RAM
    + Empire Strikes Back - interrupt timing when CLI immediately followed by SEI
    - Elite (Superior re-release) - 2nd processor, split screen mode
    - Exile - correct handling of flags in BCD mode, and palette change timing
    - Firetrack - smooth scrolling, intro part uses horizontal overscan on model B (not Master)
    - Frogger (Trickysoft) - defines multiple displays areas per frame
    - Lunar Jetman - in addition to the protection, requires a programmed VSYNC length of 0 to be treated as 16, otherwise the mode/palette split will be in the wrong place
    - Nightshade - tape protection
    + Pharaoh's Curse - lightpen abuse
    - Revs - interrupt timing (6522 T1)
    - Rig Attack - interrupt timing
    + Snapper - timer output on VIA port B
    + Spy Hunter (disc version) - sampled speech playback
    - Stryker's Run (Master)- music fidelity
    - Time and Magik (Master) - video effects
    + Ultimate games (Jet-Pac, Cookie, Atic Atac, Sabre Wulf) - correct handling of 'flash' bit in video ULA
    - Uridium - vertical rupture
    + Volcano - something VIA related
    + Zalaga - illegal opcodes
    - Some Nasty Effects - multiple palette changes per scanline
    - Skirmish - VIA timers

******************************************************************************/

#include "emu.h"
#include "bbc.h"


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
	// output from IC32 74LS259 bits 4 and 5
	int c0 = m_latch->q4_r();
	int c1 = m_latch->q5_r();

	// the 4 bit input port b on IC39 are produced by 4 NAND gates. These NAND gates take their inputs
	// from c0 and c1 (from IC32) and ma12 (from the 6845)

	// get bit m12 from the 6845
	int ma12 = BIT(ma, 12);

	// 4 bit input B on IC39 74LS283 (4 bit adder)
	// 3 input NAND part of IC 36
	int b1 = (~(c1 & c0 & ma12)) & 1;
	// 2 input NAND part of IC40 (b3 is calculated before b2 and b4 because b3 feed back into b2 and b4)
	int b3 = (~(c0 & ma12)) & 1;
	// 3 input NAND part of IC 36
	int b2 = (~(c1 & b3 & ma12)) & 1;
	// 2 input NAND part of IC 27
	int b4 = (~(b3 & ma12)) & 1;

	// inputs port b to IC39 are taken from the NAND gates b1 to b4
	int b = (b1 << 0) | (b2 << 1) | (b3 << 2) | (b4 << 3);

	// inputs port a to IC39 are MA8 to MA11 from the 6845
	int a = (ma >> 8) & 0xf;

	// IC39 performs the 4 bit add with the carry input set high
	int s = (a + b + 1) & 0xf;

	// if MA13 (TTXVDU) is low then IC8 and IC9 are used to calculate
	// the memory location required for the hires video.
	// if MA13 is high then IC10 and IC11 are used to calculate the memory location for the teletext chip
	uint16_t addr;
	if (BIT(ma, 13))
		addr = ((ma & 0x3ff) | 0x3c00) | ((s & 0x8) << 11);
	else
		addr = ((ma & 0xff) << 3) | (s << 11) | (ra & 0x7);

	if (m_ram->size() == 16 * 1024) addr &= 0x3fff;

	return addr;
}


/***************************************************************************
 * Palette
 ***************************************************************************/

void bbc_state::update_palette(monitor_type monitor_type)
{
	for (int i = 0; i < m_palette->entries(); i++)
	{
		rgb_t pen = rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
		float luma = float(pen.r()) * 0.299 + float(pen.g()) * 0.587 + float(pen.b()) * 0.114;
		switch (monitor_type)
		{
		case monitor_type::COLOUR:
			m_palette->set_pen_color(i, pen);
			break;
		case monitor_type::BLACKWHITE:
			m_palette->set_pen_color(i, rgb_t(luma, luma, luma));
			break;
		case monitor_type::GREEN:
			m_palette->set_pen_color(i, rgb_t(0.2 * luma, 0.9 * luma, 0.1 * luma));
			break;
		case monitor_type::AMBER:
			m_palette->set_pen_color(i, rgb_t(1.0 * luma, 0.8 * luma, 0.1 * luma));
			break;
		}
		m_vnula.flash[i & 7] = 1;
	}
}


/************************************************************************
 * VideoULA
 ************************************************************************/

static const int pixels_per_byte_set[8] = { 2,4,8,16,1,2,4,8 };

static const int width_of_cursor_set[8] = { 0,0,1,2,1,0,2,4 };


void bbc_state::video_ula_w(offs_t offset, uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());

	uint8_t mask = m_vnula.disable ? 0x01 : 0x03;
	switch (offset & mask)
	{
	case 0: // Video ULA control register
		// b7     Master cursor size
		// b6-b5  Width of cursor in bytes
		// b4     6845 clock rate select
		// b3-b2  Number of characters per line
		// b1     Teletext/Normal select
		// b0     Flash colour select

		// flash colour select has changed
		if (BIT(m_vula_ctrl ^ data, 0))
		{
			for (int i = 0; i < 16; i++)
			{
				if (BIT(data, 0) && BIT(m_vula_palette[i], 3) && m_vnula.flash[m_vula_palette[i] & 7])
					m_vula_palette_lookup[i] = m_vula_palette[i];
				else
					m_vula_palette_lookup[i] = m_vula_palette[i] ^ 7;
			}
		}

		m_vula_ctrl = data;
		m_cursor_size = width_of_cursor_set[(data >> 5) & 7];

		// number of pixels held in each byte
		if (BIT(data, 1))
		{
			m_pixels_per_byte = 12;
			m_pixel_width = 1;
		}
		else
		{
			m_pixels_per_byte = pixels_per_byte_set[(data >> 2) & 7];
			m_pixel_width = 1 << (~(data >> 2) & 0x03);
		}

		m_crtc->set_hpixels_per_column(m_pixels_per_byte);
		if (BIT(data, 4) || BIT(data, 1)) // FIXME: double clock for MODE7 until interlace is implemented
			m_crtc->set_unscaled_clock(16_MHz_XTAL / 8);
		else
			m_crtc->set_unscaled_clock(16_MHz_XTAL / 16);
		break;

	case 1: // Video ULA palette register
		m_vula_palette[data >> 4] = data & 0x0f;
		if (BIT(data, 3) && BIT(m_vula_ctrl, 0) && m_vnula.flash[data & 7])
			m_vula_palette_lookup[data >> 4] = data & 0x0f;
		else
			m_vula_palette_lookup[data >> 4] = (data & 0x0f) ^ 7;
		break;

	case 2: // Video NuLA auxiliary control register
		switch (data >> 4)
		{
		case 1: // Set palette mode
			m_vnula.palette_mode = data & 0x01;
			break;
		case 2: // Set horizontal scroll offset
			m_vnula.horiz_offset = data & 0x07;
			break;
		case 3: // Set left-hand side blanking size
			m_vnula.left_blank = data & 0x0f;
			break;
		case 4: // Reset additional features
			update_palette(monitor_type::COLOUR);
			for (int i = 0; i < 8; i++)
				m_vnula.flash[i] = 1;
			m_vnula.palette_mode = 0;
			m_vnula.horiz_offset = 0;
			m_vnula.left_blank = 0;
			m_vnula.attr_mode = 0;
			m_vnula.attr_text = 0;
			m_vnula.palette_write = 0;
			break;
		case 5: // Disable A1 address line
			m_vnula.disable = 1;
			break;
		case 6: // Enable/disable attribute modes
			m_vnula.attr_mode = data & 0x01;
			break;
		case 7: // Enable/disable 8-colour attribute text modes
			m_vnula.attr_text = data & 0x01;
			break;
		case 8: // Set flashing flags for colours 8-11
			m_vnula.flash[0] = data & 0x08;
			m_vnula.flash[1] = data & 0x04;
			m_vnula.flash[2] = data & 0x02;
			m_vnula.flash[3] = data & 0x01;
			break;
		case 9: // Set flashing flags for colours 12-15
			m_vnula.flash[4] = data & 0x08;
			m_vnula.flash[5] = data & 0x04;
			m_vnula.flash[6] = data & 0x02;
			m_vnula.flash[7] = data & 0x01;
			break;
		}
		break;

	case 3: // Video NuLA auxiliary palette register
		if (m_vnula.palette_write)
		{
			uint8_t col = m_vnula.palette_byte >> 4;
			m_palette->set_pen_color(col, rgb_t(pal4bit(m_vnula.palette_byte & 0x0f), pal4bit(data >> 4), pal4bit(data & 0x0f)));
			// redefined colour becomes solid
			if (BIT(col, 3))
				m_vnula.flash[col & 7] = 0;

			for (int i = 0; i < 16; i++)
			{
				if (BIT(m_vula_ctrl, 0) && BIT(m_vula_palette[i], 3) && m_vnula.flash[m_vula_palette[i] & 7])
					m_vula_palette_lookup[i] = m_vula_palette[i];
				else
					m_vula_palette_lookup[i] = m_vula_palette[i] ^ 7;
			}
		}
		else
		{
			m_vnula.palette_byte = data;
		}
		m_vnula.palette_write ^= 1;
		break;
	}
}


/************************************************************************
 * BBC circuits controlled by 6845 Outputs
 ************************************************************************/

MC6845_RECONFIGURE(bbc_state::crtc_reconfigure)
{
	// ensure all graphics modes render to same bitmap size (for split-screen modes)
	rectangle rect(visarea.min_x * m_pixel_width, visarea.min_x + (visarea.width() * m_pixel_width) - 1, visarea.min_y, visarea.max_y);
	m_screen->configure(width * m_pixel_width, height, rect, frame_period);
}

MC6845_UPDATE_ROW(bbc_state::crtc_update_row)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	uint32_t *p = &bitmap.pix(y);

	if (BIT(m_vula_ctrl, 1))
	{
		m_trom->lose_w(1);
		m_trom->lose_w(0);

		if (bitmap.height() <= 312) // HACK: not interlaced so skip a line (requires SAA5050 improvements)
		{
			m_trom->lose_w(1);
			m_trom->lose_w(0);
		}

		for (int x_pos = 0; x_pos < x_count; x_pos++)
		{
			// Teletext Latch bits 0 to 5 go to bits 0 to 5 on the Teletext chip
			// Teletext Latch bit 6 is only passed onto bits 6 on the Teletext chip if DE is true
			// Teletext Latch bit 7 goes to LOSE on the Teletext chip
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

				for (int i = 0; i < m_pixel_width; i++)
				{
					*p = de ? palette[col] : rgb_t::black(); p++;
				}
			}
		}
	}
	else
	{
		for (int x_pos = 0; x_pos < x_count; x_pos++)
		{
			uint8_t data = m_video_ram[calculate_video_address(ma + x_pos, ra)];

			for (int pixelno = 0; pixelno < m_pixels_per_byte; pixelno++)
			{
				int col = !(ra & 0x08) ? m_vula_palette_lookup[bitswap<4>(data, 7, 5, 3, 1)] : 0;

				col ^= ((cursor_x != -1 && x_pos >= cursor_x && x_pos < (cursor_x + m_cursor_size)) ? 7 : 0);

				for (int i = 0; i < m_pixel_width; i++)
				{
					*p = de ? palette[col] : rgb_t::black(); p++;
				}
				data = (data << 1) | 1;
			}
		}
	}
}

void bbc_state::hsync_changed(int state)
{
	//m_trom->glr_w(!state);
}

void bbc_state::vsync_changed(int state)
{
	m_sysvia->write_ca1(state); // screen refresh interrupt
	m_trom->dew_w(state);
}

void bbc_state::de_changed(int state)
{
	if (!state)
		m_teletext_latch |= 0x80;
	else
		m_teletext_latch &= ~0x80;
}

uint8_t bbc_state::bus_video_data()
{
	int hpos = m_screen->hpos();
	int vpos = m_screen->vpos();

	return m_video_ram[calculate_video_address(hpos, vpos)];
}


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
	m_video_ram = m_ram->pointer();

	// register save states
	save_item(NAME(m_vula_ctrl));
	save_item(NAME(m_vula_palette));
	save_item(NAME(m_vula_palette_lookup));
	save_item(STRUCT_MEMBER(m_vnula, palette_mode));
	save_item(STRUCT_MEMBER(m_vnula, horiz_offset));
	save_item(STRUCT_MEMBER(m_vnula, left_blank));
	save_item(STRUCT_MEMBER(m_vnula, disable));
	save_item(STRUCT_MEMBER(m_vnula, flash));
	save_item(STRUCT_MEMBER(m_vnula, palette_byte));
	save_item(STRUCT_MEMBER(m_vnula, palette_write));
}

void bbc_state::video_reset()
{
	for (int i = 0; i < 8; i++)
		m_vnula.flash[i]  = 1;
	m_vnula.palette_mode  = 0;
	m_vnula.horiz_offset  = 0;
	m_vnula.left_blank    = 0;
	m_vnula.disable       = !BIT(m_bbcconfig.read_safe(0), 3);
	m_vnula.palette_byte  = 0;
	m_vnula.palette_write = 0;
}
