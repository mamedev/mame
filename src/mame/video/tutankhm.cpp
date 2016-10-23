// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/tutankhm.h"


/*************************************
 *
 *  Write handlers
 *
 *************************************/

void tutankhm_state::tutankhm_flip_screen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_flip_x = data & 0x01;
}


void tutankhm_state::tutankhm_flip_screen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_flip_y = data & 0x01;
}


/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t tutankhm_state::screen_update_tutankhm(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int xorx = m_flip_x ? 255 : 0;
	int xory = m_flip_y ? 255 : 0;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *dst = &bitmap.pix32(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint8_t effx = x ^ xorx;
			uint8_t yscroll = (effx < 192) ? *m_scroll : 0;
			uint8_t effy = (y ^ xory) + yscroll;
			uint8_t vrambyte = m_videoram[effy * 128 + effx / 2];
			uint8_t shifted = vrambyte >> (4 * (effx % 2));
			dst[x] = m_palette->pen_color(shifted & 0x0f);
		}
	}

	return 0;
}
