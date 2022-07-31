// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q           shows layer 0
        W           shows layer 1
        A           shows the sprites

        Keys can be used together!


                        [ 0, 1 Or 2 Scrolling Layers ]

    Each layer consists of 2 tilemaps: only one can be displayed at a
    given time (the games usually flip continuously between the two).
    The two tilemaps share the same scrolling registers.

        Layer Size:             1024 x 512
        Tiles:                  16x16x4 (16x16x6 in some games)
        Tile Format:

            Offset + 0x0000:
                            f--- ---- ---- ----     Flip X
                            -e-- ---- ---- ----     Flip Y
                            --dc ba98 7654 3210     Code

            Offset + 0x1000:

                            fedc ba98 765- ----     -
                            ---- ---- ---4 3210     Color

            The other tilemap for this layer (always?) starts at
            Offset + 0x2000.


                            [ 1024 Sprites ]

    Sprites are 16x16x4. They are just like those in "The NewZealand Story",
    "Revenge of DOH" etc (tnzs.cpp). Obviously they're hooked to a 16 bit
    CPU here, so they're mapped a bit differently in memory. Additionally,
    there are two banks of sprites. The game can flip between the two to
    do double buffering, writing to a bit of a control register(see below)


        Spriteram16_2 + 0x000.w

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc ba-- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0x400.w

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     X

        Spriteram16   + 0x000.w

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y



                            [ Floating Tilemap ]

    There's a floating tilemap made of vertical columns composed of 2x16
    "sprites". Each 32 consecutive "sprites" define a column.

    For column I:

        Spriteram16_2 + 0x800 + 0x40 * I:

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc b--- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0xc00 + 0x40 * I:

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     -

    Each column has a variable horizontal position and a vertical scrolling
    value (see also the Sprite Control Registers). For column I:


        Spriteram16   + 0x400 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y

        Spriteram16   + 0x408 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Low Bits Of X



                        [ Sprites Control Registers ]


        Spriteram16   + 0x601.b

                        7--- ----       0
                        -6-- ----       Flip Screen
                        --5- ----       0
                        ---4 ----       1 (Sprite Enable?)
                        ---- 3210       ???

        Spriteram16   + 0x603.b

                        7--- ----       0
                        -6-- ----       Sprite Bank
                        --5- ----       0 = Sprite Buffering (blandia,msgundam,qzkklogy)
                        ---4 ----       0
                        ---- 3210       Columns To Draw (1 is the special value for 16)

        Spriteram16   + 0x605.b

                        7654 3210       High Bit Of X For Columns 7-0

        Spriteram16   + 0x607.b

                        7654 3210       High Bit Of X For Columns f-8




***************************************************************************/

#include "emu.h"
#include "seta.h"
#include "screen.h"

/*      76-- ----
        --5- ----     Sound Enable
        ---4 ----     toggled in IRQ1 by many games, irq acknowledge?
                      [original comment for the above: ?? 1 in oisipuzl, sokonuke (layers related)]
        ---- 3---     Coin #1 Lock Out
        ---- -2--     Coin #0 Lock Out
        ---- --1-     Coin #1 Counter
        ---- ---0     Coin #0 Counter     */

// some games haven't the coin lockout device (blandia, eightfrc, extdwnhl, gundhara, kamenrid, magspeed, sokonuke, zingzip, zombraid)
void seta_state::seta_coin_counter_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	if (m_x1snd.found())
		m_x1snd->enable_w(BIT(data, 6));
}

void seta_state::seta_coin_lockout_w(u8 data)
{
	seta_coin_counter_w(data);

	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 2));
	machine().bookkeeping().coin_lockout_w(1, !BIT(data, 3));
}


void seta_state::seta_vregs_w(u8 data)
{
	m_vregs = data;

	/* Partly handled in vh_screenrefresh:

	        76-- ----
	        --54 3---     Samples Bank (in blandia, eightfrc, zombraid)
	        ---- -2--
	        ---- --1-     Sprites Above Frontmost Layer
	        ---- ---0     Layer 0 Above Layer 1
	*/

	const int new_bank = (data >> 3) & 0x7;

	if (new_bank != m_samples_bank)
	{
		m_samples_bank = new_bank;
		if (m_x1_bank != nullptr)
			m_x1_bank->set_entry(m_samples_bank);
	}
}


/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

Offset + 0x0000:
                    f--- ---- ---- ----     Flip X
                    -e-- ---- ---- ----     Flip Y
                    --dc ba98 7654 3210     Code

Offset + 0x1000:

                    fedc ba98 765- ----     -
                    ---- ---- ---4 3210     Color


                      [ TileMaps Control Registers]

Offset + 0x0:                               Scroll X
Offset + 0x2:                               Scroll Y
Offset + 0x4:
                    fedc ba98 765- ----     -
                    ---- ---- ---4 ----     Tilemap color mode switch (used in blandia and the other games using 6bpp graphics)
                    ---- ---- ---- 3---     Tilemap Select (There Are 2 Tilemaps Per Layer)
                    ---- ---- ---- -21-     0 (1 only in eightfrc, when flip is on!)
                    ---- ---- ---- ---0     ?

***************************************************************************/

void downtown_state::twineagl_tilebank_w(offs_t offset, u8 data)
{
	if (m_twineagl_tilebank[offset] != data)
	{
		m_twineagl_tilebank[offset] = data;
		m_layers[0]->mark_all_dirty();
	}
}

u16 downtown_state::twineagl_tile_offset(u16 code)
{
	if ((code & 0x3e00) == 0x3e00)
		return (code & 0x007f) | ((m_twineagl_tilebank[(code & 0x0180) >> 7] >> 1) << 7);
	else
		return code;
}

u16 usclssic_state::tile_offset(u16 code)
{
	return m_tiles_offset + code;
}

X1_001_SPRITE_GFXBANK_CB_MEMBER(seta_state::setac_gfxbank_callback)
{
	const int bank = (color & 0x06) >> 1;
	code = (code & 0x3fff) + (bank * 0x4000);

	return code;
}

void seta_state::video_start()
{
	m_samples_bank = -1;    // set the samples bank to an out of range value at start-up
	if (m_x1_bank != nullptr)
		m_x1_bank->set_entry(0); // TODO : Unknown init

	m_vregs = 0;
	save_item(NAME(m_vregs));
}


/***************************************************************************


                            Palette Init Functions


***************************************************************************/


/* 2 layers, 6 bit deep.

   The game can select to repeat every 16 colors to fill the 64 colors for the 6bpp gfx
   or to use the first 64 colors of the palette regardless of the color code!
*/
void seta_state::blandia_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			// layer 2-3
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x200 + ((color << 4) | (pen & 0x0f)));
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x200 + pen);

			// layer 0-1
			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x400 + ((color << 4) | (pen & 0x0f)));
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x400 + pen);
		}
	}

	// setup the colortable for the effect palette.
	// what are used for palette from 0x800 to 0xBFF?
	for (int i = 0; i < 0x2200; i++)
		palette.set_pen_indirect(0x2200 + i, 0x600 + (i & 0x1ff));
}


/* layers have 6 bits per pixel, but the color code has a 16 colors granularity,
   even if the low 2 bits are ignored (so there are only 4 different palettes) */
void seta_state::gundhara_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));

			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
	}
}


/* layers have 6 bits per pixel, but the color code has a 16 colors granularity */
void seta_state::jjsquawk_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x400 + (((color << 4) + pen) & 0x1ff)); // used by madshark
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x400 + (((color << 4) + pen) & 0x1ff));

			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x200 + (((color << 4) + pen) & 0x1ff)); // used by madshark
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x200 + (((color << 4) + pen) & 0x1ff));
		}
	}
}


// layer 0 is 6 bit per pixel, but the color code has a 16 colors granularity
void seta_state::zingzip_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x400 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0xc00 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
	}
}

// color prom
void seta_state::palette_init_RRRRRGGGGGBBBBB_proms(palette_device &palette) const
{
	const u8 *const color_prom = memregion("proms")->base();
	for (int x = 0; x < 0x200 ; x++)
	{
		const int data = (color_prom[x*2] << 8) | color_prom[x*2 + 1];
		palette.set_pen_color(x, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
	}
}

void setaroul_state::setaroul_palette(palette_device &palette) const
{
	m_spritegen->gfx(0)->set_granularity(16);
	m_layers[0]->gfx(0)->set_granularity(16);

	palette_init_RRRRRGGGGGBBBBB_proms(palette);
}

void usclssic_state::usclssic_palette(palette_device &palette) const
{
	const u8 *const color_prom = memregion("proms")->base();

	// decode PROM
	for (int x = 0; x < 0x200; x++)
	{
		const u16 data = (color_prom[x*2] << 8) | color_prom[x*2 + 1];
		const rgb_t color(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (x >= 0x100)
			palette.set_indirect_color(x + 0x000, color);
		else
			palette.set_indirect_color(x + 0x300, color);
	}

	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x200 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0xa00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
	}
}


void seta_state::set_pens()
{
	for (int i = 0; i < m_paletteram[0].bytes() / 2; i++)
	{
		const u16 data = m_paletteram[0][i];

		rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (m_palette->indirect_entries() != 0)
			m_palette->set_indirect_color(i, color);
		else
			m_palette->set_pen_color(i, color);
	}

	if (m_paletteram[1] != nullptr)
	{
		for (int i = 0; i < m_paletteram[1].bytes() / 2; i++)
		{
			const u16 data = m_paletteram[1][i];

			rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

			if (m_palette->indirect_entries() != 0)
				m_palette->set_indirect_color(i + m_paletteram[0].bytes() / 2, color);
			else
				m_palette->set_pen_color(i + m_paletteram[0].bytes() / 2, color);
		}
	}
}


void usclssic_state::usclssic_set_pens()
{
	for (int i = 0; i < 0x200; i++)
	{
		const u16 data = m_paletteram[0][i];

		rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (i >= 0x100)
			m_palette->set_indirect_color(i - 0x100, color);
		else
			m_palette->set_indirect_color(i + 0x200, color);
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/


/* For games without tilemaps */
u32 seta_state::screen_update_seta_no_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	bitmap.fill(0x1f0, cliprect);

	m_spritegen->draw_sprites(screen, bitmap,cliprect,0x1000);
	return 0;
}


/* For games with 1 or 2 tilemaps */
void seta_state::seta_layers_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_bank_size)
{
	const rectangle &visarea = screen.visible_area();
	const int vis_dimy = visarea.max_y - visarea.min_y + 1;

	const int flip = m_spritegen->is_flipped() ^ m_tilemaps_flip;
	for (int layer = 0; layer < 2; layer++)
	{
		if (m_layers[layer].found())
		{
			m_layers[layer]->set_flip(flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

			/* the hardware wants different scroll values when flipped */

			/*  bg x scroll      flip
			    metafox     0000 025d = 0, $400-$1a3 = $400 - $190 - $13
			    eightfrc    ffe8 0272
			                fff0 0260 = -$10, $400-$190 -$10
			                ffe8 0272 = -$18, $400-$190 -$18 + $1a      */

			m_layers[layer]->update_scroll(vis_dimy, flip);
		}
	}

	unsigned layers_ctrl = ~0U;
#ifdef MAME_DEBUG
	if (screen.machine().input().code_pressed(KEYCODE_Z))
	{   int msk = 0;
		if (screen.machine().input().code_pressed(KEYCODE_Q))   msk |= 1;
		if (screen.machine().input().code_pressed(KEYCODE_W))   msk |= 2;
		if (screen.machine().input().code_pressed(KEYCODE_A))   msk |= 8;
		if (msk != 0) layers_ctrl &= msk;

		if (m_layers[1].found())
			popmessage("VR:%02X L0:%04X L1:%04X",
				m_vregs, m_layers[0]->vctrl(2), m_layers[1]->vctrl(2));
		else if (m_layers[0].found())
			popmessage("L0:%04X", m_layers[0]->vctrl(2));
	}
#endif

	bitmap.fill(0, cliprect);

	const int order = m_layers[1].found() ? m_vregs : 0;
	if (order & 1)  // swap the layers?
	{
		if (m_layers[1].found())
		{
			if (layers_ctrl & 2) m_layers[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		}

		if (order & 2)  // layer-sprite priority?
		{
			if (layers_ctrl & 8) m_spritegen->draw_sprites(screen, bitmap,cliprect,sprite_bank_size);

			if (order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1) m_layers[0]->draw(screen, bitmap, cliprect, 0, 0);
		}
		else
		{
			if (order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1) m_layers[0]->draw(screen, bitmap, cliprect, 0, 0);

			if (layers_ctrl & 8) m_spritegen->draw_sprites(screen, bitmap,cliprect,sprite_bank_size);
		}
	}
	else
	{
		if (layers_ctrl & 1) m_layers[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

		if (order & 2)  // layer-sprite priority?
		{
			if (layers_ctrl & 8) m_spritegen->draw_sprites(screen, bitmap,cliprect,sprite_bank_size);

			if ((order & 4) && m_paletteram[1] != nullptr)
			{
				m_layers[1]->draw_tilemap_palette_effect(bitmap, cliprect, flip);
			}
			else
			{
				if (order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (m_layers[1].found())
				{
					if (layers_ctrl & 2) m_layers[1]->draw(screen, bitmap, cliprect, 0, 0);
				}
			}
		}
		else
		{
			if ((order & 4) && m_paletteram[1] != nullptr)
			{
				m_layers[1]->draw_tilemap_palette_effect(bitmap, cliprect, flip);
			}
			else
			{
				if (order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (m_layers[1].found())
				{
					if (layers_ctrl & 2) m_layers[1]->draw(screen, bitmap, cliprect, 0, 0);
				}
			}

			if (layers_ctrl & 8) m_spritegen->draw_sprites(screen,bitmap,cliprect,sprite_bank_size);
		}
	}

}

u32 seta_state::screen_update_seta_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	seta_layers_update(screen, bitmap, cliprect, 0x1000);
	return 0;
}


u32 setaroul_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x0, cliprect);

	if (m_led & 0x80)
		seta_layers_update(screen, bitmap, cliprect, 0x800);

	return 0;
}

WRITE_LINE_MEMBER(setaroul_state::screen_vblank)
{
	// rising edge
	if (state)
		m_spritegen->tnzs_eof();
}


u32 seta_state::screen_update_seta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	return screen_update_seta_layers(screen, bitmap, cliprect);
}


u32 usclssic_state::screen_update_usclssic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	usclssic_set_pens();
	return screen_update_seta_layers(screen, bitmap, cliprect);
}
