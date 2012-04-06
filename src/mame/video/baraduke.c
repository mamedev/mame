#include "emu.h"
#include "includes/baraduke.h"


/***************************************************************************

    Convert the color PROMs.

    The palette PROMs are connected to the RGB output this way:

    bit 3   -- 220 ohm resistor  -- RED/GREEN/BLUE
            -- 470 ohm resistor  -- RED/GREEN/BLUE
            -- 1  kohm resistor  -- RED/GREEN/BLUE
    bit 0   -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( baraduke )
{
	int i;
	int bit0,bit1,bit2,bit3,r,g,b;

	for (i = 0; i < 2048; i++)
	{
		/* red component */
		bit0 = (color_prom[2048] >> 0) & 0x01;
		bit1 = (color_prom[2048] >> 1) & 0x01;
		bit2 = (color_prom[2048] >> 2) & 0x01;
		bit3 = (color_prom[2048] >> 3) & 0x01;
		r = 0x0e*bit0 + 0x1f*bit1 + 0x43*bit2 + 0x8f*bit3;

		/* green component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		g = 0x0e*bit0 + 0x1f*bit1 + 0x43*bit2 + 0x8f*bit3;

		/* blue component */
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		b = 0x0e*bit0 + 0x1f*bit1 + 0x43*bit2 + 0x8f*bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( tx_tilemap_scan )
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

static TILE_GET_INFO( tx_get_tile_info )
{
	baraduke_state *state = machine.driver_data<baraduke_state>();
	SET_TILE_INFO(
			0,
			state->m_textram[tile_index],
			(state->m_textram[tile_index+0x400] << 2) & 0x1ff,
			0);
}

static TILE_GET_INFO( get_tile_info0 )
{
	baraduke_state *state = machine.driver_data<baraduke_state>();
	int code = state->m_videoram[2*tile_index];
	int attr = state->m_videoram[2*tile_index + 1];

	SET_TILE_INFO(
			1,
			code + ((attr & 0x03) << 8),
			attr,
			0);
}

static TILE_GET_INFO( get_tile_info1 )
{
	baraduke_state *state = machine.driver_data<baraduke_state>();
	int code = state->m_videoram[0x1000 + 2*tile_index];
	int attr = state->m_videoram[0x1000 + 2*tile_index + 1];

	SET_TILE_INFO(
			2,
			code + ((attr & 0x03) << 8),
			attr,
			0);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( baraduke )
{
	baraduke_state *state = machine.driver_data<baraduke_state>();
	state->m_tx_tilemap = tilemap_create(machine, tx_get_tile_info,tx_tilemap_scan,8,8,36,28);
	state->m_bg_tilemap[0] = tilemap_create(machine, get_tile_info0,tilemap_scan_rows,8,8,64,32);
	state->m_bg_tilemap[1] = tilemap_create(machine, get_tile_info1,tilemap_scan_rows,8,8,64,32);

	state->m_tx_tilemap->set_transparent_pen(3);
	state->m_bg_tilemap[0]->set_transparent_pen(7);
	state->m_bg_tilemap[1]->set_transparent_pen(7);

	state->m_tx_tilemap->set_scrolldx(0,512-288);
	state->m_tx_tilemap->set_scrolldy(16,16);
}



/***************************************************************************

    Memory handlers

***************************************************************************/

READ8_MEMBER(baraduke_state::baraduke_videoram_r)
{
	return m_videoram[offset];
}

WRITE8_MEMBER(baraduke_state::baraduke_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap[offset/0x1000]->mark_tile_dirty((offset&0xfff)/2);
}

READ8_MEMBER(baraduke_state::baraduke_textram_r)
{
	return m_textram[offset];
}

WRITE8_MEMBER(baraduke_state::baraduke_textram_w)
{
	m_textram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}


static void scroll_w(address_space *space, int layer, int offset, int data)
{
	baraduke_state *state = space->machine().driver_data<baraduke_state>();
	switch (offset)
	{
		case 0:	/* high scroll x */
			state->m_xscroll[layer] = (state->m_xscroll[layer] & 0xff) | (data << 8);
			break;
		case 1:	/* low scroll x */
			state->m_xscroll[layer] = (state->m_xscroll[layer] & 0xff00) | data;
			break;
		case 2:	/* scroll y */
			state->m_yscroll[layer] = data;
			break;
	}
}

WRITE8_MEMBER(baraduke_state::baraduke_scroll0_w)
{
	scroll_w(&space, 0, offset, data);
}
WRITE8_MEMBER(baraduke_state::baraduke_scroll1_w)
{
	scroll_w(&space, 1, offset, data);
}



READ8_MEMBER(baraduke_state::baraduke_spriteram_r)
{
	return m_spriteram[offset];
}

WRITE8_MEMBER(baraduke_state::baraduke_spriteram_w)
{
	m_spriteram[offset] = data;

	/* a write to this offset tells the sprite chip to buffer the sprite list */
	if (offset == 0x1ff2)
		m_copy_sprites = 1;
}



/***************************************************************************

    Display Refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority)
{
	baraduke_state *state = machine.driver_data<baraduke_state>();
	UINT8 *spriteram = state->m_spriteram + 0x1800;
	const UINT8 *source = &spriteram[0x0000];
	const UINT8 *finish = &spriteram[0x0800-16];	/* the last is NOT a sprite */

	int sprite_xoffs = spriteram[0x07f5] - 256 * (spriteram[0x07f4] & 1);
	int sprite_yoffs = spriteram[0x07f7];

	while( source<finish )
	{
/*
    source[10] S-FT ---P
    source[11] TTTT TTTT
    source[12] CCCC CCCX
    source[13] XXXX XXXX
    source[14] ---T -S-F
    source[15] YYYY YYYY
*/
		int priority = source[10] & 0x01;
		if (priority == sprite_priority)
		{
			static const int gfx_offs[2][2] =
			{
				{ 0, 1 },
				{ 2, 3 }
			};
			int attr1 = source[10];
			int attr2 = source[14];
			int color = source[12];
			int sx = source[13] + (color & 0x01)*256;
			int sy = 240 - source[15];
			int flipx = (attr1 & 0x20) >> 5;
			int flipy = (attr2 & 0x01);
			int sizex = (attr1 & 0x80) >> 7;
			int sizey = (attr2 & 0x04) >> 2;
			int sprite = (source[11] & 0xff)*4;
			int x,y;

			if ((attr1 & 0x10) && !sizex) sprite += 1;
			if ((attr2 & 0x10) && !sizey) sprite += 2;
			color = color >> 1;

			sx += sprite_xoffs;
			sy -= sprite_yoffs;

			sy -= 16 * sizey;

			if (flip_screen_get(machine))
			{
				sx = 496+3 - 16 * sizex - sx;
				sy = 240 - 16 * sizey - sy;
				flipx ^= 1;
				flipy ^= 1;
			}

			for (y = 0;y <= sizey;y++)
			{
				for (x = 0;x <= sizex;x++)
				{
					drawgfx_transpen( bitmap, cliprect,machine.gfx[3],
						sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
						color,
						flipx,flipy,
						-71 + ((sx + 16*x) & 0x1ff),
						1 + ((sy + 16*y) & 0xff),0xf);
				}
			}
		}

		source+=16;
	}
}


static void set_scroll(running_machine &machine, int layer)
{
	baraduke_state *state = machine.driver_data<baraduke_state>();
	static const int xdisp[2] = { 26, 24 };
	int scrollx, scrolly;

	scrollx = state->m_xscroll[layer] + xdisp[layer];
	scrolly = state->m_yscroll[layer] + 9;
	if (flip_screen_get(machine))
	{
		scrollx = -scrollx + 3;
		scrolly = -scrolly;
	}

	state->m_bg_tilemap[layer]->set_scrollx(0, scrollx);
	state->m_bg_tilemap[layer]->set_scrolly(0, scrolly);
}


SCREEN_UPDATE_IND16( baraduke )
{
	baraduke_state *state = screen.machine().driver_data<baraduke_state>();
	UINT8 *spriteram = state->m_spriteram + 0x1800;
	int back;

	/* flip screen is embedded in the sprite control registers */
	/* can't use flip_screen_set(screen.machine(), ) because the visible area is asymmetrical */
	flip_screen_set_no_update(screen.machine(), spriteram[0x07f6] & 0x01);
	screen.machine().tilemap().set_flip_all(flip_screen_get(screen.machine()) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	set_scroll(screen.machine(), 0);
	set_scroll(screen.machine(), 1);

	if (((state->m_xscroll[0] & 0x0e00) >> 9) == 6)
		back = 1;
	else
		back = 0;

	state->m_bg_tilemap[back]->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(screen.machine(), bitmap,cliprect,0);
	state->m_bg_tilemap[back ^ 1]->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect,1);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}


SCREEN_VBLANK( baraduke )
{
	// rising edge
	if (vblank_on)
	{
		baraduke_state *state = screen.machine().driver_data<baraduke_state>();
		if (state->m_copy_sprites)
		{
			UINT8 *spriteram = state->m_spriteram + 0x1800;
			int i,j;

			for (i = 0;i < 0x800;i += 16)
			{
				for (j = 10;j < 16;j++)
					spriteram[i+j] = spriteram[i+j - 6];
			}

			state->m_copy_sprites = 0;
		}
	}
}
