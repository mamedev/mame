/***************************************************************************

  Video Hardware for some Technos games:
    Double Dragon, Double Dragon bootleg, Double Dragon II and China Gate

  Two Tile layers.
    Background layer is 512x512 , tiles are 16x16
    Top        layer is 256x256 , tiles are 8x8

  Sprites are 16x16, 16x32, 32x16, or 32x32 (attribute bits set dimension)


BG Tile Layout
  0          1
  ---- -xxx  xxxx xxxx  = Tile number
  --xx x---  ---- ----  = Color
  -x-- ----  ---- ----  = X Flip
  x--- ----  ---- ----  = Y Flip


Top Tile layout.
  0          1
  ---- xxxx  xxxx xxxx  = Tile number
  xxxx ----  ---- ----  = Color (China Gate)
  xxx- ----  ---- ----  = Color (Double Dragon)


Sprite layout.
  0          1          2          3          4
  ---- ----  ---- ----  ---- xxxx  xxxx xxxx  ---- ----  = Sprite number
  ---- ----  ---- ----  -xxx ----  ---- ----  ---- ----  = Color
  xxxx xxxx  ---- ----  ---- ----  ---- ----  ---- ----  = Y position
  ---- ----  ---- ---x  ---- ----  ---- ----  ---- ----  = Y MSb position ???
  ---- ----  ---- ----  ---- ----  ---- ----  xxxx xxxx  = X position
  ---- ----  ---- --x-  ---- ----  ---- ----  ---- ----  = X MSb position ???
  ---- ----  ---- -x--  ---- ----  ---- ----  ---- ----  = Y Flip
  ---- ----  ---- x---  ---- ----  ---- ----  ---- ----  = X Flip
  ---- ----  --xx ----  ---- ----  ---- ----  ---- ----  = Sprite Dimension
  ---- ----  x--- ----  ---- ----  ---- ----  ---- ----  = Visible

***************************************************************************/

#include "emu.h"
#include "includes/ddragon.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(ddragon_state::background_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

TILE_GET_INFO_MEMBER(ddragon_state::get_bg_tile_info)
{
	UINT8 attr = m_bgvideoram[2 * tile_index];
	SET_TILE_INFO_MEMBER(
			2,
			m_bgvideoram[2 * tile_index+1] + ((attr & 0x07) << 8),
			(attr >> 3) & 0x07,
			TILE_FLIPYX((attr & 0xc0) >> 6));
}

TILE_GET_INFO_MEMBER(ddragon_state::get_fg_tile_info)
{
	UINT8 attr = m_fgvideoram[2 * tile_index];
	SET_TILE_INFO_MEMBER(
			0,
			m_fgvideoram[2 * tile_index + 1] + ((attr & 0x07) << 8),
			attr >> 5,
			0);
}

TILE_GET_INFO_MEMBER(ddragon_state::get_fg_16color_tile_info)
{
	UINT8 attr = m_fgvideoram[2 * tile_index];
	SET_TILE_INFO_MEMBER(
			0,
			m_fgvideoram[2 * tile_index+1] + ((attr & 0x0f) << 8),
			attr >> 4,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(ddragon_state,ddragon)
{

	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(ddragon_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(ddragon_state::background_scan),this), 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(ddragon_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scrolldx(0, 384 - 256);
	m_bg_tilemap->set_scrolldx(0, 384 - 256);
	m_fg_tilemap->set_scrolldy(-8, -8);
	m_bg_tilemap->set_scrolldy(-8, -8);
}

VIDEO_START_MEMBER(ddragon_state,chinagat)
{

	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(ddragon_state::get_bg_tile_info),this),tilemap_mapper_delegate(FUNC(ddragon_state::background_scan),this), 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(ddragon_state::get_fg_16color_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scrolldy(-8, -8);
	m_bg_tilemap->set_scrolldy(-8, -8);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(ddragon_state::ddragon_bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(ddragon_state::ddragon_fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}


/***************************************************************************

  Display refresh

***************************************************************************/

#define DRAW_SPRITE( order, sx, sy ) drawgfx_transpen( bitmap, \
					cliprect,gfx, \
					(which + order),color,flipx,flipy,sx,sy,0);

static void draw_sprites( running_machine& machine, bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	ddragon_state *state = machine.driver_data<ddragon_state>();
	gfx_element *gfx = machine.gfx[1];

	UINT8 *src;
	int i;

	if (state->m_technos_video_hw == 1)     /* China Gate Sprite RAM */
		src = (UINT8 *) (state->m_spriteram);
	else
		src = (UINT8 *) (&(state->m_spriteram[0x800]));

	for (i = 0; i < (64 * 5); i += 5)
	{
		int attr = src[i + 1];
		if (attr & 0x80)  /* visible */
		{
			int sx = 240 - src[i + 4] + ((attr & 2) << 7);
			int sy = 232 - src[i + 0] + ((attr & 1) << 8);
			int size = (attr & 0x30) >> 4;
			int flipx = (attr & 8);
			int flipy = (attr & 4);
			int dx = -16,dy = -16;

			int which;
			int color;

			if (state->m_technos_video_hw == 2)     /* Double Dragon 2 */
			{
				color = (src[i + 2] >> 5);
				which = src[i + 3] + ((src[i + 2] & 0x1f) << 8);
			}
			else
			{
				if (state->m_technos_video_hw == 1)     /* China Gate */
				{
					if ((sx < -7) && (sx > -16)) sx += 256; /* fix sprite clip */
					if ((sy < -7) && (sy > -16)) sy += 256; /* fix sprite clip */
				}
				color = (src[i + 2] >> 4) & 0x07;
				which = src[i + 3] + ((src[i + 2] & 0x0f) << 8);
			}

			if (state->flip_screen())
			{
				sx = 240 - sx;
				sy = 256 - sy;
				flipx = !flipx;
				flipy = !flipy;
				dx = -dx;
				dy = -dy;
			}

			which &= ~size;

			switch (size)
			{
				case 0: /* normal */
				DRAW_SPRITE(0, sx, sy);
				break;

				case 1: /* double y */
				DRAW_SPRITE(0, sx, sy + dy);
				DRAW_SPRITE(1, sx, sy);
				break;

				case 2: /* double x */
				DRAW_SPRITE(0, sx + dx, sy);
				DRAW_SPRITE(2, sx, sy);
				break;

				case 3:
				DRAW_SPRITE(0, sx + dx, sy + dy);
				DRAW_SPRITE(1, sx + dx, sy);
				DRAW_SPRITE(2, sx, sy + dy);
				DRAW_SPRITE(3, sx, sy);
				break;
			}
		}
	}
}

#undef DRAW_SPRITE


UINT32 ddragon_state::screen_update_ddragon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	int scrollx = (m_scrollx_hi << 8) | *m_scrollx_lo;
	int scrolly = (m_scrolly_hi << 8) | *m_scrolly_lo;

	m_bg_tilemap->set_scrollx(0, scrollx);
	m_bg_tilemap->set_scrolly(0, scrolly);

	m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(machine(), bitmap, cliprect);
	m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
