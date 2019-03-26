// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*******************************************************************

Namco System 1 Video Hardware

*******************************************************************/

#include "emu.h"
#include "includes/namcos1.h"
#include "screen.h"


/*
  video ram map
  0000-1fff : scroll playfield (0) : 64*64*2
  2000-3fff : scroll playfield (1) : 64*64*2
  4000-5fff : scroll playfield (2) : 64*64*2
  6000-6fff : scroll playfield (3) : 64*32*2
  7000-700f : ?
  7010-77ef : fixed playfield (4)  : 36*28*2
  77f0-77ff : ?
  7800-780f : ?
  7810-7fef : fixed playfield (5)  : 36*28*2
  7ff0-7fff : ?
*/

/*
  paletteram map (s1ram  0x0000-0x7fff)
  0000-17ff : palette page0 : sprite
  2000-37ff : palette page1 : playfield
  4000-57ff : palette page2 : playfield (shadow)
  6000-77ff : palette page3 : work RAM

  x800-x807 : CUS116 registers (visibility window)

  so there is just 3x0x2000 RAM, plus the CUS116 internal registers.
*/

/*
  spriteram map (s1ram 0x10000-0x10fff)
  0000-07ff : work ram
  0800-0fef : sprite ram    : 0x10 * 127
  0ff0-0fff : display control registers

  1000-1fff : playfield control registers
*/


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

void namcos1_state::TilemapCB(u16 code, int *tile, int *mask)
{
	code &= 0x3fff;
	*tile = code;
	*mask = code;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void namcos1_state::video_start()
{
	int i;

	/* set table for sprite color == 0x7f */
	for (i = 0;i < 15;i++)
		m_drawmode_table[i] = DRAWMODE_SHADOW;
	m_drawmode_table[15] = DRAWMODE_NONE;

	/* all palette entries are not affected by shadow sprites... */
	for (i = 0;i < 0x2000;i++)
		m_c116->shadow_table()[i] = i;
	/* ... except for tilemap colors */
	for (i = 0x0800;i < 0x1000;i++)
		m_c116->shadow_table()[i] = i + 0x0800;

	m_copy_sprites = false;

	save_item(NAME(m_copy_sprites));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void namcos1_state::spriteram_w(offs_t offset, u8 data)
{
	/* 0000-07ff work ram */
	/* 0800-0fff sprite ram */
	m_spriteram[offset] = data;

	/* a write to this offset tells the sprite chip to buffer the sprite list */
	if (offset == 0x0ff2)
		m_copy_sprites = true;
}



/***************************************************************************

  Display refresh

***************************************************************************/

/*
sprite format:

0-3  scratchpad RAM
4-9  CPU writes here, hardware copies from here to 10-15
10   xx------  X size (16, 8, 32, 4)
10   --x-----  X flip
10   ---xx---  X offset inside 32x32 tile
10   -----xxx  tile bank
11   xxxxxxxx  tile number
12   xxxxxxx-  color
12   -------x  X position MSB
13   xxxxxxxx  X position
14   xxx-----  priority
14   ---xx---  Y offset inside 32x32 tile
14   -----xx-  Y size (16, 8, 32, 4)
14   -------x  Y flip
15   xxxxxxxx  Y position
*/

void namcos1_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 *spriteram = m_spriteram + 0x800;
	const u8 *source = &spriteram[0x800-0x20];   /* the last is NOT a sprite */
	const u8 *finish = &spriteram[0];
	gfx_element *gfx = m_gfxdecode->gfx(0);

	const int sprite_xoffs = spriteram[0x07f5] + ((spriteram[0x07f4] & 1) << 8);
	const int sprite_yoffs = spriteram[0x07f7];

	while (source >= finish)
	{
		static const int sprite_size[4] = { 16, 8, 32, 4 };
		const u8 attr1 = source[10];
		const u8 attr2 = source[14];
		u32 color = source[12];
		int flipx = (attr1 & 0x20) >> 5;
		int flipy = (attr2 & 0x01);
		const u16 sizex = sprite_size[(attr1 & 0xc0) >> 6];
		const u16 sizey = sprite_size[(attr2 & 0x06) >> 1];
		const u16 tx = (attr1 & 0x18) & (~(sizex - 1));
		const u16 ty = (attr2 & 0x18) & (~(sizey - 1));
		int sx = source[13] + ((color & 0x01) << 8);
		int sy = -source[15] - sizey;
		u32 sprite = source[11];
		const u32 sprite_bank = attr1 & 7;
		const int priority = (source[14] & 0xe0) >> 5;
		const u8 pri_mask = (0xff << (priority + 1)) & 0xff;

		sprite += sprite_bank * 256;
		color = color >> 1;

		sx += sprite_xoffs;
		sy -= sprite_yoffs;

		if (flip_screen())
		{
			sx = -sx - sizex;
			sy = -sy - sizey;
			flipx ^= 1;
			flipy ^= 1;
		}

		sy++;   /* sprites are buffered and delayed by one scanline */

		gfx->set_source_clip(tx, sizex, ty, sizey);
		if (color != 0x7f)
			gfx->prio_transpen(bitmap,cliprect,
					sprite,
					color,
					flipx,flipy,
					sx & 0x1ff,
					((sy + 16) & 0xff) - 16,
					screen.priority(), pri_mask,
					0xf);
		else
			gfx->prio_transtable(bitmap,cliprect,
					sprite,
					color,
					flipx,flipy,
					sx & 0x1ff,
					((sy + 16) & 0xff) - 16,
					screen.priority(), pri_mask,
					m_drawmode_table);

		source -= 0x10;
	}
}



u32 namcos1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	rectangle new_clip = cliprect;

	/* flip screen is embedded in the sprite control registers */
	flip_screen_set(m_spriteram[0x0ff6] & 1);

	/* background color */
	bitmap.fill(m_c116->black_pen(), cliprect);

	/* berabohm uses asymmetrical visibility windows to iris on the character */
	i = m_c116->get_reg(0) - 1;                         // min x
	if (new_clip.min_x < i) new_clip.min_x = i;
	i = m_c116->get_reg(1) - 1 - 1;                     // max x
	if (new_clip.max_x > i) new_clip.max_x = i;
	i = m_c116->get_reg(2) - 0x11;                      // min y
	if (new_clip.min_y < i) new_clip.min_y = i;
	i = m_c116->get_reg(3) - 0x11 - 1;                  // max y
	if (new_clip.max_y > i) new_clip.max_y = i;

	if (new_clip.empty())
		return 0;

	m_c123tmap->init_scroll(flip_screen());

	screen.priority().fill(0, new_clip);

	/* bit 0-2 priority */
	/* bit 3   disable  */
	for (int priority = 0; priority < 8; priority++)
	{
		m_c123tmap->draw(screen, bitmap, new_clip, priority, priority, 0);
	}

	draw_sprites(screen, bitmap, new_clip);
	return 0;
}


WRITE_LINE_MEMBER(namcos1_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		if (m_copy_sprites)
		{
			u8 *spriteram = m_spriteram + 0x800;

			for (int i = 0; i < 0x800; i += 16)
			{
				for (int j = 10; j < 16; j++)
					spriteram[i + j] = spriteram[i + j - 6];
			}

			m_copy_sprites = false;
		}
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		m_audiocpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		m_mcu->set_input_line(HD6301_IRQ_LINE, ASSERT_LINE);
	}
}
