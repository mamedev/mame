// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -=  SunA 8 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)

    These games have only sprites, of a peculiar type:

    there is a region of memory where 4 pages of 32x32 tile codes can
    be written like a tilemap made of 4 pages of 256x256 pixels. Each
    tile uses 2 bytes. Later games may use more pages through RAM
    banking.

    Sprites are rectangular regions of *tiles* fetched from there and
    sent to the screen. Each sprite uses 4 bytes, held within the last
    page of tiles.

    Set TILEMAPS to 1 to debug.
    Press Z (you see the "tilemaps" in RAM) or
    Press X (you see the "tilemaps" in ROM) then

    - use Q&W to cycle through the pages.
    - Use E&R to cycle through the tiles banks.
    - Use A&S to cycle through the ROM banks (only with X pressed, of course).

***************************************************************************/

#include "emu.h"
#include "includes/suna8.h"
#include "drawgfxm.h"

/***************************************************************************
    For Debug: there's no tilemap, just sprites.
***************************************************************************/
#if TILEMAPS
TILE_GET_INFO_MEMBER(suna8_state::get_tile_info)
{
	UINT8 code, attr;

	if (machine().input().code_pressed(KEYCODE_X))
	{
		UINT8 *rom = memregion("maincpu")->base() + 0x10000 + 0x4000 * m_trombank;
		code = rom[ 2 * tile_index + 0 ];
		attr = rom[ 2 * tile_index + 1 ];
	}
	else
	{
		code = m_spriteram[ 2 * tile_index + 0 ];
		attr = m_spriteram[ 2 * tile_index + 1 ];
	}
	SET_TILE_INFO_MEMBER(m_page / 8,
			( (attr & 0x03) << 8 ) + code + m_tiles*0x400,
			(attr >> 2) & 0xf,
			TILE_FLIPYX( (attr >> 6) & 3 ));
}
#endif


READ8_MEMBER( suna8_state::banked_paletteram_r )
{
	offset += m_palettebank * 0x200;
	return m_banked_paletteram[offset];
}

READ8_MEMBER(suna8_state::suna8_banked_spriteram_r)
{
	offset += m_spritebank * 0x2000;
	return m_spriteram[offset];
}

WRITE8_MEMBER(suna8_state::suna8_spriteram_w)
{
	m_spriteram[offset] = data;
#if TILEMAPS
	m_bg_tilemap->mark_tile_dirty(offset/2);
#endif
}

WRITE8_MEMBER(suna8_state::suna8_banked_spriteram_w)
{
	offset += m_spritebank * 0x2000;
	m_spriteram[offset] = data;
#if TILEMAPS
	m_bg_tilemap->mark_tile_dirty(offset/2);
#endif
}

/*
    Banked Palette RAM. The data is scrambled
*/
WRITE8_MEMBER( suna8_state::brickzn_banked_paletteram_w )
{
	if (!m_paletteram_enab)
		return;

	int r,g,b;
	UINT16 rgb;

	offset += m_palettebank * 0x200;
	m_banked_paletteram[offset] = data;
	rgb = (m_banked_paletteram[offset&~1] << 8) + m_banked_paletteram[offset|1];

	if (m_prot2_prev == 0x3c && m_prot2 == 0x80)
	{
		rgb ^= 0x4444;

		r   =   (rgb >> 12) & 0x0f;
		g   =   (rgb >>  8) & 0x0f;
		b   =   (rgb >>  4) & 0x0f;
	}
	// see code at 697b
	else if (m_prot2_prev == 0xa8 && m_prot2 == 0x00)
	{
		r   =   (((rgb & (1<<0xc))?1:0)<<0) |
				(((rgb & (1<<0xb))?1:0)<<1) |
				(((rgb & (1<<0xe))?1:0)<<2) |
				(((rgb & (1<<0xf))?1:0)<<3);
		g   =   (((rgb & (1<<0x8))?1:0)<<0) |
				(((rgb & (1<<0x9))?1:0)<<1) |
				(((rgb & (1<<0xa))?1:0)<<2) |
				(((rgb & (1<<0xd))?1:0)<<3);
		b   =   (((rgb & (1<<0x4))?1:0)<<0) |
				(((rgb & (1<<0x3))?1:0)<<1) |
				(((rgb & (1<<0x6))?1:0)<<2) |
				(((rgb & (1<<0x7))?1:0)<<3);
	}
	else
	{
		r   =   (rgb >> 12) & 0x0f;
		g   =   (rgb >>  8) & 0x0f;
		b   =   (rgb >>  4) & 0x0f;
	}

	m_palette->set_pen_color(offset/2,pal4bit(r),pal4bit(g),pal4bit(b));
}



void suna8_state::suna8_vh_start_common(bool has_text, GFXBANK_TYPE_T gfxbank_type)
{
	m_has_text      =   has_text;
	m_spritebank    =   0;
	m_gfxbank       =   0;
	m_gfxbank_type  =   gfxbank_type;
	m_palettebank   =   0;

	if (!m_has_text)
	{
		m_banked_paletteram.allocate(0x200 * 2);

		m_spriteram.allocate(0x2000 * 2 * 2);   // 2 RAM banks, sparkman has 2 "chips"
		memset(m_spriteram,0,0x2000 * 2 * 2);   // helps debugging
	}

#if TILEMAPS
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(suna8_state::get_tile_info),this), TILEMAP_SCAN_COLS,

								8, 8, 0x20*(m_has_text ? 4 : 16), 0x20);

	m_bg_tilemap->set_transparent_pen(15);
#endif
}

VIDEO_START_MEMBER(suna8_state,suna8_text)              { suna8_vh_start_common( true,  GFXBANK_TYPE_SPARKMAN); }
VIDEO_START_MEMBER(suna8_state,suna8_sparkman)          { suna8_vh_start_common( false, GFXBANK_TYPE_SPARKMAN); }
VIDEO_START_MEMBER(suna8_state,suna8_brickzn)           { suna8_vh_start_common( false, GFXBANK_TYPE_BRICKZN);  }
VIDEO_START_MEMBER(suna8_state,suna8_starfigh)          { suna8_vh_start_common( false, GFXBANK_TYPE_STARFIGH); }

/***************************************************************************


                                Sprites Drawing


***************************************************************************/

#define PIXEL_OP_REBASE_TRANSPEN_PRIORITY_MASK(DEST, PRIORITY, SOURCE)              \
do                                                                                  \
{                                                                                   \
	UINT32 srcdata = (SOURCE);                                                      \
	if (srcdata != trans_pen)                                                       \
	{                                                                               \
		if ((PRIORITY) == 0)                                                        \
			(DEST) = color + srcdata;                                               \
	}                                                                               \
}                                                                                   \
while (0)

class mygfx_element : public gfx_element
{
public:
	void prio_mask_transpen(bitmap_ind16 &dest, const rectangle &cliprect,
			UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
			bitmap_ind8 &priority, UINT32 trans_pen)
	{
		color = colorbase() + granularity() * (color % colors());
		code %= elements();
		DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN_PRIORITY_MASK, UINT8);
	}
};

/***************************************************************************

                          [ Sprites Format ]

    * Note: later games use a more complex format than the following

    Offset:         Bits:               Value:

        0.b                             Y (Bottom up)

        1.b         7--- ----           Sprite Size (1 = 2x32 tiles; 0 = 2x2)

                    2x2 Sprites:
                    -65- ----           Tiles Row (height = 8 tiles)
                    ---4 ----           Page

                    2x32 Sprites:
                    -6-- ----           Ignore X (Multisprite)
                    --54 ----           Page

                    ---- 3210           Tiles Column (width = 2 tiles)

        2.b                             X

        3.b         7--- ----           Text Sprite
                    -6-- ----           X (Sign Bit) <- Also Set For Text Sprites
                    --54 3210           Tiles Bank ($400 tiles each)


                        [ Sprite's Tiles Format ]


    Offset:         Bits:                   Value:

        0.b                             Code (Low Bits)

        1.b         7--- ----           Flip Y
                    -6-- ----           Flip X
                    --54 32--           Color
                    ---- --10           Code (High Bits)

***************************************************************************/

void suna8_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int which)
{
	UINT8 *spriteram = m_spriteram + which * 0x2000 * 2;

	int mx = 0; // multisprite x counter

	int max_x = m_screen->width() - 8;
	int max_y = m_screen->height() - 8;

	if (m_has_text)
		screen.priority().fill(0, cliprect);

	for (int i = start; i < end; i += 4)
	{
		int srcpg, srcx,srcy, dimx,dimy, tx, ty;
		int gfxbank, colorbank = 0, flipx,flipy, multisprite;

		int y       =   spriteram[i + 0];
		int code    =   spriteram[i + 1];
		int x       =   spriteram[i + 2];
		int bank    =   spriteram[i + 3];

		bool read_mask = false;

		if (m_has_text)
		{
			// Older, simpler hardware: hardhead, rranger

			// rranger (20:18):
			//   fireball (fe00: 19 00 28 07) not masked by text (fd48: e0 00 00 c0, fd4c: 10 00 00 c0), fire (fdb0: 30 00 00 c0, fdb4: 20 00 00 c0)
			//   fireball (fe00: 20 00 ba 07) is masked!? But only by fire?
			// rranger (33:04, 35:17):
			//   generally no masking by fire, but there are missing legs on some enemies.
			//   Could be a sprite limit hit, as there are many sprites on that line, and it's affected by the horizontal (dead) player
			read_mask = true;

			if ((bank & 0xc0) == 0xc0)
			{
				// hardhead: fd88/8c/90 -> f994..f9c0
				// rranger:  fd48/4c    -> f980..f9ac
				//           fdb0/b4    -> f9c0..f9fc
				//           note: fireballs in level 5 (fe20/40) should go above flames and score (text sprites)
				int text_list  = (i - start) & 0x20;
				int text_start = text_list ? 0x19c0 : 0x1980;
				bool write_mask = (text_list == 0); // hack?
				draw_text_sprites(screen, bitmap, cliprect, text_start, text_start + 0x80, y, write_mask);
				continue;
			}

			flipx = 0;
			flipy = 0;
			gfxbank = bank & 0x3f;
			switch( code & 0x80 )
			{
			case 0x80:
				dimx = 2;                   dimy =  32;
				srcx  = (code & 0xf) * 2;   srcy = 0;
				srcpg = (code >> 4) & 3;
				break;
			case 0x00:
			default:
				dimx = 2;                   dimy =  2;
				srcx  = (code & 0xf) * 2;   srcy = ((code >> 5) & 0x3) * 8 + 6;
				srcpg = (code >> 4) & 1;
				break;
			}
			multisprite = ((code & 0x80) && (code & 0x40));
		}
		else
		{
			// Newer, more complex hardware: brickzn, hardhea2, sparkman, starfigh
			switch( code & 0xc0 )
			{
			case 0xc0:
				dimx = 4;                   dimy = 32;
				srcx  = (code & 0xe) * 2;   srcy = 0;
				flipx = (code & 0x1);
				flipy = 0;
				gfxbank = bank & 0x1f;
				srcpg = (code >> 4) & 3;
				break;
			case 0x80:
				dimx = 2;                   dimy = 32;
				srcx  = (code & 0xf) * 2;   srcy = 0;
				flipx = 0;
				flipy = 0;
				gfxbank = bank & 0x1f;
				srcpg = (code >> 4) & 3;
				break;
			case 0x40:
				// hardhea2: fire code=52/54 bank=a4; player code=02/04/06 bank=08; arrow:code=16 bank=27
				dimx = 4;                   dimy = 4;
				srcx  = (code & 0xe) * 2;
				flipx = code & 0x01;
				flipy = bank & 0x10;
				srcy  = (((bank & 0x80)>>4) + (bank & 0x04) + ((~bank >> 4)&2)) * 2;
				srcpg = ((code >> 4) & 3) + 4;
				gfxbank = (bank & 0x3);
				switch (m_gfxbank_type)
				{
					case suna8_state::GFXBANK_TYPE_SPARKMAN:
						break;

					case suna8_state::GFXBANK_TYPE_BRICKZN:
						gfxbank += 4;   // brickzn: 06,a6,a2,b2->6
						break;

					case suna8_state::GFXBANK_TYPE_STARFIGH:
						// starfigh: boss 2 head, should be p7 g7 x8/c y4:
						//      67 74 88 03
						//      67 76 ac 03
						// starfigh: boss 2 chainguns should be p6 g7:
						//      a8 68/a/c/e 62 23
						//      48 68/a/c/e 62 23
						// starfigh: player, p4 g0:
						//      64 40 d3 20
						// starfigh: title star, p5 g1 / p7 g0:
						//      70 56/8/a/c 0e 01 (gfxhi=1)
						//      6f 78/a/c/e 0f 04 ""
						if (gfxbank == 3)
							gfxbank += m_gfxbank;
						break;
				}
				colorbank = (bank & 8) >> 3;
				break;
			case 0x00:
			default:
				dimx = 2;                   dimy = 2;
				srcx  = (code & 0xf) * 2;
				flipx = 0;
				flipy = 0;
				srcy  = (((bank & 0x80)>>4) + (bank & 0x04) + ((~bank >> 4)&3)) * 2;
				srcpg = (code >> 4) & 3;
				gfxbank = bank & 0x03;
				switch (m_gfxbank_type)
				{
					case suna8_state::GFXBANK_TYPE_STARFIGH:
						// starfigh: boss 2 tail, p2 g7:
						//      61 20 1b 27
						if (gfxbank == 3)
							gfxbank += m_gfxbank;
					break;

					default:
					break;
				}
				break;
			}
			multisprite = ((code & 0x80) && (bank & 0x80));
		}

		x = x - ((bank & 0x40) ? 0x100 : 0);
		y = (0x100 - y - dimy*8 ) & 0xff;

		// Multi Sprite
		if ( multisprite )  {   mx += dimx*8;   x = mx; }
		else                    mx = x;

		gfxbank *= 0x400;

		for (ty = 0; ty < dimy; ty ++)
		{
			for (tx = 0; tx < dimx; tx ++)
			{
				int addr    =   (srcpg * 0x20 * 0x20) +
								((srcx + (flipx?dimx-tx-1:tx)) & 0x1f) * 0x20 +
								((srcy + (flipy?dimy-ty-1:ty)) & 0x1f);

				int tile    =   spriteram[addr*2 + 0];
				int attr    =   spriteram[addr*2 + 1];

				int tile_flipx  =   attr & 0x40;
				int tile_flipy  =   attr & 0x80;

				int sx      =    x + tx * 8;
				int sy      =   (y + ty * 8) & 0xff;

				if (flipx)  tile_flipx = !tile_flipx;
				if (flipy)  tile_flipy = !tile_flipy;

				if (flip_screen())
				{
					sx = max_x - sx;    tile_flipx = !tile_flipx;
					sy = max_y - sy;    tile_flipy = !tile_flipy;
				}

				int code  = tile + (attr & 0x3)*0x100 + gfxbank;
				int color = (((attr >> 2) & 0xf) ^ colorbank) + 0x10 * m_palettebank;    // player2 in hardhea2 and sparkman

				if (read_mask)
					((mygfx_element*)(m_gfxdecode->gfx(which)))->prio_mask_transpen(bitmap, cliprect,
								code, color, tile_flipx, tile_flipy, sx, sy, screen.priority(), 0xf);
				else
					m_gfxdecode->gfx(which)->transpen(bitmap, cliprect,
								code, color, tile_flipx, tile_flipy, sx, sy, 0xf);
			}
		}
	}
}

/***************************************************************************

                          [ Text Sprites Format ]

    Offset:         Bits:               Value:

        0.b                             Tiles Y (height = 2 tiles)

        1.b         7--- ----           0 = Skip Sprite
                    -6-- ----
                    --54 ----           Page
                    ---- 3210           Tiles Column (width = 2 tiles)

        2.b                             X

        3.b         7--- ----           0 = Last Sprite
                    -6-- ----           X (Sign Bit)
                    --54 3210           Tiles Bank ($400 tiles each)

    Each text sprite is 2x2 tiles

***************************************************************************/

void suna8_state::draw_text_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int ypos, bool write_mask)
{
	UINT8 *spriteram = m_spriteram;

	int max_x = m_screen->width() - 8;
	int max_y = m_screen->height() - 8;

	bool last = false;
	for (int i = start; i < end && !last; i += 4)
	{
		int srcpg, srcx,srcy, dimx,dimy, tx, ty;

		int y       =   spriteram[i + 0];
		int code    =   spriteram[i + 1];
		int x       =   spriteram[i + 2];
		int bank    =   spriteram[i + 3];

		if (~code & 0x80)   continue;
		last = !(bank & 0x80);

		dimx = 2;                   dimy = 2;
		srcx  = (code & 0xf) * 2;   srcy = ((y & 0xf8) - (ypos & 0xf8) - 0x10) / 8;
		srcpg = (code >> 4) & 3;

		x = x - ((bank & 0x40) ? 0x100 : 0);

		bank    =   (bank & 0x3f) * 0x400;

		for (ty = 0; ty < dimy; ty ++)
		{
			for (tx = 0; tx < dimx; tx ++)
			{
				int addr    =   (srcpg * 0x20 * 0x20) +
								((srcx + tx) & 0x1f) * 0x20 +
								((srcy + ty) & 0x1f);

				int tile    =   spriteram[addr*2 + 0];
				int attr    =   spriteram[addr*2 + 1];

				int flipx   =   attr & 0x40;
				int flipy   =   attr & 0x80;

				int sx      =    x + tx * 8;
				int sy      =   (0xf0 - ypos + ty * 8) & 0xff;

				if (flip_screen())
				{
					sx = max_x - sx;    flipx = !flipx;
					sy = max_y - sy;    flipy = !flipy;
				}

				m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
							tile + (attr & 0x3)*0x100 + bank,
							(attr >> 2) & 0xf,
							flipx, flipy,
							sx, sy, 0xf );
			}
		}
	}

	if (write_mask)
	{
		// Fill the text sprites row with high priority for masking
		int sy = (0xf0 - ypos) & 0xff;
		if (flip_screen())
			sy = max_y - sy - 8;
		rectangle text_clip(cliprect.min_x, cliprect.max_x, sy, sy + 0x10 - 1);
		text_clip &= cliprect;
		screen.priority().fill(1, text_clip);
	}
}

/***************************************************************************


                                Screen Drawing


***************************************************************************/

UINT32 suna8_state::screen_update_suna8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// see hardhead, hardhea2 test mode (press button 2 for both players)
	bitmap.fill(0xff, cliprect);

#ifdef MAME_DEBUG
#if TILEMAPS
	if (machine().input().code_pressed(KEYCODE_Z) || machine().input().code_pressed(KEYCODE_X))
	{
		int max_tiles = memregion("gfx1")->bytes() / (0x400 * 0x20);

		if (machine().input().code_pressed_once(KEYCODE_Q)) { m_page--;     machine().tilemap().mark_all_dirty();   }
		if (machine().input().code_pressed_once(KEYCODE_W)) { m_page++;     machine().tilemap().mark_all_dirty();   }
		if (machine().input().code_pressed_once(KEYCODE_E)) { m_tiles--;    machine().tilemap().mark_all_dirty();   }
		if (machine().input().code_pressed_once(KEYCODE_R)) { m_tiles++;    machine().tilemap().mark_all_dirty();   }
		if (machine().input().code_pressed_once(KEYCODE_A)) { m_trombank--; machine().tilemap().mark_all_dirty();   }
		if (machine().input().code_pressed_once(KEYCODE_S)) { m_trombank++; machine().tilemap().mark_all_dirty();   }

		m_trombank  &=  0xf;
		m_page      &=  m_has_text ? 3 : (m_gfxdecode->gfx(1) ? 15 : 7);
		m_tiles     %=  max_tiles;
		if (m_tiles < 0) m_tiles += max_tiles;

		m_bg_tilemap->set_scrollx(0, 0x100 * m_page);
		m_bg_tilemap->set_scrolly(0, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
#if 1
	popmessage("%02X %02X %02X - p%2X g%02X r%02X",
						m_rombank, m_palettebank, m_spritebank,
						m_page, m_tiles, m_trombank);
#endif
		return 0;
	}
#endif
#endif

	// Sprites
	draw_sprites(screen, bitmap, cliprect, 0x1d00, 0x2000, 0);

	// More sprites (second sprite "chip" in sparkman)
	if (m_gfxdecode->gfx(1))
		draw_sprites(screen, bitmap, cliprect, 0x1d00, 0x2000, 1);

	return 0;
}
