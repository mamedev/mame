/***************************************************************************

                            -= American Speedway =-

                    driver by   Luca Elia (l.elia@tin.it)


- 8x8 4 Color Tiles (with 8 palettes) used for both:

    - 1 256x256 non scrolling layer
    - 64 (32?) Sprites

***************************************************************************/

#include "emu.h"
#include "includes/amspdwy.h"


WRITE8_MEMBER(amspdwy_state::amspdwy_paletteram_w)
{
	data ^= 0xff;
	paletteram_BBGGGRRR_w(space, offset, data);
//  paletteram_RRRGGGBB_w(offset, data);
}

WRITE8_MEMBER(amspdwy_state::amspdwy_flipscreen_w)
{
	m_flipscreen ^= 1;
	flip_screen_set(machine(), m_flipscreen);
}

/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

    Videoram:   76543210    Code Low Bits
    Colorram:   765-----
                ---43---    Code High Bits
                -----210    Color

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	amspdwy_state *state = machine.driver_data<amspdwy_state>();
	UINT8 code = state->m_videoram[tile_index];
	UINT8 color = state->m_colorram[tile_index];
	SET_TILE_INFO(
			0,
			code + ((color & 0x18)<<5),
			color & 0x07,
			0);
}

WRITE8_MEMBER(amspdwy_state::amspdwy_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(amspdwy_state::amspdwy_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


/* logical (col,row) -> memory offset */
static TILEMAP_MAPPER( tilemap_scan_cols_back )
{
	return col * num_rows + (num_rows - row - 1);
}


VIDEO_START( amspdwy )
{
	amspdwy_state *state = machine.driver_data<amspdwy_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_cols_back, 8, 8, 0x20, 0x20);
}



/***************************************************************************

                                Sprites Drawing

Offset:     Format:     Value:

0                       Y
1                       X
2                       Code Low Bits
3           7-------    Flip X
            -6------    Flip Y
            --5-----
            ---4----    ?
            ----3---    Code High Bit?
            -----210    Color

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	amspdwy_state *state = machine.driver_data<amspdwy_state>();
	UINT8 *spriteram = state->m_spriteram;
	int i;
	int max_x = machine.primary_screen->width()  - 1;
	int max_y = machine.primary_screen->height() - 1;

	for (i = 0; i < state->m_spriteram_size ; i += 4)
	{
		int y = spriteram[i + 0];
		int x = spriteram[i + 1];
		int code = spriteram[i + 2];
		int attr = spriteram[i + 3];
		int flipx = attr & 0x80;
		int flipy = attr & 0x40;

		if (flip_screen_get(machine))
		{
			x = max_x - x - 8;
			y = max_y - y - 8;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
//              code + ((attr & 0x18)<<5),
				code + ((attr & 0x08)<<5),
				attr,
				flipx, flipy,
				x,y,0 );
	}
}




/***************************************************************************

                                Screen Drawing

***************************************************************************/

SCREEN_UPDATE_IND16( amspdwy )
{
	amspdwy_state *state = screen.machine().driver_data<amspdwy_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
