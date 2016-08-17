// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert
/*

    TODO:

    - rewrite shifter
    - STe pixelofs
    - blitter hog
    - high resolution

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/ram.h"
#include "video/atarist.h"
#include "includes/atarist.h"



//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define LOG 0

static const int BLITTER_NOPS[16][4] =
{
	{ 1, 1, 1, 1 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 1, 1, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 1, 1, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 1, 1, 1, 1 }
};


//**************************************************************************
//  SHIFTER
//**************************************************************************

//-------------------------------------------------
//  shift_mode_0 -
//-------------------------------------------------

inline pen_t st_state::shift_mode_0()
{
	int color = (BIT(m_shifter_rr[3], 15) << 3) | (BIT(m_shifter_rr[2], 15) << 2) | (BIT(m_shifter_rr[1], 15) << 1) | BIT(m_shifter_rr[0], 15);

	m_shifter_rr[0] <<= 1;
	m_shifter_rr[1] <<= 1;
	m_shifter_rr[2] <<= 1;
	m_shifter_rr[3] <<= 1;

	return m_palette->pen(color);
}


//-------------------------------------------------
//  shift_mode_1 -
//-------------------------------------------------

inline pen_t st_state::shift_mode_1()
{
	int color = (BIT(m_shifter_rr[1], 15) << 1) | BIT(m_shifter_rr[0], 15);

	m_shifter_rr[0] <<= 1;
	m_shifter_rr[1] <<= 1;
	m_shifter_shift++;

	if (m_shifter_shift == 16)
	{
		m_shifter_rr[0] = m_shifter_rr[2];
		m_shifter_rr[1] = m_shifter_rr[3];
		m_shifter_rr[2] = m_shifter_rr[3] = 0;
		m_shifter_shift = 0;
	}

	return m_palette->pen(color);
}


//-------------------------------------------------
//  shift_mode_2 -
//-------------------------------------------------

inline pen_t st_state::shift_mode_2()
{
	int color = BIT(m_shifter_rr[0], 15);

	m_shifter_rr[0] <<= 1;
	m_shifter_shift++;

	switch (m_shifter_shift)
	{
	case 16:
		m_shifter_rr[0] = m_shifter_rr[1];
		m_shifter_rr[1] = m_shifter_rr[2];
		m_shifter_rr[2] = m_shifter_rr[3];
		m_shifter_rr[3] = 0;
		break;

	case 32:
		m_shifter_rr[0] = m_shifter_rr[1];
		m_shifter_rr[1] = m_shifter_rr[2];
		m_shifter_rr[2] = 0;
		break;

	case 48:
		m_shifter_rr[0] = m_shifter_rr[1];
		m_shifter_rr[1] = 0;
		m_shifter_shift = 0;
		break;
	}

	return m_palette->pen(color);
}


//-------------------------------------------------
//  shifter_tick -
//-------------------------------------------------

void st_state::shifter_tick()
{
	int y = machine().first_screen()->vpos();
	int x = machine().first_screen()->hpos();

	pen_t pen;

	switch (m_shifter_mode)
	{
	case 0:
		pen = shift_mode_0();
		break;

	case 1:
		pen = shift_mode_1();
		break;

	case 2:
		pen = shift_mode_2();
		break;

	default:
		pen = m_palette->black_pen();
		break;
	}

	m_bitmap.pix32(y, x) = pen;
}


//-------------------------------------------------
//  shifter_load -
//-------------------------------------------------

inline void st_state::shifter_load()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT16 data = program.read_word(m_shifter_ofs);

	m_shifter_ir[m_shifter_bitplane] = data;
	m_shifter_bitplane++;
	m_shifter_ofs += 2;

	if (m_shifter_bitplane == 4)
	{
		m_shifter_bitplane = 0;

		m_shifter_rr[0] = m_shifter_ir[0];
		m_shifter_rr[1] = m_shifter_ir[1];
		m_shifter_rr[2] = m_shifter_ir[2];
		m_shifter_rr[3] = m_shifter_ir[3];
	}
}


//-------------------------------------------------
//  glue_tick -
//-------------------------------------------------

void st_state::glue_tick()
{
	int y = machine().first_screen()->vpos();
	int x = machine().first_screen()->hpos();

	int v = (y >= m_shifter_y_start) && (y < m_shifter_y_end);
	int h = (x >= m_shifter_x_start) && (x < m_shifter_x_end);

	if(m_shifter_mode == 1) {
		int dt = 8;
		h = (x >= m_shifter_x_start-dt) && (x < m_shifter_x_end-dt);
	}
	int de = h && v;

	if(!x) {
		m_shifter_bitplane = 0;
		m_shifter_shift = 0;
	}

	if (de != m_shifter_de)
	{
		m_mfp->tbi_w(de);
		m_shifter_de = de;
	}

	if (de)
	{
		shifter_load();
	}

	if ((y == m_shifter_vblank_start) && (x == 0))
	{
		m_maincpu->set_input_line(M68K_IRQ_4, HOLD_LINE);
		m_shifter_ofs = m_shifter_base;
	}

	if (x == m_shifter_hblank_start)
	{
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE);
//      m_shifter_ofs += (m_shifter_lineofs * 2); // STe
	}

	pen_t pen;

	switch (m_shifter_mode)
	{
	case 0:
		pen = shift_mode_0();
		m_bitmap.pix32(y, x) = pen;
		m_bitmap.pix32(y, x+1) = pen;
		pen = shift_mode_0();
		m_bitmap.pix32(y, x+2) = pen;
		m_bitmap.pix32(y, x+3) = pen;
		pen = shift_mode_0();
		m_bitmap.pix32(y, x+4) = pen;
		m_bitmap.pix32(y, x+5) = pen;
		pen = shift_mode_0();
		m_bitmap.pix32(y, x+6) = pen;
		m_bitmap.pix32(y, x+7) = pen;
		break;

	case 1:
		pen = shift_mode_1();
		m_bitmap.pix32(y, x) = pen;
		pen = shift_mode_1();
		m_bitmap.pix32(y, x+1) = pen;
		pen = shift_mode_1();
		m_bitmap.pix32(y, x+2) = pen;
		pen = shift_mode_1();
		m_bitmap.pix32(y, x+3) = pen;
		pen = shift_mode_1();
		m_bitmap.pix32(y, x+4) = pen;
		pen = shift_mode_1();
		m_bitmap.pix32(y, x+5) = pen;
		pen = shift_mode_1();
		m_bitmap.pix32(y, x+6) = pen;
		pen = shift_mode_1();
		m_bitmap.pix32(y, x+7) = pen;
		break;

	case 2:
		pen = shift_mode_2();
		break;

	default:
		pen = m_palette->black_pen();
		break;
	}
}


//-------------------------------------------------
//  set_screen_parameters -
//-------------------------------------------------

void st_state::set_screen_parameters()
{
	if (m_shifter_sync & 0x02)
	{
		m_shifter_x_start = ATARIST_HBDEND_PAL*2;
		m_shifter_x_end = ATARIST_HBDSTART_PAL*2;
		m_shifter_y_start = ATARIST_VBDEND_PAL;
		m_shifter_y_end = ATARIST_VBDSTART_PAL;
		m_shifter_hblank_start = ATARIST_HBSTART_PAL*2;
		m_shifter_vblank_start = ATARIST_VBSTART_PAL;
	}
	else
	{
		m_shifter_x_start = ATARIST_HBDEND_NTSC*2;
		m_shifter_x_end = ATARIST_HBDSTART_NTSC*2;
		m_shifter_y_start = ATARIST_VBDEND_NTSC;
		m_shifter_y_end = ATARIST_VBDSTART_NTSC;
		m_shifter_hblank_start = ATARIST_HBSTART_NTSC*2;
		m_shifter_vblank_start = ATARIST_VBSTART_NTSC;
	}
}


//-------------------------------------------------
//  shifter_base_r -
//-------------------------------------------------

READ8_MEMBER( st_state::shifter_base_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 0x00:
		data = (m_shifter_base >> 16) & 0x3f;
		break;

	case 0x01:
		data = (m_shifter_base >> 8) & 0xff;
		break;
	}

	return data;
}


//-------------------------------------------------
//  shifter_base_w -
//-------------------------------------------------

WRITE8_MEMBER( st_state::shifter_base_w )
{
	switch (offset)
	{
	case 0x00:
		m_shifter_base = (m_shifter_base & 0x00ff00) | (data & 0x3f) << 16;
		logerror("SHIFTER Video Base Address %06x\n", m_shifter_base);
		break;

	case 0x01:
		m_shifter_base = (m_shifter_base & 0x3f0000) | (data << 8);
		logerror("SHIFTER Video Base Address %06x\n", m_shifter_base);
		break;
	}
}


//-------------------------------------------------
//  shifter_counter_r -
//-------------------------------------------------

READ8_MEMBER( st_state::shifter_counter_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 0x00:
		data = (m_shifter_ofs >> 16) & 0x3f;
		break;

	case 0x01:
		data = (m_shifter_ofs >> 8) & 0xff;
		break;

	case 0x02:
		data = m_shifter_ofs & 0xff;
		break;
	}

	return data;
}


//-------------------------------------------------
//  shifter_sync_r -
//-------------------------------------------------

READ8_MEMBER( st_state::shifter_sync_r )
{
	return m_shifter_sync;
}


//-------------------------------------------------
//  shifter_sync_w -
//-------------------------------------------------

WRITE8_MEMBER( st_state::shifter_sync_w )
{
	m_shifter_sync = data;
	logerror("SHIFTER Sync %x\n", m_shifter_sync);
	set_screen_parameters();
}


//-------------------------------------------------
//  shifter_mode_r -
//-------------------------------------------------

READ8_MEMBER( st_state::shifter_mode_r )
{
	return m_shifter_mode;
}


//-------------------------------------------------
//  shifter_mode_w -
//-------------------------------------------------

WRITE8_MEMBER( st_state::shifter_mode_w )
{
	m_shifter_mode = data;
	logerror("SHIFTER Mode %x\n", m_shifter_mode);
}


//-------------------------------------------------
//  shifter_palette_r -
//-------------------------------------------------

READ16_MEMBER( st_state::shifter_palette_r )
{
	return m_shifter_palette[offset] | 0xf888;
}


//-------------------------------------------------
//  shifter_palette_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::shifter_palette_w )
{
	m_shifter_palette[offset] = data;
	//  logerror("SHIFTER Palette[%x] = %x\n", offset, data);

	m_palette->set_pen_color(offset, pal3bit(data >> 8), pal3bit(data >> 4), pal3bit(data));
}



//**************************************************************************
//  STE SHIFTER
//**************************************************************************

//-------------------------------------------------
//  shifter_base_low_r -
//-------------------------------------------------

READ8_MEMBER( ste_state::shifter_base_low_r )
{
	return m_shifter_base & 0xfe;
}


//-------------------------------------------------
//  shifter_base_low_w -
//-------------------------------------------------

WRITE8_MEMBER( ste_state::shifter_base_low_w )
{
	m_shifter_base = (m_shifter_base & 0x3fff00) | (data & 0xfe);
	logerror("SHIFTER Video Base Address %06x\n", m_shifter_base);
}


//-------------------------------------------------
//  shifter_counter_r -
//-------------------------------------------------

READ8_MEMBER( ste_state::shifter_counter_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 0x00:
		data = (m_shifter_ofs >> 16) & 0x3f;
		break;

	case 0x01:
		data = (m_shifter_ofs >> 8) & 0xff;
		break;

	case 0x02:
		data = m_shifter_ofs & 0xfe;
		break;
	}

	return data;
}


//-------------------------------------------------
//  shifter_counter_w -
//-------------------------------------------------

WRITE8_MEMBER( ste_state::shifter_counter_w )
{
	switch (offset)
	{
	case 0x00:
		m_shifter_ofs = (m_shifter_ofs & 0x00fffe) | (data & 0x3f) << 16;
		logerror("SHIFTER Video Address Counter %06x\n", m_shifter_ofs);
		break;

	case 0x01:
		m_shifter_ofs = (m_shifter_ofs & 0x3f00fe) | (data << 8);
		logerror("SHIFTER Video Address Counter %06x\n", m_shifter_ofs);
		break;

	case 0x02:
		m_shifter_ofs = (m_shifter_ofs & 0x3fff00) | (data & 0xfe);
		logerror("SHIFTER Video Address Counter %06x\n", m_shifter_ofs);
		break;
	}
}


//-------------------------------------------------
//  shifter_palette_w -
//-------------------------------------------------

WRITE16_MEMBER( ste_state::shifter_palette_w )
{
	int r = ((data >> 7) & 0x0e) | BIT(data, 11);
	int g = ((data >> 3) & 0x0e) | BIT(data, 7);
	int b = ((data << 1) & 0x0e) | BIT(data, 3);

	m_shifter_palette[offset] = data;
	logerror("SHIFTER palette %x = %x\n", offset, data);

	m_palette->set_pen_color(offset, r, g, b);
}


//-------------------------------------------------
//  shifter_lineofs_r -
//-------------------------------------------------

READ8_MEMBER( ste_state::shifter_lineofs_r )
{
	return m_shifter_lineofs;
}


//-------------------------------------------------
//  shifter_lineofs_w -
//-------------------------------------------------

WRITE8_MEMBER( ste_state::shifter_lineofs_w )
{
	m_shifter_lineofs = data;
	logerror("SHIFTER Line Offset %x\n", m_shifter_lineofs);
}


//-------------------------------------------------
//  shifter_pixelofs_r -
//-------------------------------------------------

READ8_MEMBER( ste_state::shifter_pixelofs_r )
{
	return m_shifter_pixelofs;
}


//-------------------------------------------------
//  shifter_pixelofs_w -
//-------------------------------------------------

WRITE8_MEMBER( ste_state::shifter_pixelofs_w )
{
	m_shifter_pixelofs = data & 0x0f;
	logerror("SHIFTER Pixel Offset %x\n", m_shifter_pixelofs);
}



//**************************************************************************
//  BLITTER
//**************************************************************************

//-------------------------------------------------
//  blitter_source -
//-------------------------------------------------

void st_state::blitter_source()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT16 data = program.read_word(m_blitter_src);

	if (m_blitter_src_inc_x < 0)
	{
		m_blitter_srcbuf = (data << 16) | (m_blitter_srcbuf >> 16);
	}
	else
	{
		m_blitter_srcbuf = (m_blitter_srcbuf << 16) | data;
	}
}


//-------------------------------------------------
//  blitter_hop -
//-------------------------------------------------

UINT16 st_state::blitter_hop()
{
	UINT16 source = m_blitter_srcbuf >> (m_blitter_skew & 0x0f);
	UINT16 halftone = m_blitter_halftone[m_blitter_ctrl & 0x0f];

	if (m_blitter_ctrl & ATARIST_BLITTER_CTRL_SMUDGE)
	{
		halftone = m_blitter_halftone[source & 0x0f];
	}

	switch (m_blitter_hop)
	{
	case 0:
		return 0xffff;
	case 1:
		return halftone;
	case 2:
		return source;
	case 3:
		return source & halftone;
	}

	return 0;
}


//-------------------------------------------------
//  blitter_op -
//-------------------------------------------------

void st_state::blitter_op(UINT16 s, UINT32 dstaddr, UINT16 mask)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	UINT16 d = program.read_word(dstaddr);
	UINT16 result = 0;

	if (m_blitter_op & 0x08) result = (~s & ~d);
	if (m_blitter_op & 0x04) result |= (~s & d);
	if (m_blitter_op & 0x02) result |= (s & ~d);
	if (m_blitter_op & 0x01) result |= (s & d);

	program.write_word(dstaddr, result);
}


//-------------------------------------------------
//  blitter_tick -
//-------------------------------------------------

void st_state::blitter_tick()
{
	do
	{
		if (m_blitter_skew & ATARIST_BLITTER_SKEW_FXSR)
		{
			blitter_source();
			m_blitter_src += m_blitter_src_inc_x;
		}

		blitter_source();
		blitter_op(blitter_hop(), m_blitter_dst, m_blitter_endmask1);
		m_blitter_xcount--;

		while (m_blitter_xcount > 0)
		{
			m_blitter_src += m_blitter_src_inc_x;
			m_blitter_dst += m_blitter_dst_inc_x;

			if (m_blitter_xcount == 1)
			{
				if (!(m_blitter_skew & ATARIST_BLITTER_SKEW_NFSR))
				{
					blitter_source();
				}

				blitter_op(blitter_hop(), m_blitter_dst, m_blitter_endmask3);
			}
			else
			{
				blitter_source();
				blitter_op(blitter_hop(), m_blitter_dst, m_blitter_endmask2);
			}

			m_blitter_xcount--;
		}

		m_blitter_src += m_blitter_src_inc_y;
		m_blitter_dst += m_blitter_dst_inc_y;

		if (m_blitter_dst_inc_y < 0)
		{
			m_blitter_ctrl = (m_blitter_ctrl & 0xf0) | (((m_blitter_ctrl & 0x0f) - 1) & 0x0f);
		}
		else
		{
			m_blitter_ctrl = (m_blitter_ctrl & 0xf0) | (((m_blitter_ctrl & 0x0f) + 1) & 0x0f);
		}

		m_blitter_xcount = m_blitter_xcountl;
		m_blitter_ycount--;
	}
	while (m_blitter_ycount > 0);

	m_blitter_ctrl &= 0x7f;

	m_mfp->i3_w(0);
}


//-------------------------------------------------
//  blitter_halftone_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_halftone_r )
{
	return m_blitter_halftone[offset];
}


//-------------------------------------------------
//  blitter_src_inc_x_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_src_inc_x_r )
{
	return m_blitter_src_inc_x;
}


//-------------------------------------------------
//  blitter_src_inc_y_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_src_inc_y_r )
{
	return m_blitter_src_inc_y;
}


//-------------------------------------------------
//  blitter_src_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_src_r )
{
	switch (offset)
	{
	case 0:
		return (m_blitter_src >> 16) & 0xff;
	case 1:
		return m_blitter_src & 0xfffe;
	}

	return 0;
}


//-------------------------------------------------
//  blitter_end_mask_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_end_mask_r )
{
	switch (offset)
	{
	case 0:
		return m_blitter_endmask1;
	case 1:
		return m_blitter_endmask2;
	case 2:
		return m_blitter_endmask3;
	}

	return 0;
}


//-------------------------------------------------
//  blitter_dst_inc_x_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_dst_inc_x_r )
{
	return m_blitter_dst_inc_x;
}


//-------------------------------------------------
//  blitter_dst_inc_y_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_dst_inc_y_r )
{
	return m_blitter_dst_inc_y;
}


//-------------------------------------------------
//  blitter_dst_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_dst_r )
{
	switch (offset)
	{
	case 0:
		return (m_blitter_dst >> 16) & 0xff;
	case 1:
		return m_blitter_dst & 0xfffe;
	}

	return 0;
}


//-------------------------------------------------
//  blitter_count_x_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_count_x_r )
{
	return m_blitter_xcount;
}


//-------------------------------------------------
//  blitter_count_y_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_count_y_r )
{
	return m_blitter_ycount;
}


//-------------------------------------------------
//  blitter_op_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_op_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_blitter_hop;
	}
	else
	{
		return m_blitter_op;
	}
}


//-------------------------------------------------
//  blitter_ctrl_r -
//-------------------------------------------------

READ16_MEMBER( st_state::blitter_ctrl_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_blitter_ctrl;
	}
	else
	{
		return m_blitter_skew;
	}
}


//-------------------------------------------------
//  blitter_halftone_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_halftone_w )
{
	m_blitter_halftone[offset] = data;
}


//-------------------------------------------------
//  blitter_src_inc_x_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_src_inc_x_w )
{
	m_blitter_src_inc_x = data & 0xfffe;
}


//-------------------------------------------------
//  blitter_src_inc_y_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_src_inc_y_w )
{
	m_blitter_src_inc_y = data & 0xfffe;
}


//-------------------------------------------------
//  blitter_src_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_src_w )
{
	switch (offset)
	{
	case 0:
		m_blitter_src = (data & 0xff) | (m_blitter_src & 0xfffe);
		break;

	case 1:
		m_blitter_src = (m_blitter_src & 0xff0000) | (data & 0xfffe);
		break;
	}
}


//-------------------------------------------------
//  blitter_end_mask_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_end_mask_w )
{
	switch (offset)
	{
	case 0:
		m_blitter_endmask1 = data;
		break;

	case 1:
		m_blitter_endmask2 = data;
		break;

	case 2:
		m_blitter_endmask3 = data;
		break;
	}
}


//-------------------------------------------------
//  blitter_dst_inc_x_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_dst_inc_x_w )
{
	m_blitter_dst_inc_x = data & 0xfffe;
}


//-------------------------------------------------
//  blitter_dst_inc_y_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_dst_inc_y_w )
{
	m_blitter_dst_inc_y = data & 0xfffe;
}


//-------------------------------------------------
//  blitter_dst_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_dst_w )
{
	switch (offset)
	{
	case 0:
		m_blitter_dst = (data & 0xff) | (m_blitter_dst & 0xfffe);
		break;

	case 1:
		m_blitter_dst = (m_blitter_dst & 0xff0000) | (data & 0xfffe);
		break;
	}
}


//-------------------------------------------------
//  blitter_count_x_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_count_x_w )
{
	m_blitter_xcount = data;
}


//-------------------------------------------------
//  blitter_count_y_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_count_y_w )
{
	m_blitter_ycount = data;
}


//-------------------------------------------------
//  blitter_op_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_op_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_blitter_hop = (data >> 8) & 0x03;
	}
	else
	{
		m_blitter_op = data & 0x0f;
	}
}


//-------------------------------------------------
//  blitter_ctrl_w -
//-------------------------------------------------

WRITE16_MEMBER( st_state::blitter_ctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_blitter_ctrl = (data >> 8) & 0xef;

		if (!(m_blitter_ctrl & ATARIST_BLITTER_CTRL_BUSY))
		{
			if ((data >> 8) & ATARIST_BLITTER_CTRL_BUSY)
			{
				m_mfp->i3_w(1);

				int nops = BLITTER_NOPS[m_blitter_op][m_blitter_hop]; // each NOP takes 4 cycles
				timer_set(attotime::from_hz((Y2/4)/(4*nops)), TIMER_BLITTER_TICK);
			}
		}
	}
	else
	{
		m_blitter_skew = data & 0xcf;
	}
}



//**************************************************************************
//  VIDEO
//**************************************************************************

void st_state::video_start()
{
	m_shifter_timer = timer_alloc(TIMER_SHIFTER_TICK);
	m_glue_timer = timer_alloc(TIMER_GLUE_TICK);

//  m_shifter_timer->adjust(machine().first_screen()->time_until_pos(0), 0, attotime::from_hz(Y2/4)); // 125 ns
	m_glue_timer->adjust(machine().first_screen()->time_until_pos(0), 0, attotime::from_hz(Y2/16)); // 500 ns

	machine().first_screen()->register_screen_bitmap(m_bitmap);

	/* register for state saving */
	save_item(NAME(m_shifter_base));
	save_item(NAME(m_shifter_ofs));
	save_item(NAME(m_shifter_sync));
	save_item(NAME(m_shifter_mode));
	save_item(NAME(m_shifter_palette));
	save_item(NAME(m_shifter_rr));
	save_item(NAME(m_shifter_ir));
	save_item(NAME(m_shifter_bitplane));
	save_item(NAME(m_shifter_shift));
	save_item(NAME(m_shifter_h));
	save_item(NAME(m_shifter_v));
	save_item(NAME(m_shifter_de));

	save_item(NAME(m_blitter_halftone));
	save_item(NAME(m_blitter_src_inc_x));
	save_item(NAME(m_blitter_src_inc_y));
	save_item(NAME(m_blitter_dst_inc_x));
	save_item(NAME(m_blitter_dst_inc_y));
	save_item(NAME(m_blitter_src));
	save_item(NAME(m_blitter_dst));
	save_item(NAME(m_blitter_endmask1));
	save_item(NAME(m_blitter_endmask2));
	save_item(NAME(m_blitter_endmask3));
	save_item(NAME(m_blitter_xcount));
	save_item(NAME(m_blitter_ycount));
	save_item(NAME(m_blitter_xcountl));
	save_item(NAME(m_blitter_hop));
	save_item(NAME(m_blitter_op));
	save_item(NAME(m_blitter_ctrl));
	save_item(NAME(m_blitter_skew));

	set_screen_parameters();
}


void ste_state::video_start()
{
	st_state::video_start();

	// register for state saving
	save_item(NAME(m_shifter_lineofs));
	save_item(NAME(m_shifter_pixelofs));
}

void stbook_state::video_start()
{
}


UINT32 st_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
