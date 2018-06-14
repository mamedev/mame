// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

  vigilant.c

  Functions to emulate the video hardware of the machine.

  TODO:

  - Add cocktail flipping.

***************************************************************************/

#include "emu.h"
#include "includes/vigilant.h"


static const rectangle bottomvisiblearea(16*8, 48*8-1, 6*8, 32*8-1);


void vigilant_state::video_start()
{
	m_bg_bitmap = std::make_unique<bitmap_ind16>(512*4,256);

	save_item(NAME(m_horiz_scroll));
	save_item(NAME(m_rear_horiz_scroll));
	save_item(NAME(m_rear_color));
	save_item(NAME(m_rear_disable));

	m_rear_refresh = 1;
	machine().save().register_postload(save_prepost_delegate(FUNC(vigilant_state::vigilant_postload), this));
}

void vigilant_state::vigilant_postload()
{
	m_rear_refresh = 1;
}

void vigilant_state::video_reset()
{
	m_horiz_scroll = 0;
	m_rear_horiz_scroll = 0;
	m_rear_color = 0;
	m_rear_disable = 1;
}


/***************************************************************************
 update_background

 There are three background ROMs, each one contains a 512x256 picture.
 **************************************************************************/
void vigilant_state::update_background()
{
	int row,col,page;
	int charcode;


	charcode=0;

	/* There are only three background ROMs (4 on bunccaneers!) */
	for (page=0; page<4; page++)
	{
		for( row=0; row<256; row++ )
		{
			for( col=0; col<512; col+=32 )
			{
				m_gfxdecode->gfx(2)->opaque(*m_bg_bitmap,
						m_bg_bitmap->cliprect(),
						charcode,
						row < 128 ? 0 : 1,
						0,0,
						512*page + col,row);
				charcode++;
			}
		}
	}
}

/***************************************************************************
 paletteram_w

 There are two palette chips, each one is labelled "KNA91H014".  One is
 used for the sprites, one is used for the two background layers.

 The chip has three enables (!CS, !E0, !E1), R/W pins, A0-A7 as input,
 'L' and 'H' inputs, and D0-D4 as input.  'L' and 'H' are used to bank
 into Red, Green, and Blue memory.  There are only 5 bits of memory for
 each byte, and a total of 256*3 bytes memory per chip.

 There are additionally two sets of D0-D7 inputs per chip labelled 'A'
 and 'B'.  There is also an 'S' pin to select between the two input sets.
 These are used to index a color triplet of RGB.  The triplet is read
 from RAM, and output to R0-R4, G0-G4, and B0-B4.
 **************************************************************************/
WRITE8_MEMBER(vigilant_state::paletteram_w)
{
	int bank,r,g,b;


	m_paletteram[offset] = data;

	bank = offset & 0x400;
	offset &= 0xff;

	r = (m_paletteram[bank + offset + 0x000] << 3) & 0xFF;
	g = (m_paletteram[bank + offset + 0x100] << 3) & 0xFF;
	b = (m_paletteram[bank + offset + 0x200] << 3) & 0xFF;

	m_palette->set_pen_color((bank >> 2) + offset,rgb_t(r,g,b));
}



/***************************************************************************
 vigilant_horiz_scroll_w

 m_horiz_scroll = 9-bit register, Low 8 bit is HSPL and High 1 bit HSPH
 **************************************************************************/
WRITE8_MEMBER(vigilant_state::vigilant_horiz_scroll_w)
{
	if (offset==0)
		m_horiz_scroll = (m_horiz_scroll & 0x100) | data;
	else
		m_horiz_scroll = (m_horiz_scroll & 0x0ff) | ((data & 0x01) << 8);
}

/***************************************************************************
 vigilant_rear_horiz_scroll_w

 m_rear_horiz_scroll  = 16-bit register but only 11 bits are saved
 Low 8 bit is RHSPL and High 8 bit RHSPH
***************************************************************************/
WRITE8_MEMBER(vigilant_state::vigilant_rear_horiz_scroll_w)
{
	if (offset==0)
		m_rear_horiz_scroll = (m_rear_horiz_scroll & 0x700) | data;
	else
		m_rear_horiz_scroll = (m_rear_horiz_scroll & 0x0ff) | ((data & 0x07) << 8);
}

/***************************************************************************
 vigilant_rear_color_w

 This is an 8-bit register labelled RCOD.
 D6 is hooked to !ROME (rear_disable)
 D3 = RCC2 (rear color bit 2)
 D2 = RCC1 (rear color bit 1)
 D0 = RCC0 (rear color bit 0)

 I know it looks odd, but D1, D4, D5, and D7 are empty.

 What makes this extremely odd is that RCC is supposed to hook up to the
 palette.  However, the top four bits of the palette inputs are labelled:
 "RCC3", "RCC2", "V256E", "RCC0".  Methinks there's a typo.
 **************************************************************************/
WRITE8_MEMBER(vigilant_state::vigilant_rear_color_w)
{
	m_rear_disable = data & 0x40;
	m_rear_color = (data & 0x0d);
}

/***************************************************************************
 draw_foreground

 ???
 **************************************************************************/

void vigilant_state::draw_foreground(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int opaque )
{
	int scroll = -m_horiz_scroll;


	for (int offs = 0; offs < 0x1000; offs += 2)
	{
		int sy = ((offs/2) >> 6) << 3;
		int sx = ((offs/2) & 0x3f) << 3;
		int attributes = m_videoram[offs+1];
		int color = attributes & 0x0F;
		int tile_number = m_videoram[offs] | ((attributes & 0xF0) << 4);

		if (priority)    /* foreground */
		{
			if ((color & 0x0c) == 0x0c) /* mask sprites */
			{
				if (sy >= 48)
				{
					sx = (sx + scroll) & 0x1ff;

					m_gfxdecode->gfx(0)->transmask(bitmap,bottomvisiblearea,
							tile_number,
							color,
							0,0,
							sx,sy,0x00ff);
				}
			}
		}
		else     /* background */
		{
			if (sy >= 48)
				sx = (sx + scroll) & 0x1ff;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					tile_number,
					color,
					0,0,
					sx,sy,
					(opaque || color >= 4) ? -1 : 0);
		}
	}
}



void vigilant_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int scrollx = 0x17a + 16*8 - m_rear_horiz_scroll;


	if (m_rear_refresh)
	{
		update_background();
		m_rear_refresh=0;
	}

	copyscrollbitmap(bitmap,*m_bg_bitmap,1,&scrollx,0,nullptr,bottomvisiblearea);
}


void vigilant_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = 0;offs < m_spriteram.bytes();offs += 8)
	{
		int code,color,sx,sy,flipx,flipy,h,y;

		code = m_spriteram[offs+4] | ((m_spriteram[offs+5] & 0x0f) << 8);
		color = m_spriteram[offs+0] & 0x0f;
		sx = (m_spriteram[offs+6] | ((m_spriteram[offs+7] & 0x01) << 8));
		sy = 256+128 - (m_spriteram[offs+2] | ((m_spriteram[offs+3] & 0x01) << 8));
		flipx = m_spriteram[offs+5] & 0x40;
		flipy = m_spriteram[offs+5] & 0x80;
		h = 1 << ((m_spriteram[offs+5] & 0x30) >> 4);
		sy -= 16 * h;

		code &= ~(h - 1);

		for (y = 0;y < h;y++)
		{
			int c = code;

			if (flipy) c += h-1-y;
			else c += y;

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					c,
					color,
					flipx,flipy,
					sx,sy + 16*y,0);
		}
	}
}

uint32_t vigilant_state::screen_update_kikcubic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x1000; offs += 2)
	{
		int sy = ((offs/2) >> 6) << 3;
		int sx = ((offs/2) & 0x3f) << 3;
		int attributes = m_videoram[offs+1];
		int color = (attributes & 0xF0) >> 4;
		int tile_number = m_videoram[offs] | ((attributes & 0x0F) << 8);

		m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
				tile_number,
				color,
				0,0,
				sx,sy);
	}

	draw_sprites(bitmap,cliprect);
	return 0;
}

uint32_t vigilant_state::screen_update_vigilant(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* copy the background palette */
	for (int i = 0;i < 16;i++)
	{
		int r,g,b;


		r = (m_paletteram[0x400 + 16 * m_rear_color + i] << 3) & 0xFF;
		g = (m_paletteram[0x500 + 16 * m_rear_color + i] << 3) & 0xFF;
		b = (m_paletteram[0x600 + 16 * m_rear_color + i] << 3) & 0xFF;

		m_palette->set_pen_color(512 + i,rgb_t(r,g,b));

		r = (m_paletteram[0x400 + 16 * m_rear_color + 32 + i] << 3) & 0xFF;
		g = (m_paletteram[0x500 + 16 * m_rear_color + 32 + i] << 3) & 0xFF;
		b = (m_paletteram[0x600 + 16 * m_rear_color + 32 + i] << 3) & 0xFF;

		m_palette->set_pen_color(512 + 16 + i,rgb_t(r,g,b));
	}

	if (m_rear_disable)  /* opaque foreground */
	{
		draw_foreground(bitmap,cliprect,0,1);
		draw_sprites(bitmap,bottomvisiblearea);
		draw_foreground(bitmap,cliprect,1,0);
	}
	else
	{
		draw_background(bitmap,cliprect);
		draw_foreground(bitmap,cliprect,0,0);
		draw_sprites(bitmap,bottomvisiblearea);
		draw_foreground(bitmap,cliprect,1,0); // priority tiles
	}
	return 0;
}
