/* Flower Video Hardware */

#include "emu.h"
#include "includes/flower.h"


PALETTE_INIT( flower )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	for (i = 0; i < 0x100; i++)
		colortable_entry_set_value(machine.colortable, i, i);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	flower_state *state = machine.driver_data<flower_state>();
	const gfx_element *gfx = machine.gfx[1];
	UINT8 *source = state->m_spriteram + 0x200;
	UINT8 *finish = source - 0x200;

	source -= 8;

	while( source>=finish )
	{
		int xblock,yblock;
		int sy = 256-32-source[0]+1;
		int	sx = (source[4]|(source[5]<<8))-55;
		int code = source[1] & 0x3f;
		int color = (source[6]>>4);

		/*
            Byte 0: Y
            Byte 1:
                0x80 - FlipY
                0x40 - FlipX
                0x3f - Tile
            Byte 2:
                0x08 - Tile MSB
                0x01 - Tile MSB
            Byte 3:
                0x07 - X Zoom
                0x08 - X Size
                0x70 - Y Zoom
                0x80 - Y Size
            Byte 4: X LSB
            Byte 5: X MSB
            Byte 6:
                0xf0 - Colour
        */

		int flipy = source[1] & 0x80;
		int flipx = source[1] & 0x40;

		int size = source[3];

		int xsize = ((size & 0x08)>>3);
		int ysize = ((size & 0x80)>>7);

		xsize++;
		ysize++;

		if (ysize==2) sy -= 16;

		code |= ((source[2] & 0x01) << 6);
		code |= ((source[2] & 0x08) << 4);

		if(flip_screen_get(machine))
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = sx+16;
			sy = 250-sy;

			if (ysize==2) sy += 16;
		}

		for (xblock = 0; xblock<xsize; xblock++)
		{
			int xoffs=!flipx ? (xblock*8) : ((xsize-xblock-1)*8);
			int zoomx=((size&7)+1)<<13;
			int zoomy=((size&0x70)+0x10)<<9;
			int xblocksizeinpixels=(zoomx*16)>>16;
			int yblocksizeinpixels=(zoomy*16)>>16;

			for (yblock = 0; yblock<ysize; yblock++)
			{
				int yoffs=!flipy ? yblock : (ysize-yblock-1);
				int sxoffs=(16-xblocksizeinpixels)/2;
				int syoffs=(16-yblocksizeinpixels)/2;
				if (xblock) sxoffs+=xblocksizeinpixels;
				if (yblock) syoffs+=yblocksizeinpixels;

				drawgfxzoom_transpen(bitmap,cliprect,gfx,
						code+yoffs+xoffs,
						color,
						flipx,flipy,
						sx+sxoffs,sy+syoffs,
						zoomx,zoomy,15);
			}
		}
		source -= 8;
	}

}

static TILE_GET_INFO( get_bg0_tile_info )
{
	flower_state *state = machine.driver_data<flower_state>();
	int code = state->m_bg0ram[tile_index];
	int color = state->m_bg0ram[tile_index+0x100];
	/* Todo - may be tile flip bits? */

	SET_TILE_INFO(2, code, color>>4, 0);
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	flower_state *state = machine.driver_data<flower_state>();
	int code = state->m_bg1ram[tile_index];
	int color = state->m_bg1ram[tile_index+0x100];
	/* Todo - may be tile flip bits? */

	SET_TILE_INFO(2, code, color>>4, 0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	flower_state *state = machine.driver_data<flower_state>();
	int code = state->m_textram[tile_index];
	int color = state->m_textram[tile_index+0x400];
	/* Todo - may be tile flip bits? */

	SET_TILE_INFO(0, code, color>>2, 0);
}

VIDEO_START(flower)
{
	flower_state *state = machine.driver_data<flower_state>();
	state->m_bg0_tilemap        = tilemap_create(machine, get_bg0_tile_info, tilemap_scan_rows,16,16,16,16);
	state->m_bg1_tilemap        = tilemap_create(machine, get_bg1_tile_info, tilemap_scan_rows,16,16,16,16);
	state->m_text_tilemap       = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows, 8, 8,32,32);
	state->m_text_right_tilemap = tilemap_create(machine, get_text_tile_info,tilemap_scan_cols, 8, 8, 2,32);

	state->m_bg1_tilemap->set_transparent_pen(15);
	state->m_text_tilemap->set_transparent_pen(3);
	state->m_text_right_tilemap->set_transparent_pen(3);

	state->m_text_tilemap->set_scrolly(0, 16);
	state->m_text_right_tilemap->set_scrolly(0, 16);
}

SCREEN_UPDATE_IND16( flower )
{
	flower_state *state = screen.machine().driver_data<flower_state>();
	rectangle myclip = cliprect;

	state->m_bg0_tilemap->set_scrolly(0, state->m_bg0_scroll[0]+16);
	state->m_bg1_tilemap->set_scrolly(0, state->m_bg1_scroll[0]+16);

	state->m_bg0_tilemap->draw(bitmap, cliprect, 0,0);
	state->m_bg1_tilemap->draw(bitmap, cliprect, 0,0);

	draw_sprites(screen.machine(),bitmap,cliprect);

	if(flip_screen_get(screen.machine()))
	{
		myclip.min_x = cliprect.min_x;
		myclip.max_x = cliprect.min_x + 15;
	}
	else
	{
		myclip.min_x = cliprect.max_x - 15;
		myclip.max_x = cliprect.max_x;
	}

	state->m_text_tilemap->draw(bitmap, cliprect, 0,0);
	state->m_text_right_tilemap->draw(bitmap, myclip, 0,0);
	return 0;
}

WRITE8_MEMBER(flower_state::flower_textram_w)
{
	m_textram[offset] = data;
	m_text_tilemap->mark_tile_dirty(offset);
	m_text_right_tilemap->mark_all_dirty();
}

WRITE8_MEMBER(flower_state::flower_bg0ram_w)
{
	m_bg0ram[offset] = data;
	m_bg0_tilemap->mark_tile_dirty(offset & 0x1ff);
}

WRITE8_MEMBER(flower_state::flower_bg1ram_w)
{
	m_bg1ram[offset] = data;
	m_bg1_tilemap->mark_tile_dirty(offset & 0x1ff);
}

WRITE8_MEMBER(flower_state::flower_flipscreen_w)
{
	flip_screen_set(machine(), data);
}
