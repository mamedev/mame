// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Nicola Salmoria
#include "emu.h"
#include "volfied.h"

/******************************************************
          INITIALISATION AND CLEAN-UP
******************************************************/

void volfied_state::video_start()
{
	m_video_ram = std::make_unique<uint16_t[]>(0x40000);

	m_video_ctrl = 0;
	m_video_mask = 0;

	save_pointer(NAME(m_video_ram), 0x40000);
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_video_mask));
}


/*******************************************************
          READ AND WRITE HANDLERS
*******************************************************/

uint16_t volfied_state::video_ram_r(offs_t offset)
{
	return m_video_ram[offset];
}

void volfied_state::video_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	mem_mask &= m_video_mask;

	COMBINE_DATA(&m_video_ram[offset]);
}

void volfied_state::video_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_ctrl);
}

uint16_t volfied_state::video_ctrl_r()
{
	/* Could this be some kind of hardware collision detection? If bit 6 is
	   set the game will check for collisions with the large enemy, whereas
	   bit 5 does the same for small enemies. Bit 7 is also used although
	   its purpose is unclear. This register is usually read during a VBI
	   and stored in work RAM for later use. */

	return 0x60;
}

void volfied_state::video_mask_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_mask);
}

void volfied_state::volfied_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl)
{
	sprite_colbank = 0x100 | ((sprite_ctrl & 0x3c) << 2);
	pri_mask = 0; /* sprites over everything */
}


/*******************************************************
                SCREEN REFRESH
*******************************************************/

void volfied_state::refresh_pixel_layer( bitmap_ind16 &bitmap )
{
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

	uint16_t* p = m_video_ram.get();
	int width = m_screen->width();
	int height = m_screen->height();

	if (m_video_ctrl & 1)
		p += 0x20000;

	for (int y = 0; y < height; y++)
	{
		for (int x = 1; x < width + 1; x++) // Hmm, 1 pixel offset is needed to align properly with sprites
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

			bitmap.pix(y, x - 1) = color;
		}

		p += 512;
	}
}

uint32_t volfied_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	refresh_pixel_layer(bitmap);
	m_pc090oj->draw_sprites(screen, bitmap, cliprect);
	return 0;
}
