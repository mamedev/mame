// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/tutankhm.h"


#define NUM_PENS    (0x10)


/*************************************
 *
 *  Write handlers
 *
 *************************************/

WRITE8_MEMBER(tutankhm_state::tutankhm_flip_screen_x_w)
{
	m_flip_x = data & 0x01;
}


WRITE8_MEMBER(tutankhm_state::tutankhm_flip_screen_y_w)
{
	m_flip_y = data & 0x01;
}


/*************************************
 *
 *  Palette management
 *
 *************************************/

void tutankhm_state::get_pens( pen_t *pens )
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		UINT8 data = m_paletteram[i];

		pens[i] = rgb_t(pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}
}


/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 tutankhm_state::screen_update_tutankhm(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int xorx = m_flip_x ? 255 : 0;
	int xory = m_flip_y ? 255 : 0;
	pen_t pens[NUM_PENS];
	int x, y;

	get_pens( pens);

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *dst = &bitmap.pix32(y);

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT8 effx = x ^ xorx;
			UINT8 yscroll = (effx < 192) ? *m_scroll : 0;
			UINT8 effy = (y ^ xory) + yscroll;
			UINT8 vrambyte = m_videoram[effy * 128 + effx / 2];
			UINT8 shifted = vrambyte >> (4 * (effx % 2));
			dst[x] = pens[shifted & 0x0f];
		}
	}

	return 0;
}
