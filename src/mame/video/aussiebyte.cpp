// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************

    Video
    Graphics not working properly.
    Mode 0 - lores (wide, chunky)
    Mode 1 - external, *should* be ok
    Mode 2 - thin graphics, not explained well enough to code
    Mode 3 - alphanumeric, works



************************************************************/
#include "includes/aussiebyte.h"
/***********************************************************

    I/O Ports

************************************************************/

// dummy read port, forces requested action to happen
READ8_MEMBER( aussiebyte_state::port33_r )
{
	return 0xff;
}

/*
Video control - needs to be fully understood
d0, d1, d2, d3 - can replace RA0-3 in graphics mode
d4 - GS - unknown
d5 - /SRRD - controls write of data to either vram or aram (1=vram, 0=aram)
d6 - /VWR - 0 = enable write vdata to vram, read from aram to vdata ; 1 = enable write to aram from vdata
d7 - OE on port 35
*/
WRITE8_MEMBER( aussiebyte_state::port34_w )
{
	m_port34 = data;
}

WRITE8_MEMBER( aussiebyte_state::port35_w )
{
	m_port35 = data;
}

READ8_MEMBER( aussiebyte_state::port36_r )
{
	if BIT(m_port34, 5)
	{
		if BIT(m_p_attribram[m_alpha_address & 0x7ff], 7)
			return m_p_videoram[m_alpha_address];
		else
			return m_p_videoram[m_graph_address];
	}
	else
		return m_p_attribram[m_alpha_address & 0x7ff];
}

READ8_MEMBER( aussiebyte_state::port37_r )
{
	return m_crtc->de_r() ? 0xff : 0xfe;
}


/***********************************************************

    Video

************************************************************/
MC6845_ON_UPDATE_ADDR_CHANGED( aussiebyte_state::crtc_update_addr )
{
/* not sure what goes in here - parameters passed are device, address, strobe */
//  m_video_address = address;// & 0x7ff;
}

WRITE8_MEMBER( aussiebyte_state::address_w )
{
	m_crtc->address_w( space, 0, data );

	m_video_index = data & 0x1f;

	if (m_video_index == 31)
	{
		m_alpha_address++;
		m_alpha_address &= 0x3fff;
		m_graph_address = (m_alpha_address << 4) | (m_port34 & 15);

		if BIT(m_port34, 5)
		{
			if BIT(m_p_attribram[m_alpha_address & 0x7ff], 7)
				m_p_videoram[m_alpha_address] = m_port35;
			else
				m_p_videoram[m_graph_address] = m_port35;
		}
		else
			m_p_attribram[m_alpha_address & 0x7ff] = m_port35;
	}
}

WRITE8_MEMBER( aussiebyte_state::register_w )
{
	m_crtc->register_w( space, 0, data );
	UINT16 temp = m_alpha_address;

	// Get transparent address
	if (m_video_index == 18)
		m_alpha_address = (data << 8 ) | (temp & 0xff);
	else
	if (m_video_index == 19)
		m_alpha_address = data | (temp & 0xff00);
}

UINT8 aussiebyte_state::crt8002(UINT8 ac_ra, UINT8 ac_chr, UINT8 ac_attr, UINT16 ac_cnt, bool ac_curs)
{
	UINT8 gfx = 0;
	switch (ac_attr & 3)
	{
		case 0: // lores gfx
			switch (ac_ra)
			{
				case 0:
				case 1:
				case 2:
					gfx = (BIT(ac_chr, 7) ? 0xf8 : 0) | (BIT(ac_chr, 3) ? 7 : 0);
					break;
				case 3:
				case 4:
				case 5:
					gfx = (BIT(ac_chr, 6) ? 0xf8 : 0) | (BIT(ac_chr, 2) ? 7 : 0);
					break;
				case 6:
				case 7:
				case 8:
					gfx = (BIT(ac_chr, 5) ? 0xf8 : 0) | (BIT(ac_chr, 1) ? 7 : 0);
					break;
				default:
					gfx = (BIT(ac_chr, 4) ? 0xf8 : 0) | (BIT(ac_chr, 0) ? 7 : 0);
					break;
			}
			break;
		case 1: // external mode
			gfx = BITSWAP8(ac_chr, 0,1,2,3,4,5,6,7);
			break;
		case 2: // thin gfx
			break;
		case 3: // alpha
			gfx = m_p_chargen[((ac_chr & 0x7f)<<4) | ac_ra];
			break;
	}

	if (BIT(ac_attr, 3) & (ac_ra == 11)) // underline
		gfx = 0xff;
	if (BIT(ac_attr, 2) & ((ac_ra == 5) | (ac_ra == 6))) // strike-through
		gfx = 0xff;
	if (BIT(ac_attr, 6) & BIT(ac_cnt, 13)) // flash
		gfx = 0;
	if BIT(ac_attr, 5) // blank
		gfx = 0;
	if (ac_curs && BIT(ac_cnt, 14)) // cursor
		gfx ^= 0xff;
	if BIT(ac_attr, 4) // reverse video
		gfx ^= 0xff;
	return gfx;
}

MC6845_UPDATE_ROW( aussiebyte_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx,attr;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);
	ra &= 15;
	m_cnt++;

	for (x = 0; x < x_count; x++)
	{
		mem = ma + x;
		attr = m_p_attribram[mem & 0x7ff];
		if BIT(attr, 7)
			chr = m_p_videoram[mem & 0x3fff]; // alpha
		else
			chr = m_p_videoram[(mem << 4) | ra]; // gfx

		gfx = crt8002(ra, chr, attr, m_cnt, (x==cursor_x));

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}
