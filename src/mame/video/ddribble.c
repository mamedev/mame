/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/ddribble.h"


PALETTE_INIT( ddribble )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x40);

	for (i = 0x10; i < 0x40; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	/* sprite #2 uses pens 0x00-0x0f */
	for (i = 0x40; i < 0x140; i++)
	{
		UINT8 ctabentry = color_prom[i - 0x40] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}


static void set_pens( running_machine &machine )
{
	ddribble_state *state = machine.driver_data<ddribble_state>();
	int i;

	for (i = 0x00; i < 0x80; i += 2)
	{
		UINT16 data = state->m_paletteram[i | 1] | (state->m_paletteram[i] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
		colortable_palette_set_color(machine.colortable, i >> 1, color);
	}
}


WRITE8_MEMBER(ddribble_state::K005885_0_w)
{
	switch (offset)
	{
		case 0x03:	/* char bank selection for set 1 */
			if ((data & 0x03) != m_charbank[0])
			{
				m_charbank[0] = data & 0x03;
				m_fg_tilemap->mark_all_dirty();
			}
			break;
		case 0x04:	/* IRQ control, flipscreen */
			m_int_enable_0 = data & 0x02;
			break;
	}
	m_vregs[0][offset] = data;
}

WRITE8_MEMBER(ddribble_state::K005885_1_w)
{
	switch (offset)
	{
		case 0x03:	/* char bank selection for set 2 */
			if ((data & 0x03) != m_charbank[1])
			{
				m_charbank[1] = data & 0x03;
				m_bg_tilemap->mark_all_dirty();
			}
			break;
		case 0x04:	/* IRQ control, flipscreen */
			m_int_enable_1 = data & 0x02;
			break;
	}
	m_vregs[1][offset] = data;
}

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( tilemap_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 6);	/* skip 0x400 */
}

static TILE_GET_INFO( get_fg_tile_info )
{
	ddribble_state *state = machine.driver_data<ddribble_state>();
	UINT8 attr = state->m_fg_videoram[tile_index];
	int num = state->m_fg_videoram[tile_index + 0x400] + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5) + ((state->m_charbank[0] & 2) << 10);
	SET_TILE_INFO(
			0,
			num,
			0,
			TILE_FLIPYX((attr & 0x30) >> 4));
}

static TILE_GET_INFO( get_bg_tile_info )
{
	ddribble_state *state = machine.driver_data<ddribble_state>();
	UINT8 attr = state->m_bg_videoram[tile_index];
	int num = state->m_bg_videoram[tile_index + 0x400] + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5) + (state->m_charbank[1] << 11);
	SET_TILE_INFO(
			1,
			num,
			0,
			TILE_FLIPYX((attr & 0x30) >> 4));
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ddribble )
{
	ddribble_state *state = machine.driver_data<ddribble_state>();

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan, 8, 8, 64, 32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan, 8, 8, 64, 32);

	state->m_fg_tilemap->set_transparent_pen(0);
}

/***************************************************************************

    Memory handlers

***************************************************************************/

WRITE8_MEMBER(ddribble_state::ddribble_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0xbff);
}

WRITE8_MEMBER(ddribble_state::ddribble_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0xbff);
}

/***************************************************************************

    Double Dribble sprites

Each sprite has 5 bytes:
byte #0:    sprite number
byte #1:
    bits 0..2:  sprite bank #
    bit 3:      not used?
    bits 4..7:  sprite color
byte #2:    y position
byte #3:    x position
byte #4:    attributes
    bit 0:      x position (high bit)
    bit 1:      ???
    bits 2..4:  sprite size
    bit 5:      flip x
    bit 6:      flip y
    bit 7:      unused?

***************************************************************************/

static void draw_sprites( running_machine& machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8* source, int lenght, int gfxset, int flipscreen )
{
	gfx_element *gfx = machine.gfx[gfxset];
	const UINT8 *finish = source + lenght;

	while (source < finish)
	{
		int number = source[0] | ((source[1] & 0x07) << 8);	/* sprite number */
		int attr = source[4];								/* attributes */
		int sx = source[3] | ((attr & 0x01) << 8);			/* vertical position */
		int sy = source[2];									/* horizontal position */
		int flipx = attr & 0x20;							/* flip x */
		int flipy = attr & 0x40;							/* flip y */
		int color = (source[1] & 0xf0) >> 4;				/* color */
		int width, height;

		if (flipscreen)
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = 240 - sx;
			sy = 240 - sy;

			if ((attr & 0x1c) == 0x10)
			{	/* ???. needed for some sprites in flipped mode */
				sx -= 0x10;
				sy -= 0x10;
			}
		}

		switch (attr & 0x1c)
		{
			case 0x10:	/* 32x32 */
				width = height = 2; number &= (~3); break;
			case 0x08:	/* 16x32 */
				width = 1; height = 2; number &= (~2); break;
			case 0x04:	/* 32x16 */
				width = 2; height = 1; number &= (~1); break;
			/* the hardware allow more sprite sizes, but ddribble doesn't use them */
			default:	/* 16x16 */
				width = height = 1; break;
		}

		{
			static const int x_offset[2] = { 0x00, 0x01 };
			static const int y_offset[2] = { 0x00, 0x02 };
			int x, y, ex, ey;

			for (y = 0; y < height; y++)
			{
				for (x = 0; x < width; x++)
				{
					ex = flipx ? (width - 1 - x) : x;
					ey = flipy ? (height - 1 - y) : y;

					drawgfx_transpen(bitmap,cliprect,
						gfx,
						(number)+x_offset[ex]+y_offset[ey],
						color,
						flipx, flipy,
						sx+x*16,sy+y*16, 0);
				}
			}
		}
		source += 5;
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

SCREEN_UPDATE_IND16( ddribble )
{
	ddribble_state *state = screen.machine().driver_data<ddribble_state>();
	set_pens(screen.machine());

	state->m_fg_tilemap->set_flip((state->m_vregs[0][4] & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	state->m_bg_tilemap->set_flip((state->m_vregs[1][4] & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* set scroll registers */
	state->m_fg_tilemap->set_scrollx(0, state->m_vregs[0][1] | ((state->m_vregs[0][2] & 0x01) << 8));
	state->m_bg_tilemap->set_scrollx(0, state->m_vregs[1][1] | ((state->m_vregs[1][2] & 0x01) << 8));
	state->m_fg_tilemap->set_scrolly(0, state->m_vregs[0][0]);
	state->m_bg_tilemap->set_scrolly(0, state->m_vregs[1][0]);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram_1, 0x07d, 2, state->m_vregs[0][4] & 0x08);
	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram_2, 0x140, 3, state->m_vregs[1][4] & 0x08);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
