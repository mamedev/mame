// license:???
// copyright-holders:Marc Lafontaine, Couriersud
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/popeye.h"

static const size_t popeye_bitmapram_size = 0x2000;

enum { TYPE_SKYSKIPR, TYPE_POPEYE };

#define USE_NEW_COLOR (1)

// Only enable USE_INTERLACE if you can ensure the game is rendered at an
// integer multiple of it's original resolution
#define USE_INTERLACE (0)


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Popeye has four color PROMS:
  - 32x8 char palette
  - 32x8 background palette
  - two 256x4 sprite palette

  The char and sprite PROMs are connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE (inverted)
        -- 470 ohm resistor  -- BLUE (inverted)
        -- 220 ohm resistor  -- GREEN (inverted)
        -- 470 ohm resistor  -- GREEN (inverted)
        -- 1  kohm resistor  -- GREEN (inverted)
        -- 220 ohm resistor  -- RED (inverted)
        -- 470 ohm resistor  -- RED (inverted)
  bit 0 -- 1  kohm resistor  -- RED (inverted)

  The background PROM is connected to the RGB output this way:

  bit 7 -- 470 ohm resistor  -- BLUE (inverted)
        -- 680 ohm resistor  -- BLUE (inverted)  (1300 ohm in Sky Skipper)
        -- 470 ohm resistor  -- GREEN (inverted)
        -- 680 ohm resistor  -- GREEN (inverted)
        -- 1.2kohm resistor  -- GREEN (inverted)
        -- 470 ohm resistor  -- RED (inverted)
        -- 680 ohm resistor  -- RED (inverted)
  bit 0 -- 1.2kohm resistor  -- RED (inverted)

  The bootleg is the same, but the outputs are not inverted.

***************************************************************************/

static const res_net_decode_info popeye_7051_decode_info =
{
	1,      /*  one prom 5 lines */
	0,      /*  start at 0 */
	15,     /*  end at 15 (banked) */
	/*  R,   G,   B,  */
	{   0,   0,   0 },      /*  offsets */
	{   0,   3,   6 },      /*  shifts */
	{0x07,0x07,0x03 }           /*  masks */
};

static const res_net_decode_info popeye_7052_decode_info =
{
	2,      /*  there may be two proms needed to construct color */
	0,      /*  start at 0 */
	255,    /*  end at 255 */
	/*  R,   G,   B,   R,   G,   B */
	{   0,   0,   0, 256, 256, 256},        /*  offsets */
	{   0,   3,   0,   0,  -1,   2},        /*  shifts */
	{0x07,0x01,0x00,0x00,0x06,0x03}         /*  masks */
};

static const res_net_info popeye_7051_txt_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7051 | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 680, 0, 2, {  470, 220,   0 } }  /*  popeye */
	}
};

static const res_net_info popeye_7051_bck_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7051 | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1200, 680, 470 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1200, 680, 470 } },
		{ RES_NET_AMP_DARLINGTON, 680, 0, 2, {  680, 470,   0 } }  /*  popeye */
	}
};


static const res_net_info popeye_7052_obj_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 680, 0, 2, {  470, 220,   0 } }  /*  popeye */
	}
};


void popeye_state::convert_color_prom(const UINT8 *color_prom)
{
	int i;


	/* palette entries 0-15 are directly used by the background and changed at runtime */
	color_prom += 32;

	/* characters */
#if USE_NEW_COLOR
	for (i = 0; i < 16; i++)
	{
		int prom_offs = i | ((i & 8) << 1); /* address bits 3 and 4 are tied together */
		int r, g, b;
		r = compute_res_net(((color_prom[prom_offs] ^ m_invertmask) >> 0) & 0x07, 0, popeye_7051_txt_net_info);
		g = compute_res_net(((color_prom[prom_offs] ^ m_invertmask) >> 3) & 0x07, 1, popeye_7051_txt_net_info);
		b = compute_res_net(((color_prom[prom_offs] ^ m_invertmask) >> 6) & 0x03, 2, popeye_7051_txt_net_info);
		m_palette->set_pen_color(16 + (2 * i) + 0,rgb_t(0,0,0));
		m_palette->set_pen_color(16 + (2 * i) + 1,rgb_t(r,g,b));
	}

#else
	for (i = 0;i < 16;i++)
	{
		int prom_offs = i | ((i & 8) << 1); /* address bits 3 and 4 are tied together */
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = ((color_prom[prom_offs] ^ m_invertmask) >> 0) & 0x01;
		bit1 = ((color_prom[prom_offs] ^ m_invertmask) >> 1) & 0x01;
		bit2 = ((color_prom[prom_offs] ^ m_invertmask) >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = ((color_prom[prom_offs] ^ m_invertmask) >> 3) & 0x01;
		bit1 = ((color_prom[prom_offs] ^ m_invertmask) >> 4) & 0x01;
		bit2 = ((color_prom[prom_offs] ^ m_invertmask) >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = ((color_prom[prom_offs] ^ m_invertmask) >> 6) & 0x01;
		bit2 = ((color_prom[prom_offs] ^ m_invertmask) >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		m_palette->set_pen_color(16 + (2 * i) + 1,rgb_t(r,g,b));
	}
#endif
	color_prom += 32;

#if USE_NEW_COLOR
	/* sprites */
	std::vector<rgb_t> rgb;
	UINT8 cpi[512];

	for (i=0; i<512; i++)
		cpi[i] = color_prom[i] ^ m_invertmask;

	compute_res_net_all(rgb, &cpi[0], popeye_7052_decode_info, popeye_7052_obj_net_info);
	m_palette->set_pen_colors(48, rgb);
#else
	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = ((color_prom[0] ^ m_invertmask) >> 0) & 0x01;
		bit1 = ((color_prom[0] ^ m_invertmask) >> 1) & 0x01;
		bit2 = ((color_prom[0] ^ m_invertmask) >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = ((color_prom[0] ^ m_invertmask) >> 3) & 0x01;
		bit1 = ((color_prom[256] ^ m_invertmask) >> 0) & 0x01;
		bit2 = ((color_prom[256] ^ m_invertmask) >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = ((color_prom[256] ^ m_invertmask) >> 2) & 0x01;
		bit2 = ((color_prom[256] ^ m_invertmask) >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		m_palette->set_pen_color(48+i,rgb_t(r,g,b));

		color_prom++;
	}
#endif
}

PALETTE_INIT_MEMBER(popeye_state, popeye)
{
	const UINT8 *color_prom = memregion("proms")->base();
	m_invertmask = (USE_NEW_COLOR) ? 0x00 : 0xff;

	convert_color_prom(color_prom);
}

PALETTE_INIT_MEMBER(popeye_state,popeyebl)
{
	const UINT8 *color_prom = memregion("proms")->base();
	m_invertmask = (USE_NEW_COLOR) ? 0xff : 0x00;

	convert_color_prom(color_prom);
}

void popeye_state::set_background_palette(int bank)
{
	int i;
	UINT8 *color_prom = memregion("proms")->base() + 16 * bank;

#if USE_NEW_COLOR
	UINT8 cpi[16];
	std::vector<rgb_t> rgb;
	for (i=0; i<16; i++)
		cpi[i] = color_prom[i] ^ m_invertmask;

	compute_res_net_all(rgb, &cpi[0], popeye_7051_decode_info, popeye_7051_bck_net_info);
	m_palette->set_pen_colors(0, rgb);

#else
	for (i = 0;i < 16;i++)
	{
		int bit0,bit1,bit2;
		int r,g,b;

		/* red component */
		bit0 = ((color_prom[0] ^ m_invertmask) >> 0) & 0x01;
		bit1 = ((color_prom[0] ^ m_invertmask) >> 1) & 0x01;
		bit2 = ((color_prom[0] ^ m_invertmask) >> 2) & 0x01;
		r = 0x1c * bit0 + 0x31 * bit1 + 0x47 * bit2;
		/* green component */
		bit0 = ((color_prom[0] ^ m_invertmask) >> 3) & 0x01;
		bit1 = ((color_prom[0] ^ m_invertmask) >> 4) & 0x01;
		bit2 = ((color_prom[0] ^ m_invertmask) >> 5) & 0x01;
		g = 0x1c * bit0 + 0x31 * bit1 + 0x47 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = ((color_prom[0] ^ m_invertmask) >> 6) & 0x01;
		bit2 = ((color_prom[0] ^ m_invertmask) >> 7) & 0x01;
		if (m_bitmap_type == TYPE_SKYSKIPR)
		{
			/* Sky Skipper has different weights */
			bit0 = bit1;
			bit1 = 0;
		}
		b = 0x1c * bit0 + 0x31 * bit1 + 0x47 * bit2;

		m_palette->set_pen_color(i,rgb_t(r,g,b));

		color_prom++;
	}
#endif
}

WRITE8_MEMBER(popeye_state::popeye_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(popeye_state::popeye_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(popeye_state::popeye_bitmap_w)
{
	int sx,sy,x,y,colour;

	m_bitmapram[offset] = data;

	if (m_bitmap_type == TYPE_SKYSKIPR)
	{
		sx = 8 * (offset % 128);
		sy = 8 * (offset / 128);

		if (flip_screen())
			sy = 512-8 - sy;

		colour = data & 0x0f;
		for (y = 0; y < 8; y++)
		{
			for (x = 0; x < 8; x++)
			{
				m_tmpbitmap2->pix16(sy+y, sx+x) = colour;
			}
		}
	}
	else
	{
		sx = 8 * (offset % 64);
		sy = 4 * (offset / 64);

		if (flip_screen())
			sy = 512-4 - sy;

		colour = data & 0x0f;
		for (y = 0; y < 4; y++)
		{
			for (x = 0; x < 8; x++)
			{
				m_tmpbitmap2->pix16(sy+y, sx+x) = colour;
			}
		}
	}
}

WRITE8_MEMBER(popeye_state::skyskipr_bitmap_w)
{
	offset = ((offset & 0xfc0) << 1) | (offset & 0x03f);
	if (data & 0x80)
		offset |= 0x40;

	popeye_bitmap_w(space,offset,data);
}

TILE_GET_INFO_MEMBER(popeye_state::get_fg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index] & 0x0f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void popeye_state::video_start()
{
	m_bitmapram = std::make_unique<UINT8[]>(popeye_bitmapram_size);
	m_tmpbitmap2 = std::make_unique<bitmap_ind16>(1024,1024);    /* actually 1024x512 but not rolling over vertically? */

	m_bitmap_type = TYPE_SKYSKIPR;

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(popeye_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_lastflip = 0;
	m_field = 0;

	save_item(NAME(m_field));
	save_item(NAME(m_lastflip));
	save_item(NAME(*m_tmpbitmap2));
	save_pointer(NAME(m_bitmapram.get()), popeye_bitmapram_size);
}

VIDEO_START_MEMBER(popeye_state,popeye)
{
	m_bitmapram = std::make_unique<UINT8[]>(popeye_bitmapram_size);
	m_tmpbitmap2 = std::make_unique<bitmap_ind16>(512,512);

	m_bitmap_type = TYPE_POPEYE;

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(popeye_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_lastflip = 0;
	m_field = 0;

	save_item(NAME(m_field));
	save_item(NAME(m_lastflip));
	save_item(NAME(*m_tmpbitmap2));
	save_pointer(NAME(m_bitmapram.get()), popeye_bitmapram_size);
}

void popeye_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (m_lastflip != flip_screen())
	{
		for (offs = 0;offs < popeye_bitmapram_size;offs++)
			popeye_bitmap_w(space,offs,m_bitmapram[offs]);

		m_lastflip = flip_screen();
	}

	set_background_palette((*m_palettebank & 0x08) >> 3);

	if (m_background_pos[1] == 0)    /* no background */
		bitmap.fill(0, cliprect);
	else
	{
		/* copy the background graphics */
		int scrollx = 200 - m_background_pos[0] - 256*(m_background_pos[2]&1); /* ??? */
		int scrolly = 2 * (256 - m_background_pos[1]);

		if (m_bitmap_type == TYPE_SKYSKIPR)
			scrollx = 2*scrollx - 512;

		if (flip_screen())
		{
			if (m_bitmap_type == TYPE_POPEYE)
				scrollx = -scrollx;
			scrolly = -scrolly;
		}

		copyscrollbitmap(bitmap,*m_tmpbitmap2,1,&scrollx,1,&scrolly,cliprect);
	}
}

void popeye_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = 0;offs < m_spriteram.bytes();offs += 4)
	{
		int code,color,flipx,flipy,sx,sy;

		/*
		 * offs+3:
		 * bit 7 ?
		 * bit 6 ?
		 * bit 5 ?
		 * bit 4 MSB of sprite code
		 * bit 3 vertical flip
		 * bit 2 sprite bank
		 * bit 1 \ color (with bit 2 as well)
		 * bit 0 /
		 */

		color = (spriteram[offs + 3] & 0x07);
		if (color == 0) continue;

		code = (spriteram[offs + 2] & 0x7f)
			+ ((spriteram[offs + 3] & 0x10) << 3)
			+ ((spriteram[offs + 3] & 0x04) << 6);

		color += (*m_palettebank & 0x07) << 3;
		if (m_bitmap_type == TYPE_SKYSKIPR)
		{
			/* Two of the PROM address pins are tied together and one is not connected... */
			color = (color & 0x0f) | ((color & 0x08) << 1);
		}

		flipx = spriteram[offs + 2] & 0x80;
		flipy = spriteram[offs + 3] & 0x08;

		sx = 2*(spriteram[offs])-8;
		sy = 2*(256-spriteram[offs + 1]);

		if (flip_screen())
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = 496 - sx;
			sy = 496 - sy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code ^ 0x1ff,
					color,
					flipx,flipy,
					sx,sy,0);
	}
}

void popeye_state::draw_field(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x;
	int y;

	for (y=(cliprect.min_y & ~1) + m_field; y<=cliprect.max_y; y += 2)
		for (x=cliprect.min_x; x<=cliprect.max_x; x++)
			bitmap.pix(y, x) = 0;
}

UINT32 popeye_state::screen_update_popeye(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
#if USE_INTERLACE
	draw_field(bitmap, cliprect);
#endif
	return 0;
}
