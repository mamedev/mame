// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Angelo Salese, hap, David Haywood

/* There are 2 versions of the Air Raid / Cross Shooter hardware, one has everything integrated on a single PCB
   the other is a Air Raid specific video PCB used with the Street Fight motherboard, there could be differences.

   This is very similar to Dark Mist */

#include "emu.h"
#include "airraid_dev.h"

DEFINE_DEVICE_TYPE(AIRRAID_VIDEO, airraid_video_device, "airraid_vid", "Seibu Air Raid Video")

airraid_video_device::airraid_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AIRRAID_VIDEO, tag, owner, clock),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "^palette"),
		m_screen(*this, "screen"),
		m_tx_clut(*this, "tx_clut"),
		m_fg_clut(*this, "fg_clut"),
		m_bg_clut(*this, "bg_clut"),
		m_spr_clut(*this, "spr_clut"),
		m_fgmap(*this, "fg_map"),
		m_bgmap(*this, "bg_map"),
		m_sprite_ram(*this, "^sprite_ram"),
		m_txram(*this,"^txram"),
		m_vregs(*this,"^vregs"),
		m_hw(0x09)
{
}

static const gfx_layout charlayout =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,1),      /* 512 characters */
	2,          /* 4 bits per pixel */
	{ 0,4 },
	{ 8,9,10,11,0,1,2,3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128*1
};

static const gfx_layout char16layout =
{
	16,16,        /* 8*8 characters */
	RGN_FRAC(1,1),      /* 512 characters */
	4,          /* 4 bits per pixel */
	{ 0,4,8,12 },
	{ 0,1,2,3, 16,17,18,19, 512+0,512+1,512+2,512+3, 512+16,512+17,512+18,512+19},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};

static GFXDECODE_START( gfx_cshooter )
	GFXDECODE_ENTRY( "tx_gfx", 0,     charlayout, 0, 16  )
	GFXDECODE_ENTRY( "spr_gfx", 0,     char16layout, 0, 16  )
	GFXDECODE_ENTRY( "bg_gfx", 0,     char16layout, 0, 16  )
	GFXDECODE_ENTRY( "fg_gfx", 0,     char16layout, 0, 16  )
GFXDECODE_END

void airraid_video_device::device_add_mconfig(machine_config &config)
{
	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 16, 256-1-16);
	m_screen->set_screen_update(FUNC(airraid_video_device::screen_update_airraid));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cshooter);
}


void airraid_video_device::device_start()
{
	if (!m_gfxdecode->started())
		throw device_missing_dependencies();

	save_item(NAME(m_hw));

	// there might actually be 4 banks of 2048 x 16 tilemaps in here as the upper scroll bits are with the rom banking.
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(airraid_video_device::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(airraid_video_device::bg_scan)), 16, 16, 2048, 64);

	// which could in turn mean this is actually 256 x 128, not 256 x 512
//  m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(airraid_video_device::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(airraid_video_device::fg_scan)), 16, 16, 256, 512);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(airraid_video_device::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(airraid_video_device::fg_scan)), 16, 16, 256, 128);

	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(airraid_video_device::get_cstx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

//  m_fg_tilemap->set_transparent_pen(0);
//  m_tx_tilemap->set_transparent_pen(0);

	// we do manual mixing using a temp bitmap
	m_screen->register_screen_bitmap(m_temp_bitmap);
}

void airraid_video_device::device_reset()
{
}

TILEMAP_MAPPER_MEMBER(airraid_video_device::bg_scan)
{
	return ((row&0xf) * 0x10) + (col&0xf) + (((col&0x7f0) >> 4)*0x100) + ((row & 0x30)>>4) * 0x8000;
}

TILEMAP_MAPPER_MEMBER(airraid_video_device::fg_scan)
{
	return ((row&0xf) * 0x10) + (col&0xf) + (((col&0x0f0) >> 4)*0x100) + ((row & 0x1f0)>>4) * 0x1000;
}


TILE_GET_INFO_MEMBER(airraid_video_device::get_bg_tile_info)
{
	int tile = m_bgmap[(tile_index*2)+1] & 0xff;
	int attr = m_bgmap[(tile_index*2)+0] & 0xff;

	tile |= (attr & 0x70) << 4;

	tileinfo.set(2,
			tile,
			attr&0xf,
			0);
}

TILE_GET_INFO_MEMBER(airraid_video_device::get_fg_tile_info)
{
	int tile = m_fgmap[(tile_index*2)+1] & 0xff;
	int attr = m_fgmap[(tile_index*2)+0] & 0xff;

	tile |= (attr & 0x70) << 4;

	tileinfo.set(3,
			tile,
			attr&0xf,
			0);
}

TILE_GET_INFO_MEMBER(airraid_video_device::get_cstx_tile_info)
{
	int code = (m_txram[tile_index*2]);
	int attr = (m_txram[tile_index*2+1]);
	int color = attr & 0xf;

	tileinfo.set(0, (code << 1) | ((attr & 0x20) >> 5), color, 0);
}



void airraid_video_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0x200 - 4; i >= 0 ; i -= 4)
	{
		if (m_sprite_ram[i+1]&0x80)
			continue;

		uint16_t tile = (m_sprite_ram[i]);
		tile |= (m_sprite_ram[i + 1] & 0x70) << 4;

		uint16_t col = (m_sprite_ram[i+1] & 0x0f);
		//col |= (m_sprite_ram[i+1] & 0x80)<<3;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, tile,col, 0, 0, m_sprite_ram[i+3],m_sprite_ram[i+2],0);
	}
}


#define DISPLAY_SPR     1
#define DISPLAY_FG      2
#define DISPLAY_BG      4
#define DISPLAY_TXT     8
#define DM_GETSCROLL(n) (((m_vregs[(n)]<<1)&0xff) + ((m_vregs[(n)]&0x80)?1:0) +( ((m_vregs[(n)-1]<<4) | (m_vregs[(n)-1]<<12) )&0xff00))

void airraid_video_device::mix_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* clut, int base)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t *const dest = &bitmap.pix(y);
		uint16_t const *const src = &m_temp_bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint8_t pix = src[x] & 0xff;
			uint8_t real = clut[pix];

			if (!(real & 0x40))
			{
				dest[x] = (real & 0x3f) + base;
			}
		}
	}
}


uint32_t airraid_video_device::screen_update_airraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t bgscrolly = DM_GETSCROLL(0x6);
	// this is more likely to be 'bank' than scroll, like NMK16
	bgscrolly += ((m_hw & 0xc0) >> 6) * 256;

	m_bg_tilemap->set_scrollx(0, DM_GETSCROLL(0x2));
	m_bg_tilemap->set_scrolly(0, bgscrolly);
	m_fg_tilemap->set_scrollx(0, DM_GETSCROLL(0xa));
	m_fg_tilemap->set_scrolly(0, DM_GETSCROLL(0xe));

	// draw screen
	bitmap.fill(0x80, cliprect); // temp

//  m_temp_bitmap.fill(0x00, cliprect);

	if ((m_hw & DISPLAY_BG) == 0x00)
	{
		m_bg_tilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
		mix_layer(screen, bitmap, cliprect, m_bg_clut, 0x80);
	}

	if ((m_hw & DISPLAY_FG) == 0x00)
	{
		m_fg_tilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
		mix_layer(screen, bitmap, cliprect, m_fg_clut, 0x00);
	}

	if (m_hw & DISPLAY_SPR)
	{
		m_temp_bitmap.fill(0x00, cliprect);
		draw_sprites(m_temp_bitmap, cliprect); // technically this should draw manually because 0x40 in the prom is transparency and our code just assumes it to be 0.
		mix_layer(screen, bitmap, cliprect, m_spr_clut, 0x40);
	}

	if (m_hw & DISPLAY_TXT)
	{
		m_tx_tilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
		mix_layer(screen, bitmap, cliprect, m_tx_clut, 0xc0);
	}

	return 0;
}

// public functions

void airraid_video_device::txram_w(offs_t offset, uint8_t data)
{
	m_txram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset/2);
}

void airraid_video_device::vregs_w(offs_t offset, uint8_t data)
{
	m_vregs[offset] = data;

	if ((offset != 0x2) && (offset != 0x01) && (offset != 0xa) && (offset != 0x09)   && (offset != 0xe) && (offset != 0x0d)  )
		logerror("vregs_w %02x: %02x\n", offset, data);
}

void airraid_video_device::layer_enable_w(uint8_t enable)
{
	m_hw = enable;
}
