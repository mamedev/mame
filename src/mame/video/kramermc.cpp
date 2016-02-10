// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Kramer MC video driver by Miodrag Milanovic

        13/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/kramermc.h"

#define KRAMERMC_VIDEO_MEMORY       0xFC00

const gfx_layout kramermc_charlayout =
{
	8, 8,               /* 8x8 characters */
	256,                /* 256 characters */
	1,                /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{0, 1, 2, 3, 4, 5, 6, 7},
	{0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8},
	8*8                 /* size of one char */
};

void kramermc_state::video_start()
{
}

UINT32 kramermc_state::screen_update_kramermc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	for(y = 0; y < 16; y++ )
	{
		for(x = 0; x < 64; x++ )
		{
			int code = space.read_byte(KRAMERMC_VIDEO_MEMORY + x + y*64);
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,  code , 0, 0,0, x*8,y*8);
		}
	}
	return 0;
}
