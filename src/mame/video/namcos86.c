/*******************************************************************

Namco System 86 Video Hardware

*******************************************************************/

#include "emu.h"
#include "includes/namcos86.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Rolling Thunder has two palette PROMs (512x8 and 512x4) and two 2048x8
  lookup table PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( namcos86 )
{
	namcos86_state *state = machine.driver_data<namcos86_state>();
	int i;
	rgb_t palette[512];

	for (i = 0;i < 512;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[512] >> 0) & 0x01;
		bit1 = (color_prom[512] >> 1) & 0x01;
		bit2 = (color_prom[512] >> 2) & 0x01;
		bit3 = (color_prom[512] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette[i] = MAKE_RGB(r,g,b);
		color_prom++;
	}

	color_prom += 512;
	/* color_prom now points to the beginning of the lookup table */

	/* tiles lookup table */
	for (i = 0;i < 2048;i++)
		palette_set_color(machine, i, palette[*color_prom++]);

	/* sprites lookup table */
	for (i = 0;i < 2048;i++)
		palette_set_color(machine, 2048 + i, palette[256 + *color_prom++]);

	/* color_prom now points to the beginning of the tile address decode PROM */

	state->m_tile_address_prom = color_prom;	/* we'll need this at run time */
}




/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void get_tile_info(running_machine &machine,tile_data &tileinfo,int tile_index,int layer,UINT8 *vram)
{
	namcos86_state *state = machine.driver_data<namcos86_state>();
	int attr = vram[2*tile_index + 1];
	int tile_offs;
	if (layer & 2)
		tile_offs = ((state->m_tile_address_prom[((layer & 1) << 4) + (attr & 0x03)] & 0xe0) >> 5) * 0x100;
	else
		tile_offs = ((state->m_tile_address_prom[((layer & 1) << 4) + ((attr & 0x03) << 2)] & 0x0e) >> 1) * 0x100 + state->m_tilebank * 0x800;

	SET_TILE_INFO(
			(layer & 2) ? 1 : 0,
			vram[2*tile_index] + tile_offs,
			attr,
			0);
}

static TILE_GET_INFO( get_tile_info0 )
{
	namcos86_state *state = machine.driver_data<namcos86_state>();
	get_tile_info(machine,tileinfo,tile_index,0,&state->m_rthunder_videoram1[0x0000]);
}

static TILE_GET_INFO( get_tile_info1 )
{
	namcos86_state *state = machine.driver_data<namcos86_state>();
	get_tile_info(machine,tileinfo,tile_index,1,&state->m_rthunder_videoram1[0x1000]);
}

static TILE_GET_INFO( get_tile_info2 )
{
	namcos86_state *state = machine.driver_data<namcos86_state>();
	get_tile_info(machine,tileinfo,tile_index,2,&state->m_rthunder_videoram2[0x0000]);
}

static TILE_GET_INFO( get_tile_info3 )
{
	namcos86_state *state = machine.driver_data<namcos86_state>();
	get_tile_info(machine,tileinfo,tile_index,3,&state->m_rthunder_videoram2[0x1000]);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( namcos86 )
{
	namcos86_state *state = machine.driver_data<namcos86_state>();
	state->m_bg_tilemap[0] = tilemap_create(machine, get_tile_info0,tilemap_scan_rows,8,8,64,32);
	state->m_bg_tilemap[1] = tilemap_create(machine, get_tile_info1,tilemap_scan_rows,8,8,64,32);
	state->m_bg_tilemap[2] = tilemap_create(machine, get_tile_info2,tilemap_scan_rows,8,8,64,32);
	state->m_bg_tilemap[3] = tilemap_create(machine, get_tile_info3,tilemap_scan_rows,8,8,64,32);

	state->m_bg_tilemap[0]->set_transparent_pen(7);
	state->m_bg_tilemap[1]->set_transparent_pen(7);
	state->m_bg_tilemap[2]->set_transparent_pen(7);
	state->m_bg_tilemap[3]->set_transparent_pen(7);

	state->m_spriteram = state->m_rthunder_spriteram + 0x1800;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_MEMBER(namcos86_state::rthunder_videoram1_r)
{
	return m_rthunder_videoram1[offset];
}

WRITE8_MEMBER(namcos86_state::rthunder_videoram1_w)
{
	m_rthunder_videoram1[offset] = data;
	m_bg_tilemap[offset/0x1000]->mark_tile_dirty((offset & 0xfff)/2);
}

READ8_MEMBER(namcos86_state::rthunder_videoram2_r)
{
	return m_rthunder_videoram2[offset];
}

WRITE8_MEMBER(namcos86_state::rthunder_videoram2_w)
{
	m_rthunder_videoram2[offset] = data;
	m_bg_tilemap[2+offset/0x1000]->mark_tile_dirty((offset & 0xfff)/2);
}

WRITE8_MEMBER(namcos86_state::rthunder_tilebank_select_w)
{
	int bit = BIT(offset,10);
	if (m_tilebank != bit)
	{
		m_tilebank = bit;
		m_bg_tilemap[0]->mark_all_dirty();
		m_bg_tilemap[1]->mark_all_dirty();
	}
}

static void scroll_w(address_space *space, int offset, int data, int layer)
{
	namcos86_state *state = space->machine().driver_data<namcos86_state>();
	switch (offset)
	{
		case 0:
			state->m_xscroll[layer] = (state->m_xscroll[layer]&0xff)|(data<<8);
			break;
		case 1:
			state->m_xscroll[layer] = (state->m_xscroll[layer]&0xff00)|data;
			break;
		case 2:
			state->m_yscroll[layer] = data;
			break;
	}
}

WRITE8_MEMBER(namcos86_state::rthunder_scroll0_w)
{
	scroll_w(&space,offset,data,0);
}
WRITE8_MEMBER(namcos86_state::rthunder_scroll1_w)
{
	scroll_w(&space,offset,data,1);
}
WRITE8_MEMBER(namcos86_state::rthunder_scroll2_w)
{
	scroll_w(&space,offset,data,2);
}
WRITE8_MEMBER(namcos86_state::rthunder_scroll3_w)
{
	scroll_w(&space,offset,data,3);
}

WRITE8_MEMBER(namcos86_state::rthunder_backcolor_w)
{
	m_backcolor = data;
}



READ8_MEMBER(namcos86_state::rthunder_spriteram_r)
{
	return m_rthunder_spriteram[offset];
}

WRITE8_MEMBER(namcos86_state::rthunder_spriteram_w)
{
	m_rthunder_spriteram[offset] = data;

	/* a write to this offset tells the sprite chip to buffer the sprite list */
	if (offset == 0x1ff2)
		m_copy_sprites = 1;
}


/***************************************************************************

  Display refresh

***************************************************************************/

/*
sprite format:

0-3  scratchpad RAM
4-9  CPU writes here, hardware copies from here to 10-15
10   xx------  X size (16, 8, 32, 4)
10   --x-----  X flip
10   ---xx---  X offset inside 32x32 tile
10   -----xxx  tile bank
11   xxxxxxxx  tile number
12   xxxxxxx-  color
12   -------x  X position MSB
13   xxxxxxxx  X position
14   xxx-----  priority
14   ---xx---  Y offset inside 32x32 tile
14   -----xx-  Y size (16, 8, 32, 4)
14   -------x  Y flip
15   xxxxxxxx  Y position
*/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	namcos86_state *state = machine.driver_data<namcos86_state>();
	const UINT8 *source = &state->m_spriteram[0x0800-0x20];	/* the last is NOT a sprite */
	const UINT8 *finish = &state->m_spriteram[0];
	gfx_element *gfx = machine.gfx[2];

	int sprite_xoffs = state->m_spriteram[0x07f5] + ((state->m_spriteram[0x07f4] & 1) << 8);
	int sprite_yoffs = state->m_spriteram[0x07f7];

	int bank_sprites = machine.gfx[2]->total_elements / 8;

	while (source >= finish)
	{
		static const int sprite_size[4] = { 16, 8, 32, 4 };
		int attr1 = source[10];
		int attr2 = source[14];
		int color = source[12];
		int flipx = (attr1 & 0x20) >> 5;
		int flipy = (attr2 & 0x01);
		int sizex = sprite_size[(attr1 & 0xc0) >> 6];
		int sizey = sprite_size[(attr2 & 0x06) >> 1];
		int tx = (attr1 & 0x18) & (~(sizex-1));
		int ty = (attr2 & 0x18) & (~(sizey-1));
		int sx = source[13] + ((color & 0x01) << 8);
		int sy = -source[15] - sizey;
		int sprite = source[11];
		int sprite_bank = attr1 & 7;
		int priority = (source[14] & 0xe0) >> 5;
		int pri_mask = (0xff << (priority + 1)) & 0xff;

		sprite &= bank_sprites-1;
		sprite += sprite_bank * bank_sprites;
		color = color >> 1;

		sx += sprite_xoffs;
		sy -= sprite_yoffs;

		if (flip_screen_get(machine))
		{
			sx = -sx - sizex;
			sy = -sy - sizey;
			flipx ^= 1;
			flipy ^= 1;
		}

		sy++;	/* sprites are buffered and delayed by one scanline */

		gfx_element_set_source_clip(gfx, tx, sizex, ty, sizey);
		pdrawgfx_transpen( bitmap, cliprect,gfx,
				sprite,
				color,
				flipx,flipy,
				sx & 0x1ff,
				((sy + 16) & 0xff) - 16,
				machine.priority_bitmap, pri_mask,0xf);

		source -= 0x10;
	}
}


static void set_scroll(running_machine &machine, int layer)
{
	namcos86_state *state = machine.driver_data<namcos86_state>();
	static const int xdisp[4] = { 47, 49, 46, 48 };
	int scrollx,scrolly;

	scrollx = state->m_xscroll[layer] - xdisp[layer];
	scrolly = state->m_yscroll[layer] + 9;
	if (flip_screen_get(machine))
	{
		scrollx = -scrollx;
		scrolly = -scrolly;
	}
	state->m_bg_tilemap[layer]->set_scrollx(0, scrollx);
	state->m_bg_tilemap[layer]->set_scrolly(0, scrolly);
}


SCREEN_UPDATE_IND16( namcos86 )
{
	namcos86_state *state = screen.machine().driver_data<namcos86_state>();
	int layer;

	/* flip screen is embedded in the sprite control registers */
	/* can't use flip_screen_set(screen.machine(), ) because the visible area is asymmetrical */
	flip_screen_set_no_update(screen.machine(), state->m_spriteram[0x07f6] & 1);
	screen.machine().tilemap().set_flip_all(flip_screen_get(screen.machine()) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	set_scroll(screen.machine(), 0);
	set_scroll(screen.machine(), 1);
	set_scroll(screen.machine(), 2);
	set_scroll(screen.machine(), 3);

	screen.machine().priority_bitmap.fill(0, cliprect);

	bitmap.fill(screen.machine().gfx[0]->color_base + 8*state->m_backcolor+7, cliprect);

	for (layer = 0;layer < 8;layer++)
	{
		int i;

		for (i = 3;i >= 0;i--)
		{
			if (((state->m_xscroll[i] & 0x0e00) >> 9) == layer)
				state->m_bg_tilemap[i]->draw(bitmap, cliprect, 0,layer,0);
		}
	}

	draw_sprites(screen.machine(),bitmap,cliprect);
	return 0;
}


SCREEN_VBLANK( namcos86 )
{
	// rising edge
	if (vblank_on)
	{
		namcos86_state *state = screen.machine().driver_data<namcos86_state>();
		if (state->m_copy_sprites)
		{
			UINT8 *spriteram = state->m_spriteram;
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
