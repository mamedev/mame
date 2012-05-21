/* Super Slam - Video Hardware */

#include "emu.h"
#include "includes/sslam.h"


WRITE16_MEMBER(sslam_state::sslam_paletteram_w)
{
	int r, g, b, val;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);

	val = m_generic_paletteram_16[offset];
	r = (val >> 11) & 0x1e;
	g = (val >>  7) & 0x1e;
	b = (val >>  3) & 0x1e;

	r |= ((val & 0x08) >> 3);
	g |= ((val & 0x04) >> 2);
	b |= ((val & 0x02) >> 1);

	palette_set_color_rgb(machine(), offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

WRITE16_MEMBER(sslam_state::sslam_tx_tileram_w)
{

	COMBINE_DATA(&m_tx_tileram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

/* Middle Layer */

static TILE_GET_INFO( get_sslam_md_tile_info )
{
	sslam_state *state = machine.driver_data<sslam_state>();
	int code = state->m_md_tileram[tile_index] & 0x0fff;
	int colr = state->m_md_tileram[tile_index] & 0xf000;

	SET_TILE_INFO(2,code+0x2000 ,colr >> 12,0);
}

WRITE16_MEMBER(sslam_state::sslam_md_tileram_w)
{

	COMBINE_DATA(&m_md_tileram[offset]);
	m_md_tilemap->mark_tile_dirty(offset);
}

/* Background Layer */

static TILE_GET_INFO( get_sslam_bg_tile_info )
{
	sslam_state *state = machine.driver_data<sslam_state>();
	int code = state->m_bg_tileram[tile_index] & 0x1fff;
	int colr = state->m_bg_tileram[tile_index] & 0xe000;

	SET_TILE_INFO(1,code ,colr >> 13,0);
}

WRITE16_MEMBER(sslam_state::sslam_bg_tileram_w)
{

	COMBINE_DATA(&m_bg_tileram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
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

WRITE16_MEMBER(sslam_state::powerbls_bg_tileram_w)
{

	COMBINE_DATA(&m_bg_tileram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset>>1);
}

VIDEO_START(sslam)
{
	sslam_state *state = machine.driver_data<sslam_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_sslam_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_md_tilemap = tilemap_create(machine, get_sslam_md_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_tx_tilemap = tilemap_create(machine, get_sslam_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	state->m_md_tilemap->set_transparent_pen(0);
	state->m_tx_tilemap->set_transparent_pen(0);

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

SCREEN_UPDATE_IND16(sslam)
{
	sslam_state *state = screen.machine().driver_data<sslam_state>();

	if (!(state->m_regs[6] & 1))
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
		return 0;
	}

	state->m_tx_tilemap->set_scrollx(0, state->m_regs[0]+1);	/* +0 looks better, but the real board has the left most pixel at the left edge shifted off screen */
	state->m_tx_tilemap->set_scrolly(0, (state->m_regs[1] & 0xff)+8);
	state->m_md_tilemap->set_scrollx(0, state->m_regs[2]+2);
	state->m_md_tilemap->set_scrolly(0, state->m_regs[3]+8);
	state->m_bg_tilemap->set_scrollx(0, state->m_regs[4]+4);
	state->m_bg_tilemap->set_scrolly(0, state->m_regs[5]+8);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);

	/* remove wraparound from the tilemap (used on title screen) */
	if (state->m_regs[2]+2 > 0x8c8)
	{
		rectangle md_clip;
		md_clip.min_x = cliprect.min_x;
		md_clip.max_x = cliprect.max_x - (state->m_regs[2]+2 - 0x8c8);
		md_clip.min_y = cliprect.min_y;
		md_clip.max_y = cliprect.max_y;

		state->m_md_tilemap->draw(bitmap, md_clip, 0,0);
	}
	else
	{
		state->m_md_tilemap->draw(bitmap, cliprect, 0,0);
	}

	draw_sprites(screen.machine(), bitmap,cliprect);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

SCREEN_UPDATE_IND16(powerbls)
{
	sslam_state *state = screen.machine().driver_data<sslam_state>();

	if (!(state->m_regs[6] & 1))
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
		return 0;
	}

	state->m_bg_tilemap->set_scrollx(0, state->m_regs[0]+21);
	state->m_bg_tilemap->set_scrolly(0, state->m_regs[1]-240);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect);
	return 0;
}
