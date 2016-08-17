// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Nicola Salmoria
#include "emu.h"
#include "includes/volfied.h"

/******************************************************
          INITIALISATION AND CLEAN-UP
******************************************************/

void volfied_state::video_start()
{
	m_video_ram = std::make_unique<UINT16[]>(0x40000);

	m_video_ctrl = 0;
	m_video_mask = 0;

	save_pointer(NAME(m_video_ram.get()), 0x40000);
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_video_mask));
}


/*******************************************************
          READ AND WRITE HANDLERS
*******************************************************/

READ16_MEMBER(volfied_state::volfied_video_ram_r)
{
	return m_video_ram[offset];
}

WRITE16_MEMBER(volfied_state::volfied_video_ram_w)
{
	mem_mask &= m_video_mask;

	COMBINE_DATA(&m_video_ram[offset]);
}

WRITE16_MEMBER(volfied_state::volfied_video_ctrl_w)
{
	COMBINE_DATA(&m_video_ctrl);
}

READ16_MEMBER(volfied_state::volfied_video_ctrl_r)
{
	/* Could this be some kind of hardware collision detection? If bit 6 is
	   set the game will check for collisions with the large enemy, whereas
	   bit 5 does the same for small enemies. Bit 7 is also used although
	   its purpose is unclear. This register is usually read during a VBI
	   and stored in work RAM for later use. */

	return 0x60;
}

WRITE16_MEMBER(volfied_state::volfied_video_mask_w)
{
	COMBINE_DATA(&m_video_mask);
}

WRITE16_MEMBER(volfied_state::volfied_sprite_ctrl_w)
{
	m_pc090oj->set_sprite_ctrl((data & 0x3c) >> 2);
}


/*******************************************************
                SCREEN REFRESH
*******************************************************/

void volfied_state::refresh_pixel_layer( bitmap_ind16 &bitmap )
{
	int x, y;

	/*********************************************************

	VIDEO RAM has 2 screens x 256 rows x 512 columns x 16 bits

	x---------------  select image
	-x--------------  ?             (used for 3-D corners)
	--x-------------  ?             (used for 3-D walls)
	---xxxx---------  image B
	-------xxx------  palette index bits #8 to #A
	----------x-----  ?
	-----------x----  ?
	------------xxxx  image A

	'3d' corners & walls are made using unknown bits for each
	line the player draws.  However, on the pcb they just
	appear as solid black. Perhaps it was prototype code that
	was turned off at some stage.

	*********************************************************/

	UINT16* p = m_video_ram.get();
	int width = m_screen->width();
	int height = m_screen->height();

	if (m_video_ctrl & 1)
		p += 0x20000;

	for (y = 0; y < height; y++)
	{
		for (x = 1; x < width + 1; x++) // Hmm, 1 pixel offset is needed to align properly with sprites
		{
			int color = (p[x] << 2) & 0x700;

			if (p[x] & 0x8000)
			{
				color |= 0x800 | ((p[x] >> 9) & 0xf);

				if (p[x] & 0x2000)
					color &= ~0xf;    /* hack */
			}
			else
				color |= p[x] & 0xf;

			bitmap.pix16(y, x - 1) = color;
		}

		p += 512;
	}
}

UINT32 volfied_state::screen_update_volfied(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	refresh_pixel_layer(bitmap);
	m_pc090oj->draw_sprites(bitmap, cliprect, screen.priority(), 0);
	return 0;
}
