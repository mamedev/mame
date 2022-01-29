// license:BSD-3-Clause
// copyright-holders:Mark McDougall, David Haywood

// The Street Fight video appears to be 4 layers, very similar to Dark Mist and Air Raid, but at least without the CLUT transparency handling?
// which are presumably handled by the SEI0100BU chips on the other games (with the GFX inside the modules on Air Raid)

#include "emu.h"
#include "stfight_dev.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(STFIGHT_VIDEO, stfight_video_device, "stfight_vid", "Seibu Street Fight Video")


stfight_video_device::stfight_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, STFIGHT_VIDEO, tag, owner, clock),
	m_gfxdecode(*this, "gfxdecode"),
	m_palette(*this,"^palette"),
	m_screen(*this, "screen"),
	m_tx_clut(*this, "tx_clut"),
	m_fg_clut(*this, "fg_clut"),
	m_bg_clut(*this, "bg_clut"),
	m_spr_clut(*this, "spr_clut"),
	m_fgmap(*this,"fg_map"),
	m_bgmap(*this,"bg_map"),
	m_vregs(*this,"^vregs"),
	m_sprite_ram(*this, "^sprite_ram"),
	m_txram(*this, "^txram")
{
}

/* text-layer characters */
static const gfx_layout charlayout =
{
	8,8,        /* 8*8 pixels */
	512,        /* 512 characters */
	2,          /* 2 bits per pixel */
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16        /* every char takes 16 consecutive bytes */
};

/* foreground tiles */
static const gfx_layout fglayout =
{
	16,16,      /* 16*16 pixels */
	1024,       /* 1024 tiles */
	4,          /* 4 bits per pixel */
	{ 64*1024*8+0, 64*1024*8+4, 0, 4 },
	{      0,      1,       2,       3,
			8,      9,      10,      11,
		32*8+0, 32*8+1, 32*8+ 2, 32*8+ 3,
		32*8+8, 32*8+9, 32*8+10, 32*8+11 },
	{  0*8,  2*8,  4*8,  6*8,
		8*8, 10*8, 12*8, 14*8,
		16*8, 18*8, 20*8, 22*8,
		24*8, 26*8, 28*8, 30*8 },
	64*8        /* every char takes 64 consecutive bytes */
};

/*
 *      The background tiles are interleaved in banks of 2
 *      - so we need to create two separate layout structs
 *        to handle them properly with tilemaps
 */

/* background tiles */
static const gfx_layout bglayout =
{
	16,16,      /* 16*16 pixels */
	512,        /* 512 tiles */
	4,          /* 4 bits per pixel */
	{ 64*1024*8+4, 64*1024*8+0, 4, 0 },
	{      0,      1,       2,       3,
			8,      9,      10,      11,
		64*8+0, 64*8+1, 64*8+ 2, 64*8+ 3,
		64*8+8, 64*8+9, 64*8+10, 64*8+11 },
	{  0*8,  2*8,  4*8,  6*8,
		8*8, 10*8, 12*8, 14*8,
		16*8, 18*8, 20*8, 22*8,
		24*8, 26*8, 28*8, 30*8 },
	128*8       /* every tile takes 64/128 consecutive bytes */
};

/* sprites */
static const gfx_layout spritelayout =
{
	16,16,      /* 16*16 pixels */
	1024,       /* 1024 sprites */
	4,          /* 4 bits per pixel */
	{ 64*1024*8+0, 64*1024*8+4, 0, 4 },
	{      0,      1,       2,       3,
			8,      9,      10,      11,
		32*8+0, 32*8+1, 32*8+ 2, 32*8+ 3,
		32*8+8, 32*8+9, 32*8+10, 32*8+11 },
	{  0*8,  2*8,  4*8,  6*8,
		8*8, 10*8, 12*8, 14*8,
		16*8, 18*8, 20*8, 22*8,
		24*8, 26*8, 28*8, 30*8 },
	64*8        /* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( gfx_stfight )
	GFXDECODE_ENTRY( "tx_gfx", 0x0000,  charlayout,   0, 16 )
	GFXDECODE_ENTRY( "fg_gfx", 0x0000,  fglayout,     0, 16 )
	GFXDECODE_ENTRY( "bg_gfx", 0x0000,  bglayout,     0, 16 )
	GFXDECODE_ENTRY( "bg_gfx", 0x0020,  bglayout,     0, 16 )
	GFXDECODE_ENTRY( "spr_gfx", 0x0000, spritelayout, 0, 32 )
GFXDECODE_END

void stfight_video_device::device_add_mconfig(machine_config &config)
{
	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(stfight_video_device::screen_update_stfight));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_stfight);
}

/*
        Graphics ROM Format
        ===================

        Each tile is 8x8 pixels
        Each composite tile is 2x2 tiles, 16x16 pixels
        Each screen is 32x32 composite tiles, 64x64 tiles, 256x256 pixels
        Each layer is a 4-plane bitmap 8x16 screens, 2048x4096 pixels

        There are 4x256=1024 composite tiles defined for each layer

        Each layer is mapped using 2 bytes/composite tile
        - one byte for the tile
        - one byte for the tile bank, attribute
            - b7,b5     tile bank (0-3)

        Each pixel is 4 bits = 16 colours.

 */

TILEMAP_MAPPER_MEMBER(stfight_video_device::fg_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x70) << 4) + ((row & 0xf0) << 7);
}

TILE_GET_INFO_MEMBER(stfight_video_device::get_fg_tile_info)
{
	int attr,tile_base;

	attr = m_fgmap[0x8000+tile_index];
	tile_base = ((attr & 0x80) << 2) | ((attr & 0x20) << 3);

	tileinfo.set(1,
			tile_base + m_fgmap[tile_index],
			attr & 0x07,
			0);
}

TILEMAP_MAPPER_MEMBER(stfight_video_device::bg_scan)
{
	/* logical (col,row) -> memory offset */
	return ((col & 0x0e) >> 1) + ((row & 0x0f) << 3) + ((col & 0x70) << 3) +
			((row & 0x80) << 3) + ((row & 0x10) << 7) + ((col & 0x01) << 12) +
			((row & 0x60) << 8);
}

TILE_GET_INFO_MEMBER(stfight_video_device::get_bg_tile_info)
{
	int attr,tile_bank,tile_base;

	attr = m_bgmap[0x8000+tile_index];
	tile_bank = (attr & 0x20) >> 5;
	tile_base = (attr & 0x80) << 1;

	tileinfo.set(2+tile_bank,
			tile_base + m_bgmap[tile_index],
			attr & 0x07,
			0);
}

TILE_GET_INFO_MEMBER(stfight_video_device::get_tx_tile_info)
{
	uint8_t attr = m_txram[tile_index+0x400];
	int color = attr & 0x0f;

	tileinfo.group = color;

	tileinfo.set(0,
			m_txram[tile_index] + ((attr & 0x80) << 1),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0x60) >> 5));
}

void stfight_video_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, sx, sy;

	for (offs = 4096 - 32;offs >= 0;offs -= 32)
	{
		int code;
		int attr = m_sprite_ram[offs + 1];
		int flipx = attr & 0x10;
		int color = attr & 0x0f;
		color |= (attr & 0x20) >> 1; // mix in priority bit

		sy = m_sprite_ram[offs + 2];
		sx = m_sprite_ram[offs + 3];

		// non-active sprites have zero y coordinate value
		if (sy > 0)
		{
			// sprites which wrap onto/off the screen have
			// a sign extension bit in the sprite attribute
			if (sx >= 0xf0)
			{
				if (attr & 0x80)
					sx -= 0x100;
			}

			/*
			if (flip_screen())
			{
			    sx = 240 - sx;
			    sy = 240 - sy;
			    flipx = !flipx;
			}
			*/

			code = m_sprite_base + m_sprite_ram[offs];

			m_gfxdecode->gfx(4)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, 0/*flip_screen()*/,
				sx, sy,
				0x0f);
		}
	}
}


uint32_t stfight_video_device::screen_update_stfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);   /* in case m_bg_tilemap is disabled */

	m_temp_sprite_bitmap.fill(-1, cliprect);
	draw_sprites(screen, m_temp_sprite_bitmap, cliprect);

	if (m_bg_tilemap->enabled())
	{
		m_temp_bitmap.fill(-1, cliprect);
		m_bg_tilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
		mix_txlayer(screen, bitmap, m_temp_bitmap, cliprect, m_bg_clut, 0x00, 0x00, 0x00, false);
	}

	if (m_vregs[0x07] & 0x40) mix_txlayer(screen, bitmap, m_temp_sprite_bitmap, cliprect, m_spr_clut, 0x80, 0x100, 0x100, false); // low priority sprites

	if (m_fg_tilemap->enabled())
	{
		m_temp_bitmap.fill(-1, cliprect);
		m_fg_tilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
		mix_txlayer(screen, bitmap, m_temp_bitmap, cliprect, m_fg_clut, 0x40, 0x00, 0x00, false);
	}

	if (m_vregs[0x07] & 0x40) mix_txlayer(screen, bitmap, m_temp_sprite_bitmap, cliprect, m_spr_clut, 0x80, 0x100, 0x000, false); // high priority sprites

	if (m_tx_tilemap->enabled())
	{
		m_temp_bitmap.fill(-1, cliprect);
		m_tx_tilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
		mix_txlayer(screen, bitmap, m_temp_bitmap, cliprect, m_tx_clut, 0xc0, 0x00, 0x00, true);
	}
	//
	return 0;
}


void stfight_video_device::mix_txlayer(screen_device &screen, bitmap_ind16 &bitmap, bitmap_ind16 &bitmap2, const rectangle &cliprect, uint8_t* clut, int base, int mask, int condition, bool realcheck)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t *const dest = &bitmap.pix(y);
		uint16_t const *const src = &bitmap2.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (src[x] == -1)
				continue;

			if ((src[x] & mask) == condition)
			{
				uint8_t pix = src[x] & 0xff;
				uint8_t real = clut[pix];

				if (realcheck) // the text layer transparency appears to be 0xf *after* lookup
				{
					if ((real & 0x0f) != 0x0f)
					{
						dest[x] = (real & 0x3f) + base;
					}
				}
				else if ((src[x] & 0xf) != 0xf) // other layers it's pre-lookup
				{
					dest[x] = (real & 0x3f) + base;
				}
			}
		}
	}
}

void stfight_video_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	save_item(NAME(m_sprite_base));

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(stfight_video_device::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(stfight_video_device::bg_scan)), 16,16, 128,256);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(stfight_video_device::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(stfight_video_device::fg_scan)), 16,16, 128,256);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(stfight_video_device::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	// we do manual mixing using a temp bitmap
	m_screen->register_screen_bitmap(m_temp_sprite_bitmap);
	m_screen->register_screen_bitmap(m_temp_bitmap);
}

void stfight_video_device::device_reset()
{
}

// public functions

void stfight_video_device::stfight_text_char_w(offs_t offset, uint8_t data)
{
	m_txram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset&0x3ff);
}


void stfight_video_device::stfight_sprite_bank_w(uint8_t data)
{
	m_sprite_base = ( ( data & 0x04 ) << 7 ) |
							( ( data & 0x01 ) << 8 );
}

void stfight_video_device::stfight_vh_latch_w(offs_t offset, uint8_t data)
{
	int scroll;


	m_vregs[offset] = data;

	switch( offset )
	{
		case 0x00:
		case 0x01:
			scroll = (m_vregs[1] << 8) | m_vregs[0];
			m_fg_tilemap->set_scrollx(0,scroll);
			break;

		case 0x02:
		case 0x03:
			scroll = (m_vregs[3] << 8) | m_vregs[2];
			m_fg_tilemap->set_scrolly(0,scroll);
			break;

		case 0x04:
		case 0x05:
			scroll = (m_vregs[5] << 8) | m_vregs[4];
			m_bg_tilemap->set_scrollx(0,scroll);
			break;

		case 0x06:
		case 0x08:
			scroll = (m_vregs[8] << 8) | m_vregs[6];
			m_bg_tilemap->set_scrolly(0,scroll);
			break;

		case 0x07:
			m_tx_tilemap->enable(data & 0x80);
			/* 0x40 = sprites */
			m_bg_tilemap->enable(data & 0x20);
			m_fg_tilemap->enable(data & 0x10);
			//flip_screen_set(data & 0x01);
			break;
	}
}
