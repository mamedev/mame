// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/toki.h"


/*************************************************************************
                    RASTER EFFECTS

Xscroll can be altered per scanline to create rowscroll effect.

(The driver does not implement rowscroll on the bootleg. It seems unlikely
the bootleggers would have been able to do this on their chipset. They
remapped the scroll registers, so obviously they were using a different
chip.

Why then would the old ghost of one of the remapped registers
still cause rowscroll? Probably the bootleggers simply didn't bother to
remove all the code writing the $a0000 area.)

*************************************************************************/

WRITE16_MEMBER(toki_state::toki_control_w)
{
	m_screen->update_partial(m_screen->vpos() - 1);
	COMBINE_DATA(&m_scrollram[offset]);
}

TILE_GET_INFO_MEMBER(toki_state::get_text_tile_info)
{
	UINT16 *videoram = m_videoram;
	int tile = videoram[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(toki_state::get_back_tile_info)
{
	int tile = m_background1_videoram[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	SET_TILE_INFO_MEMBER(2,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(toki_state::get_fore_tile_info)
{
	int tile = m_background2_videoram[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	SET_TILE_INFO_MEMBER(3,
			tile,
			color,
			0);
}


/*************************************
 *
 *      Start/Stop
 *
 *************************************/

void toki_state::video_start()
{
	m_text_layer       = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(toki_state::get_text_tile_info),this),TILEMAP_SCAN_ROWS,  8,8,32,32);
	m_background_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(toki_state::get_back_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_foreground_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(toki_state::get_fore_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);

	m_text_layer->set_transparent_pen(15);
	m_background_layer->set_transparent_pen(15);
	m_foreground_layer->set_transparent_pen(15);
}

/*************************************/

WRITE16_MEMBER(toki_state::foreground_videoram_w)
{
	UINT16 *videoram = m_videoram;
	COMBINE_DATA(&videoram[offset]);
	m_text_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(toki_state::background1_videoram_w)
{
	COMBINE_DATA(&m_background1_videoram[offset]);
	m_background_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(toki_state::background2_videoram_w)
{
	COMBINE_DATA(&m_background2_videoram[offset]);
	m_foreground_layer->mark_tile_dirty(offset);
}

/***************************************************************************
                    SPRITES

    Original Spriteram
    ------------------

    It's not clear what purpose is served by marking tiles as being part of
    big sprites. (Big sprites in the attract abduction scene have all tiles
    marked as "first" unlike big sprites in-game.)

    We just ignore this top nibble (although perhaps in theory the bits
    enable X/Y offsets in the low byte).

    +0   x....... ........  sprite disable ??
      +0   .xx..... ........  tile is part of big sprite (4=first, 6=middle, 2=last)
    +0   .....x.. ........  ??? always set? (could be priority - see Bloodbro)
    +0   .......x ........  Flip x
    +0   ........ xxxx....  X offset: add (this * 16) to X coord
    +0   ........ ....xxxx  Y offset: add (this * 16) to Y coord

    +1   xxxx.... ........  Color bank
    +1   ....xxxx xxxxxxxx  Tile number (lo bits)
    +2   x....... ........  Tile number (hi bit)
    +2   .???.... ........  (set in not yet used entries)
    +2   .......x xxxxxxxx  X coordinate
    +3   .......x xxxxxxxx  Y coordinate

    f000 0000 f000 0000     entry not yet used: unless this is honored there
                            will be junk sprites in top left corner
    ffff ???? ???? ????     sprite marked as dead: unless this is honored
                            there will be junk sprites after floating monkey machine


    Bootleg Spriteram
    -----------------

    +0   .......x xxxxxxxx  Sprite Y coordinate
    +1   ...xxxxx xxxxxxxx  Sprite tile number
    +1   .x...... ........  Sprite flip x
    +2   xxxx.... ........  Sprite color bank
    +3   .......x xxxxxxxx  Sprite X coordinate

    f100 ???? ???? ????     dead / unused sprite ??
    ???? ???? 0000 ????     dead / unused sprite ??


***************************************************************************/


void toki_state::toki_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int x,y,xoffs,yoffs,tile,flipx,flipy,color,offs;
	UINT16 *sprite_word;

	for (offs = (m_spriteram->bytes()/2)-4;offs >= 0;offs -= 4)
	{
		sprite_word = &m_spriteram->buffer()[offs];

		if ((sprite_word[2] != 0xf000) && (sprite_word[0] != 0xffff))
		{
			xoffs = (sprite_word[0] &0xf0);
			x = (sprite_word[2] + xoffs) & 0x1ff;
			if (x > 256)
				x -= 512;

			yoffs = (sprite_word[0] &0xf) << 4;
			y = (sprite_word[3] + yoffs) & 0x1ff;
			if (y > 256)
				y -= 512;

			color = sprite_word[1] >> 12;
			flipx   = sprite_word[0] & 0x100;
			flipy   = 0;
			tile    = (sprite_word[1] & 0xfff) + ((sprite_word[2] & 0x8000) >> 3);

			if (flip_screen()) {
				x=240-x;
				y=240-y;
				if (flipx) flipx=0; else flipx=1;
				flipy=1;
			}

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					tile,
					color,
					flipx,flipy,
					x,y,15);
		}
	}
}


void toki_state::tokib_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int x,y,tile,flipx,color,offs;
	UINT16 *sprite_word;

	for (offs = 0;offs < m_spriteram->bytes() / 2;offs += 4)
	{
		sprite_word = &m_spriteram->buffer()[offs];

		if (sprite_word[0] == 0xf100)
			break;
		if (sprite_word[2])
		{
			x = sprite_word[3] & 0x1ff;
			if (x > 256)
				x -= 512;

			y = sprite_word[0] & 0x1ff;
			if (y > 256)
				y = (512-y)+240;
			else
				y = 240-y;

			flipx   = sprite_word[1] & 0x4000;
			tile    = sprite_word[1] & 0x1fff;
			color   = sprite_word[2] >> 12;

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					tile,
					color,
					flipx,0,
					x,y-1,15);
		}
	}
}

/*************************************
 *
 *      Master update function
 *
 *************************************/

UINT32 toki_state::screen_update_toki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int background_y_scroll,foreground_y_scroll,background_x_scroll,foreground_x_scroll;

	background_x_scroll=((m_scrollram[0x06] &0x7f) << 1)
									|((m_scrollram[0x06] &0x80) >> 7)
									|((m_scrollram[0x05] &0x10) << 4);
	background_y_scroll=((m_scrollram[0x0d]&0x10)<<4)+((m_scrollram[0x0e]&0x7f)<<1)+((m_scrollram[0x0e]&0x80)>>7);

	m_background_layer->set_scrollx(0, background_x_scroll );
	m_background_layer->set_scrolly(0, background_y_scroll );

	foreground_x_scroll= ((m_scrollram[0x16] &0x7f) << 1)
									|((m_scrollram[0x16] &0x80) >> 7)
									|((m_scrollram[0x15] &0x10) << 4);
	foreground_y_scroll=((m_scrollram[0x1d]&0x10)<<4)+((m_scrollram[0x1e]&0x7f)<<1)+((m_scrollram[0x1e]&0x80)>>7);

	m_foreground_layer->set_scrollx(0, foreground_x_scroll );
	m_foreground_layer->set_scrolly(0, foreground_y_scroll );

	flip_screen_set((m_scrollram[0x28]&0x8000)==0);

	if (m_scrollram[0x28]&0x100) {
		m_background_layer->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		m_foreground_layer->draw(screen, bitmap, cliprect, 0,0);
	} else {
		m_foreground_layer->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		m_background_layer->draw(screen, bitmap, cliprect, 0,0);
	}
	toki_draw_sprites(bitmap,cliprect);
	m_text_layer->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

UINT32 toki_state::screen_update_tokib(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_foreground_layer->set_scroll_rows(1);
	m_background_layer->set_scroll_rows(1);
	m_background_layer->set_scrolly(0, m_scrollram[0]+1 );
	m_background_layer->set_scrollx(0, m_scrollram[1]-0x103 );
	m_foreground_layer->set_scrolly(0, m_scrollram[2]+1 );
	m_foreground_layer->set_scrollx(0, m_scrollram[3]-0x101 );

	if (m_scrollram[3]&0x2000) {
		m_background_layer->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		m_foreground_layer->draw(screen, bitmap, cliprect, 0,0);
	} else {
		m_foreground_layer->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		m_background_layer->draw(screen, bitmap, cliprect, 0,0);
	}

	tokib_draw_sprites(bitmap,cliprect);
	m_text_layer->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
