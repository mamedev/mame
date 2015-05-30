// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Galeb video driver by Miodrag Milanovic

        01/03/2008 Updated to work with latest SVN code
        22/02/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/galeb.h"


const gfx_layout galeb_charlayout =
{
	8, 8,               /* 8x8 characters */
	256,                /* 256 characters */
	1,                /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{7, 6, 5, 4, 3, 2, 1, 0},
	{0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8},
	8*8                 /* size of one char */
};

void galeb_state::video_start()
{
}

UINT32 galeb_state::screen_update_galeb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;

	for(y = 0; y < 16; y++ )
	{
		for(x = 0; x < 48; x++ )
		{
			int code = m_video_ram[15 + x + y*64];
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,  code , 0, 0,0, x*8,y*8);
		}
	}
	return 0;
}
