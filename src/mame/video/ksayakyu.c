#include "emu.h"
#include "includes/ksayakyu.h"


WRITE8_MEMBER(ksayakyu_state::ksayakyu_videoram_w)
{
	m_videoram[offset]=data;
	m_textmap->mark_tile_dirty(offset >> 1);
}

WRITE8_MEMBER(ksayakyu_state::ksayakyu_videoctrl_w)
{
	/*
        bits:
        76543210
              xx - ?? layers enable ?
             x   - screen flip
           xx    - ??
        xxx      - scroll offset

     */
	m_video_ctrl = data;

	m_flipscreen = data & 4;
	flip_screen_set(machine(), m_flipscreen);
	m_tilemap->set_scrolly(0, (data & 0xe0) << 3);
	if(m_flipscreen)
		m_tilemap->set_flip((data & 2) ? TILEMAP_FLIPY : TILEMAP_FLIPX | TILEMAP_FLIPY);
	else
		m_tilemap->set_flip((data & 2) ? TILEMAP_FLIPX : 0);
}

PALETTE_INIT( ksayakyu )
{
	const UINT8 *prom = machine.region("proms")->base();
	int r, g, b, i;

	for (i = 0; i < 0x100; i++)
	{
		r = (prom[i] & 0x07) >> 0;
		g = (prom[i] & 0x38) >> 3;
		b = (prom[i] & 0xc0) >> 6;

		palette_set_color_rgb(machine, i, pal3bit(r), pal3bit(g), pal2bit(b));
	}
}

static TILE_GET_INFO( get_ksayakyu_tile_info )
{
	int code = machine.region("user1")->base()[tile_index];
	int attr = machine.region("user1")->base()[tile_index + 0x2000];
	code += (attr & 3) << 8;
	SET_TILE_INFO(1, code, ((attr >> 2) & 0x0f) * 2, (attr & 0x80) ? TILE_FLIPX : 0);
}

/*
xy-- ---- flip bits
--cc cc-- color
---- --bb bank select
*/
static TILE_GET_INFO( get_text_tile_info )
{
	ksayakyu_state *state = machine.driver_data<ksayakyu_state>();
	int code = state->m_videoram[tile_index * 2 + 1];
	int attr = state->m_videoram[tile_index * 2];
	int flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x40) ? TILE_FLIPY : 0);
	int color = (attr & 0x3c) >> 2;

	code |= (attr & 3) << 8;

	SET_TILE_INFO(0, code, color, flags);
}

/*
[0] X--- ---- flip x
    -ttt tttt tile number
[1] yyyy yyyy Y offset
[2] xxxx xxxx X offset
[3]
*/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	ksayakyu_state *state = machine.driver_data<ksayakyu_state>();
	const UINT8 *source = state->m_spriteram + state->m_spriteram_size - 4;
	const UINT8 *finish = state->m_spriteram;

	while (source>=finish) /* is order correct ? */
	{
		int sx = source[2];
		int sy = 240 - source[1];
		int attributes = source[3];
		int tile = source[0];
		int flipx = (tile & 0x80) ? 1 : 0;
		int flipy = 0;

		gfx_element *gfx = machine.gfx[2];

		if (state->m_flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx ^= 1;
			flipy ^= 1;
		}

			drawgfx_transpen(bitmap,cliprect,gfx,
				tile & 0x7f,
				(attributes & 0x78) >> 3,
				flipx,flipy,
				sx,sy,0 );

		source -= 4;
	}
}

VIDEO_START(ksayakyu)
{
	ksayakyu_state *state = machine.driver_data<ksayakyu_state>();
	state->m_tilemap = tilemap_create(machine, get_ksayakyu_tile_info, tilemap_scan_rows, 8, 8, 32, 32 * 8);
	state->m_textmap = tilemap_create(machine, get_text_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_textmap->set_transparent_pen(0);
}

SCREEN_UPDATE_IND16(ksayakyu)
{
	ksayakyu_state *state = screen.machine().driver_data<ksayakyu_state>();

	bitmap.fill(0, cliprect);

	if (state->m_video_ctrl & 1)
		state->m_tilemap->draw(bitmap, cliprect, 0, 0);

	state->m_textmap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
