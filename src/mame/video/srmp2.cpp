// license:BSD-3-Clause
// copyright-holders:Yochizo, Takahiro Nogi
/***************************************************************************

Functions to emulate the video hardware of the machine.

***************************************************************************/


#include "emu.h"
#include "includes/srmp2.h"

PALETTE_INIT_MEMBER(srmp2_state,srmp2)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int col;

		col = (color_prom[i] << 8) + color_prom[i + palette.entries()];
		palette.set_pen_color(i ^ 0x0f,pal5bit(col >> 10),pal5bit(col >> 5),pal5bit(col >> 0));
	}
}


PALETTE_INIT_MEMBER(srmp2_state,srmp3)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int col;

		col = (color_prom[i] << 8) + color_prom[i + palette.entries()];
		palette.set_pen_color(i,pal5bit(col >> 10),pal5bit(col >> 5),pal5bit(col >> 0));
	}
}

SETA001_SPRITE_GFXBANK_CB_MEMBER(srmp2_state::srmp3_gfxbank_callback)
{
	if (code & 0x2000)
	{
		code = (code & 0x1fff);
		code += ((m_gfx_bank + 1) * 0x2000);
	}

	return code;
}


UINT32 srmp2_state::screen_update_srmp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1ff, cliprect);

	m_seta001->set_transpen(15);

	m_seta001->set_colorbase((m_color_bank)?0x20:0x00);

	m_seta001->set_fg_xoffsets( 0x10, 0x10 );
	m_seta001->set_fg_yoffsets( 0x05, 0x07 );
	m_seta001->set_bg_xoffsets( 0x00, 0x00 ); // bg not used?
	m_seta001->set_bg_yoffsets( 0x00, 0x00 ); // bg not used?

	m_seta001->draw_sprites(screen,bitmap,cliprect,0x1000, 1);
	return 0;
}

UINT32 srmp2_state::screen_update_srmp3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_seta001->set_fg_xoffsets( 0x10, 0x10 );
	m_seta001->set_fg_yoffsets( 0x06, 0x06 );
	m_seta001->set_bg_xoffsets( -0x01, 0x10 );
	m_seta001->set_bg_yoffsets( -0x06, 0x06 );

	m_seta001->draw_sprites(screen,bitmap,cliprect,0x1000, 1);
	return 0;
}

UINT32 srmp2_state::screen_update_mjyuugi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_seta001->set_fg_xoffsets( 0x10, 0x10 );
	m_seta001->set_fg_yoffsets( 0x06, 0x06 );
	m_seta001->set_bg_yoffsets( 0x09, 0x07 );

	m_seta001->set_spritelimit( 0x1ff-6 );

	m_seta001->draw_sprites(screen,bitmap,cliprect,0x1000, 1);
	return 0;
}
