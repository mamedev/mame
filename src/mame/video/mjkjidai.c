// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/mjkjidai.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(mjkjidai_state::get_tile_info)
{
	int attr = m_videoram[tile_index + 0x800];
	int code = m_videoram[tile_index] + ((attr & 0x1f) << 8);
	int color = m_videoram[tile_index + 0x1000];
	SET_TILE_INFO_MEMBER(0,code,color >> 3,0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void mjkjidai_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mjkjidai_state::get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(mjkjidai_state::mjkjidai_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(mjkjidai_state::mjkjidai_ctrl_w)
{
//  logerror("%04x: port c0 = %02x\n",space.device().safe_pc(),data);

	/* bit 0 = NMI enable */
	m_nmi_enable = data & 1;

	/* bit 1 = flip screen */
	flip_screen_set(data & 0x02);

	/* bit 2 =display enable */
	m_display_enable = data & 0x04;

	/* bit 5 = coin counter */
	coin_counter_w(machine(), 0,data & 0x20);

	/* bits 6-7 select ROM bank */
	membank("bank1")->set_entry(data >> 6);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void mjkjidai_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT8 *spriteram = &m_videoram[0];
	UINT8 *spriteram_2 = &m_videoram[0x800];
	UINT8 *spriteram_3 = &m_videoram[0x1000];
	int offs;

	for (offs = 0x20-2;offs >= 0;offs -= 2)
	{
		int code = spriteram[offs] + ((spriteram_2[offs] & 0x1f) << 8);
		int color = (spriteram_3[offs] & 0x78) >> 3;
		int sx = 2*spriteram_2[offs+1];
		int sy = 240 - spriteram[offs+1];
		int flipx = code & 1;
		int flipy = code & 2;

		code >>= 2;

		sx += (spriteram_2[offs] & 0x20) >> 5;  // not sure about this

		if (flip_screen())
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sx += 16;
		sy += 1;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}



UINT32 mjkjidai_state::screen_update_mjkjidai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_display_enable)
		bitmap.fill(m_palette->black_pen(), cliprect);
	else
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
	}
	return 0;
}
