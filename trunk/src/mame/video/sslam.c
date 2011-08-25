/* Super Slam - Video Hardware */

#include "emu.h"
#include "includes/sslam.h"


WRITE16_HANDLER( sslam_paletteram_w )
{
	int r, g, b, val;

	COMBINE_DATA(&space->machine().generic.paletteram.u16[offset]);

	val = space->machine().generic.paletteram.u16[offset];
	r = (val >> 11) & 0x1e;
	g = (val >>  7) & 0x1e;
	b = (val >>  3) & 0x1e;

	r |= ((val & 0x08) >> 3);
	g |= ((val & 0x04) >> 2);
	b |= ((val & 0x02) >> 1);

	palette_set_color_rgb(space->machine(), offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	sslam_state *state = machine.driver_data<sslam_state>();
	const gfx_element *gfx = machine.gfx[0];
	UINT16 *source = state->m_spriteram;
	UINT16 *finish = source + 0x1000/2;

	source += 3; // strange

	while( source<finish )
	{
		int xpos, ypos, number, flipx, colr, eightbyeight;

		if (source[0] & 0x2000) break;

		xpos = source[2] & 0x1ff;
		ypos = source[0] & 0x01ff;
		colr = (source[2] & 0xf000) >> 12;
		eightbyeight = source[0] & 0x1000;
		flipx = source[0] & 0x4000;
		number = source[3];

		xpos -=16; xpos -=7; xpos += state->m_sprites_x_offset;
		ypos = 0xff - ypos;
		ypos -=16; ypos -=7;

		if(ypos < 0)
			ypos += 256;

		if(ypos >= 249)
			ypos -= 256;

		if (!eightbyeight)
		{
			if (flipx)
			{
				drawgfx_transpen(bitmap,cliprect,gfx,number,  colr,1,0,xpos+8,ypos,0);
				drawgfx_transpen(bitmap,cliprect,gfx,number+1,colr,1,0,xpos+8,ypos+8,0);
				drawgfx_transpen(bitmap,cliprect,gfx,number+2,colr,1,0,xpos,  ypos,0);
				drawgfx_transpen(bitmap,cliprect,gfx,number+3,colr,1,0,xpos,  ypos+8,0);
			}
			else
			{
				drawgfx_transpen(bitmap,cliprect,gfx,number,  colr,0,0,xpos,  ypos,0);
				drawgfx_transpen(bitmap,cliprect,gfx,number+1,colr,0,0,xpos,  ypos+8,0);
				drawgfx_transpen(bitmap,cliprect,gfx,number+2,colr,0,0,xpos+8,ypos,0);
				drawgfx_transpen(bitmap,cliprect,gfx,number+3,colr,0,0,xpos+8,ypos+8,0);
			}
		}
		else
		{
			if (flipx)
			{
				drawgfx_transpen(bitmap,cliprect,gfx,number ^ 2,colr,1,0,xpos,ypos,0);
			}
			else
			{
				drawgfx_transpen(bitmap,cliprect,gfx,number,colr,0,0,xpos,ypos,0);
			}
		}

		source += 4;
	}

}


/* Text Layer */

static TILE_GET_INFO( get_sslam_tx_tile_info )
{
	sslam_state *state = machine.driver_data<sslam_state>();
	int code = state->m_tx_tileram[tile_index] & 0x0fff;
	int colr = state->m_tx_tileram[tile_index] & 0xf000;

	SET_TILE_INFO(3,code+0xc000 ,colr >> 12,0);
}

WRITE16_HANDLER( sslam_tx_tileram_w )
{
	sslam_state *state = space->machine().driver_data<sslam_state>();

	COMBINE_DATA(&state->m_tx_tileram[offset]);
	tilemap_mark_tile_dirty(state->m_tx_tilemap,offset);
}

/* Middle Layer */

static TILE_GET_INFO( get_sslam_md_tile_info )
{
	sslam_state *state = machine.driver_data<sslam_state>();
	int code = state->m_md_tileram[tile_index] & 0x0fff;
	int colr = state->m_md_tileram[tile_index] & 0xf000;

	SET_TILE_INFO(2,code+0x2000 ,colr >> 12,0);
}

WRITE16_HANDLER( sslam_md_tileram_w )
{
	sslam_state *state = space->machine().driver_data<sslam_state>();

	COMBINE_DATA(&state->m_md_tileram[offset]);
	tilemap_mark_tile_dirty(state->m_md_tilemap,offset);
}

/* Background Layer */

static TILE_GET_INFO( get_sslam_bg_tile_info )
{
	sslam_state *state = machine.driver_data<sslam_state>();
	int code = state->m_bg_tileram[tile_index] & 0x1fff;
	int colr = state->m_bg_tileram[tile_index] & 0xe000;

	SET_TILE_INFO(1,code ,colr >> 13,0);
}

WRITE16_HANDLER( sslam_bg_tileram_w )
{
	sslam_state *state = space->machine().driver_data<sslam_state>();

	COMBINE_DATA(&state->m_bg_tileram[offset]);
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset);
}

static TILE_GET_INFO( get_powerbls_bg_tile_info )
{
	sslam_state *state = machine.driver_data<sslam_state>();
	int code = state->m_bg_tileram[tile_index*2+1] & 0x0fff;
	int colr = (state->m_bg_tileram[tile_index*2+1] & 0xf000) >> 12;
	code |= (state->m_bg_tileram[tile_index*2] & 0x0f00) << 4;

	//(state->m_bg_tileram[tile_index*2] & 0x0f00) == 0xf000 ???

	SET_TILE_INFO(1,code,colr,0);
}

WRITE16_HANDLER( powerbls_bg_tileram_w )
{
	sslam_state *state = space->machine().driver_data<sslam_state>();

	COMBINE_DATA(&state->m_bg_tileram[offset]);
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset>>1);
}

VIDEO_START(sslam)
{
	sslam_state *state = machine.driver_data<sslam_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_sslam_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_md_tilemap = tilemap_create(machine, get_sslam_md_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_tx_tilemap = tilemap_create(machine, get_sslam_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	tilemap_set_transparent_pen(state->m_md_tilemap,0);
	tilemap_set_transparent_pen(state->m_tx_tilemap,0);

	state->m_sprites_x_offset = 0;
	state->save_item(NAME(state->m_sprites_x_offset));
}

VIDEO_START(powerbls)
{
	sslam_state *state = machine.driver_data<sslam_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_powerbls_bg_tile_info,tilemap_scan_rows,8,8,64,64);

	state->m_sprites_x_offset = -21;
	state->save_item(NAME(state->m_sprites_x_offset));
}

SCREEN_UPDATE(sslam)
{
	sslam_state *state = screen->machine().driver_data<sslam_state>();

	if (!(state->m_regs[6] & 1))
	{
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine()));
		return 0;
	}

	tilemap_set_scrollx(state->m_tx_tilemap,0, state->m_regs[0]+1);	/* +0 looks better, but the real board has the left most pixel at the left edge shifted off screen */
	tilemap_set_scrolly(state->m_tx_tilemap,0, (state->m_regs[1] & 0xff)+8);
	tilemap_set_scrollx(state->m_md_tilemap,0, state->m_regs[2]+2);
	tilemap_set_scrolly(state->m_md_tilemap,0, state->m_regs[3]+8);
	tilemap_set_scrollx(state->m_bg_tilemap,0, state->m_regs[4]+4);
	tilemap_set_scrolly(state->m_bg_tilemap,0, state->m_regs[5]+8);

	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);

	/* remove wraparound from the tilemap (used on title screen) */
	if (state->m_regs[2]+2 > 0x8c8)
	{
		rectangle md_clip;
		md_clip.min_x = cliprect->min_x;
		md_clip.max_x = cliprect->max_x - (state->m_regs[2]+2 - 0x8c8);
		md_clip.min_y = cliprect->min_y;
		md_clip.max_y = cliprect->max_y;

		tilemap_draw(bitmap,&md_clip,state->m_md_tilemap,0,0);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,state->m_md_tilemap,0,0);
	}

	draw_sprites(screen->machine(), bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,state->m_tx_tilemap,0,0);
	return 0;
}

SCREEN_UPDATE(powerbls)
{
	sslam_state *state = screen->machine().driver_data<sslam_state>();

	if (!(state->m_regs[6] & 1))
	{
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine()));
		return 0;
	}

	tilemap_set_scrollx(state->m_bg_tilemap,0, state->m_regs[0]+21);
	tilemap_set_scrolly(state->m_bg_tilemap,0, state->m_regs[1]-240);

	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect);
	return 0;
}
