// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Todo:
        There are some kind of resistors hooked up to the background colours,
        the same prom colours can change for the background but not the
        foreground.  It's rarely used (Liberation title screen only?).

    Emulation by Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "cpu/m6502/deco16.h"
#include "cpu/m6502/m6502.h"
#include "includes/liberate.h"

#if 0
void liberate_state::debug_print(bitmap_ind16 &bitmap)
{
	int i, j;
	char buf[20 * 16];
	char *bufptr = buf;
	for (i = 0; i < 16; i += 2)
		bufptr += sprintf(bufptr, "%04X", deco16_io_ram[i + 1] | (deco16_io_ram[i] << 8));
	ui_draw_text(buf, 10, 6 * 6);
}
#endif

TILEMAP_MAPPER_MEMBER(liberate_state::back_scan)
{
	/* logical (col,row) -> memory offset */
	return ((row & 0xf)) + ((15 - (col & 0xf)) << 4) + ((row & 0x10) << 5) + ((col & 0x10) << 4);
}

TILEMAP_MAPPER_MEMBER(liberate_state::fix_scan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x1f) + ((31 - (col & 0x1f)) << 5);
}

TILE_GET_INFO_MEMBER(liberate_state::get_back_tile_info)
{
	const UINT8 *RAM = memregion("user1")->base();
	int tile, bank;

	/* Convert tile index of 512x512 to paged format */
	if (tile_index & 0x100)
	{
		if (tile_index & 0x200)
			tile_index = (tile_index & 0xff) + (m_io_ram[5] << 8); /* Bottom right */
		else
			tile_index = (tile_index & 0xff) + (m_io_ram[4] << 8); /* Bottom left */
	}
	else
	{
		if (tile_index & 0x200)
			tile_index = (tile_index & 0xff) + (m_io_ram[3] << 8); /* Top right */
		else
			tile_index = (tile_index & 0xff) + (m_io_ram[2] << 8); /* Top left */
	}

	tile = RAM[tile_index];
	if (tile > 0x7f)
		bank = 3;
	else
		bank = 2;
	SET_TILE_INFO_MEMBER(bank, tile & 0x7f, m_background_color, 0);
}

TILE_GET_INFO_MEMBER(liberate_state::get_fix_tile_info)
{
	UINT8 *videoram = m_videoram;
	UINT8 *colorram = m_colorram;
	int tile, color;

	tile = videoram[tile_index] + (colorram[tile_index] << 8);
	color = (colorram[tile_index] & 0x70) >> 4;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(liberate_state::prosport_get_back_tile_info)
{
	int tile;

	/*
	    robiza notes:
	    - flip y (handled with a +0x10 tile banking) depends only by position of the tile in the screen
	    - bits 0-3 are not used by gfx hardware; the value is the color of the pixel in the map (golf)
	*/

	tile = (m_bg_vram[tile_index] & 0xf0)>>4;

	if (tile_index & 0x8) tile += 0x10;

	tile += m_io_ram[0]&0x20; //Pro Bowling bg tiles banking bit

	SET_TILE_INFO_MEMBER(8, tile, 0, 0);
}

/***************************************************************************/

WRITE8_MEMBER(liberate_state::deco16_io_w)
{
	m_io_ram[offset] = data;
	if (offset > 1 && offset < 6)
		m_back_tilemap->mark_all_dirty();

	switch (offset)
	{
		case 6: /* Background colour */
			if (((data >> 4) & 3) != m_background_color)
			{
				m_background_color = (data >> 4) & 3;
				m_back_tilemap->mark_all_dirty();
			}
			m_background_disable = data & 0x4;
			flip_screen_set(data & 0x01);
			break;
		case 7: /* Background palette resistors? */
			/* Todo */
			break;
		case 8: /* Irq ack */
			m_maincpu->set_input_line(DECO16_IRQ_LINE, CLEAR_LINE);
			break;
		case 9: /* Sound */
			soundlatch_byte_w(space, 0, data);
			m_audiocpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			break;
	}
}

WRITE8_MEMBER(liberate_state::prosoccr_io_w)
{
	m_io_ram[offset] = data;
	if (offset > 1 && offset < 6)
		m_back_tilemap->mark_all_dirty();

	//  popmessage("%02x %02x",m_io_ram[6],m_io_ram[7]);

	switch (offset)
	{
		case 6: /* unused here */
			break;
		case 7:
			m_background_disable = ~data & 0x10;
			//sprite_priority = (data & 0x80)>>7;
			/* -x-- --xx used during gameplay */
			/* x--- ---- used on the attract mode */
			break;
		case 8: /* Irq ack */
			m_maincpu->set_input_line(DECO16_IRQ_LINE, CLEAR_LINE);
			break;
		case 9: /* Sound */
			soundlatch_byte_w(space, 0, data);
			m_audiocpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			break;
	}
}

/* completely different i/o...*/
WRITE8_MEMBER(liberate_state::prosport_io_w)
{
	m_io_ram[offset] = data;

	switch (offset)
	{
		case 0:
			//background_disable = ~data & 0x80;
			flip_screen_set(data & 0x80);
			m_back_tilemap->mark_all_dirty();
			break;
		case 2: /* Sound */
			soundlatch_byte_w(space, 0, data);
			m_audiocpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			break;
		case 4: /* Irq ack */
			m_maincpu->set_input_line(DECO16_IRQ_LINE, CLEAR_LINE);
			break;
	}
}

WRITE8_MEMBER(liberate_state::liberate_videoram_w)
{
	m_videoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(liberate_state::liberate_colorram_w)
{
	m_colorram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(liberate_state::prosport_bg_vram_w)
{
	m_bg_vram[offset] = data;
	m_back_tilemap->mark_tile_dirty(offset);
}

/***************************************************************************/

VIDEO_START_MEMBER(liberate_state,prosoccr)
{
	m_back_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(liberate_state::get_back_tile_info),this), tilemap_mapper_delegate(FUNC(liberate_state::back_scan),this), 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(liberate_state::get_fix_tile_info),this), tilemap_mapper_delegate(FUNC(liberate_state::fix_scan),this), 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);

	m_fg_gfx = memregion("fg_gfx")->base();
	m_charram = auto_alloc_array(machine(), UINT8, 0x1800 * 2);

	save_pointer(NAME(m_charram), 0x1800 * 2);
	save_pointer(NAME(m_fg_gfx), 0x6000);
}

VIDEO_START_MEMBER(liberate_state,boomrang)
{
	m_back_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(liberate_state::get_back_tile_info),this), tilemap_mapper_delegate(FUNC(liberate_state::back_scan),this), 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(liberate_state::get_fix_tile_info),this), tilemap_mapper_delegate(FUNC(liberate_state::fix_scan),this), 8, 8, 32, 32);

	m_back_tilemap->set_transmask(0, 0x0001, 0x007e); /* Bottom 1 pen/Top 7 pens */
	m_fix_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(liberate_state,liberate)
{
	m_back_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(liberate_state::get_back_tile_info),this), tilemap_mapper_delegate(FUNC(liberate_state::back_scan),this), 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(liberate_state::get_fix_tile_info),this), tilemap_mapper_delegate(FUNC(liberate_state::fix_scan),this), 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(liberate_state,prosport)
{
	m_back_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(liberate_state::prosport_get_back_tile_info),this), tilemap_mapper_delegate(FUNC(liberate_state::back_scan),this), 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(liberate_state::get_fix_tile_info),this), tilemap_mapper_delegate(FUNC(liberate_state::fix_scan),this), 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
}

/***************************************************************************/

PALETTE_INIT_MEMBER(liberate_state,liberate)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i, bit0, bit1, bit2, g, r, b;

	for (i = 0;i < 32;i++)
	{
		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		color_prom++;
		palette.set_pen_color(i,rgb_t(r,g,b));
	}
	palette.set_pen_color(32,rgb_t(0,0,0)); /* Allocate black for when no background is displayed */
}

/***************************************************************************/

void liberate_state::liberate_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	/* Sprites */
	for (offs = 0x000; offs < 0x800; offs += 4)
	{
		int multi, fx, fy, sx, sy, sy2, code, color;

		/*
		    Byte 0: 0x01 - ?
		            0x02 - Y flip
		            0x04 - X flip
		            0x08 - Colour?
		            0x10 - Multi sprite set
		            0x60 - Tile (high bits)
		            0x80 - ?
		    Byte 1: 0xff - Tile (low bits)
		    Byte 2: 0xff - Y position
		    Byte 3: 0xff - X position
		*/

		code = spriteram[offs + 1] + ((spriteram[offs + 0] & 0x60) << 3);
		sx = 240 - spriteram[offs + 3];
		sy = 240 - spriteram[offs + 2];
		color = ((spriteram[offs + 1] & 0x08) >> 3); // ?

		fx = spriteram[offs + 0] & 0x04;
		fy = spriteram[offs + 0] & 0x02;
		multi = spriteram[offs + 0] & 0x10;

		if (multi && fy == 0)
			sy -= 16;

		if (flip_screen())
		{
			sy = 240 - sy;
			sx = 240 - sx;
			if (fy)
				sy2 = sy + 16;
			else
				sy2 = sy - 16;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
		}
		else
		{
			if (fy)
				sy2 = sy - 16;
			else
				sy2 = sy + 16;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code,
					color,
					fx,fy,
					sx,sy,0);
		if (multi)
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code+1,
					color,
					fx,fy,
					sx,sy2,0);
	}
}

void liberate_state::prosport_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs, multi, fx, fy, sx, sy, sy2, code, code2, color, gfx_region;
	UINT8 *spriteram = m_spriteram;

	for (offs = 0x000; offs < 0x800; offs += 4)
	{
		if ((spriteram[offs + 0] & 1) != 1)
			continue;

		code = spriteram[offs + 1] + ((spriteram[offs + 0] & 0x3) << 8);
		code2 = code + 1;

		if(m_io_ram[0] & 0x40) //dynamic ram-based gfxs for Pro Golf
			gfx_region = 3 + 4;
		else
			gfx_region = ((m_io_ram[0] & 0x30) >> 4) + 4;


		multi = spriteram[offs + 0] & 0x10;

		sy = spriteram[offs + 2];
		if (multi)
			sy += 16;
		sx = (240 - spriteram[offs + 3]);
//      sy = (240 - spriteram[offs + 2]);//-16;
		sy = 240 - sy;

		color = 1;//(m_io_ram[4] & 2) + 1;//(spriteram[offs + 0] & 0x4) >> 2;

		fy = spriteram[offs + 0] & 0x02;
		fx = spriteram[offs + 0] & 0x04;
		multi = 0;// spriteram[offs + 0] & 0x10;

//      if (multi) sy -= 16;
		if ((fy && multi) || (fx && multi)) { code2 = code; code++; }

		if (flip_screen())
		{
			sy = 240 - sy;
			sx = 240 - sx;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			sy2 = sy - 16;
		}
		else
		{
			sy2 = sy + 16;
		}

		m_gfxdecode->gfx(gfx_region)->transpen(bitmap,cliprect,
				code,
				color,
				fx,fy,
				sx,sy,0);
		if (multi)
			m_gfxdecode->gfx(gfx_region)->transpen(bitmap,cliprect,
				code2,
				color,
				fx,fy,
				sx,sy2,0);
	}
}

void liberate_state::boomrang_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	UINT8 *spriteram = m_spriteram;
	int offs, multi, fx, fy, sx, sy, sy2, code, code2, color;

	for (offs = 0x000; offs < 0x800; offs += 4)
	{
		if ((spriteram[offs + 0] & 1) != 1)
			continue;
		if ((spriteram[offs + 0] & 0x8) != pri)
			continue;

		code = spriteram[offs + 1] + ((spriteram[offs + 0] & 0xe0) << 3);
		code2 = code + 1;

		multi = spriteram[offs + 0] & 0x10;

		sy=spriteram[offs + 2];
		if (multi)
			sy += 16;
		sx = (240 - spriteram[offs + 3]);
//      sy = (240-spriteram[offs + 2]);//-16;
		sy = 240 - sy;

		color = (spriteram[offs + 0] & 0x4) >> 2;

		fx = 0;
		fy = spriteram[offs + 0] & 0x02;
		multi = spriteram[offs + 0] & 0x10;

//      if (multi) sy -= 16;
		if (fy && multi) { code2 = code; code++; }

		if (flip_screen())
		{
			sy = 240 - sy;
			sx = 240 - sx;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			sy2 = sy - 16;
		}
		else
		{
			sy2 = sy + 16;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				fx,fy,
				sx,sy,0);
		if (multi)
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code2,
				color,
				fx,fy,
				sx,sy2,0);
	}
}

void liberate_state::prosoccr_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs, code, fx, fy, sx, sy;

	for (offs = 0x000; offs < 0x400; offs += 4)
	{
		if ((spriteram[offs + 0] & 1) != 1)
			continue;

		code = spriteram[offs + 1];
		sy = 240 - spriteram[offs + 2];
		sx = 240 - spriteram[offs + 3];
		fx = spriteram[offs + 0] & 4;
		fy = spriteram[offs + 0] & 2;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				0,
				fx,fy,
				sx,sy,0);
	}
}

/***************************************************************************/

UINT32 liberate_state::screen_update_prosoccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_back_tilemap->set_scrolly(0,  m_io_ram[1]);
	m_back_tilemap->set_scrollx(0, -m_io_ram[0]);

	if (m_background_disable)
		bitmap.fill(32, cliprect);
	else
		m_back_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	prosoccr_draw_sprites(bitmap, cliprect);

	return 0;
}

UINT32 liberate_state::screen_update_prosport(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	UINT8 *colorram = m_colorram;
	int mx, my, tile, offs, gfx_region;
	int scrollx, scrolly;

	bitmap.fill(0, cliprect);

	offs = 0;
	/* TODO: what's bits 0 and 2 for? Internal scrolling state? */
	scrolly = ((m_io_ram[0] & 0x8) << 5);
	scrollx = ((m_io_ram[0] & 0x2) << 7) | (m_io_ram[1]);

	m_back_tilemap->set_scrolly(0, scrolly);
	m_back_tilemap->set_scrollx(0, -scrollx);

	m_back_tilemap->draw(screen, bitmap, cliprect, 0, 0);

//  popmessage("%d %02x %02x %02x %02x %02x %02x %02x %02x",scrollx,deco16_io_ram[0],deco16_io_ram[1],deco16_io_ram[2],deco16_io_ram[3]
//  ,deco16_io_ram[4],deco16_io_ram[5],deco16_io_ram[6],deco16_io_ram[7]);

	for (offs = 0; offs < 0x400; offs++)
	{
		tile = videoram[offs] + ((colorram[offs] & 0x3) << 8);

		if(m_io_ram[0] & 0x40) //dynamic ram-based gfxs for Pro Golf
			gfx_region = 3;
		else
			gfx_region = ((m_io_ram[0] & 0x30) >> 4);

		my = (offs) % 32;
		mx = (offs) / 32;

		m_gfxdecode->gfx(gfx_region)->transpen(bitmap,cliprect,
				tile, 1, 0, 0, 248 - 8 * mx, 8 * my, 0);
	}

	prosport_draw_sprites(bitmap, cliprect);

	return 0;
}

UINT32 liberate_state::screen_update_boomrang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_back_tilemap->set_scrolly(0,  m_io_ram[1]);
	m_back_tilemap->set_scrollx(0, -m_io_ram[0]);

	if (m_background_disable)
		bitmap.fill(32, cliprect);
	else
		m_back_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);

	boomrang_draw_sprites(bitmap,cliprect,8);
	if (!m_background_disable)
		m_back_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);

	boomrang_draw_sprites(bitmap, cliprect, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 liberate_state::screen_update_liberate(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_back_tilemap->set_scrolly(0,  m_io_ram[1]);
	m_back_tilemap->set_scrollx(0, -m_io_ram[0]);

	if (m_background_disable)
		bitmap.fill(32, cliprect);
	else
		m_back_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	liberate_draw_sprites(bitmap, cliprect);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
