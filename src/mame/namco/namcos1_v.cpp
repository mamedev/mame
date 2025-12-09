// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*******************************************************************

Namco System 1 Video Hardware

*******************************************************************/

#include "emu.h"
#include "namcos1.h"
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

void namcos1_state::TilemapCB(u16 code, int &tile, int &mask)
{
	code &= 0x3fff;
	tile = code;
	mask = code;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void namcos1_state::video_start()
{
	// set table for sprite color == 0x7f
	for (int i = 0; i < 15; i++)
		m_drawmode_table[i] = DRAWMODE_SHADOW;
	m_drawmode_table[15] = DRAWMODE_NONE;

	// all palette entries are not affected by shadow sprites...
	for (int i = 0; i < 0x2000; i++)
		m_c116->shadow_table()[i] = i;
	// ... except for tilemap colors
	for (int i = 0x0800; i < 0x1000; i++)
		m_c116->shadow_table()[i] = i + 0x0800;
}


/***************************************************************************

  Display refresh

***************************************************************************/

std::pair<bool, u8 const *> namcos1_state::sprite_shadow_cb(u8 color)
{
	return std::make_pair(color == 0x7f, m_drawmode_table);
}

u32 namcos1_state::sprite_pri_cb(u8 attr1, u8 attr2)
{
	const int priority = (attr2 & 0xe0) >> 5;
	return (0xff << (priority + 1)) & 0xff;
}

u32 namcos1_state::sprite_bank_cb(u32 code, u32 bank)
{
	return (code & 0xff) + (bank << 8);
}


u32 namcos1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle new_clip = cliprect;

	// background color
	bitmap.fill(m_c116->black_pen(), cliprect);

	// berabohm uses asymmetrical visibility windows to iris on the character
	int i = m_c116->get_reg(0) - 1;                     // min x
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

	// bit 0-2 priority
	// bit 3   disable
	for (int priority = 0; priority < 8; priority++)
	{
		m_c123tmap->draw(screen, bitmap, new_clip, priority, priority, 0);
	}

	m_spritegen->draw_sprites(screen, bitmap, new_clip);
	return 0;
}


void namcos1_state::screen_vblank(int state)
{
	if (state)
	{
		m_spritegen->copy_sprites();
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
	}
	else
	{
		// falling edge
		// splatter 2nd bossfight music fails if audiocpu irq is at the same time as main/sub irq
		m_audiocpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		m_mcu->set_input_line(HD6301_IRQ1_LINE, ASSERT_LINE);
	}
}
