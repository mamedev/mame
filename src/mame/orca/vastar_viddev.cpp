// license:BSD-3-Clause
// copyright-holders: Allard van der Bas

// Orca video system, implemented on ORCA OVG-46C PCB

#include "emu.h"
#include "vastar_viddev.h"

#include "emupal.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(VASTAR_VIDEO_DEVICE, vastar_video_device, "vastar_viddev", "Orca Vastar Video Device")

vastar_video_device::vastar_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VASTAR_VIDEO_DEVICE, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, nullptr, "palette")
	, device_video_interface(mconfig, *this)
	, m_bgvideoram(*this, { finder_base::DUMMY_TAG, finder_base::DUMMY_TAG })
	, m_fgvideoram(*this, finder_base::DUMMY_TAG)
	, m_fg_vregs(0)
	, m_flip_screen(0)
{
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static const gfx_layout spritelayoutdw =
{
	16,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			96*8, 97*8, 98*8, 99*8, 100*8, 101*8, 102*8, 103*8 },
	128*8
};

GFXDECODE_MEMBER(vastar_video_device::gfxinfo)
	GFXDECODE_ENTRY( "fgtiles",  0, charlayout,     0, 64 )
	GFXDECODE_ENTRY( "sprites",  0, spritelayout,   0, 64 )
	GFXDECODE_ENTRY( "sprites",  0, spritelayoutdw, 0, 64 )
	GFXDECODE_ENTRY( "bgtiles0", 0, charlayout,     0, 64 )
	GFXDECODE_ENTRY( "bgtiles1", 0, charlayout,     0, 64 )
GFXDECODE_END


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void vastar_video_device::device_start()
{
	decode_gfx(gfxinfo);

	m_fg_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(vastar_video_device::get_fg_tile_info)),  TILEMAP_SCAN_ROWS, 8,8, 32,32);
	m_bg_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(vastar_video_device::get_bg_tile_info<0>)), TILEMAP_SCAN_ROWS, 8,8, 32,32);
	m_bg_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(vastar_video_device::get_bg_tile_info<1>)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);

	m_bg_tilemap[0]->set_scroll_cols(32);
	m_bg_tilemap[1]->set_scroll_cols(32);

	m_fg_vregs = 0;
	m_flip_screen = 0;

	save_item(NAME(m_fg_vregs));
	save_item(NAME(m_flip_screen));
}

void vastar_video_device::flipscreen_w(int state)
{
	m_flip_screen = state ? 1 : 0;

	m_bg_tilemap[0]->set_flip(m_flip_screen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_bg_tilemap[1]->set_flip(m_flip_screen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_fg_tilemap->set_flip(m_flip_screen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(vastar_video_device::get_bg_tile_info)
{
	int const code = m_bgvideoram[Which][tile_index + m_bg_codebase] | (m_bgvideoram[Which][tile_index + m_bg_attrbase] << 8);
	int const color = m_bgvideoram[Which][tile_index + m_bg_colbase];
	int const fxy = (code & 0xc00) >> 10;
	tileinfo.set(4 - Which,
			code,
			color & 0x3f,
			TILE_FLIPXY(fxy));
}

TILE_GET_INFO_MEMBER(vastar_video_device::get_fg_tile_info)
{
	int const code = m_fgvideoram[tile_index + m_fg_codebase] | (m_fgvideoram[tile_index + m_fg_attrbase] << 8);
	int const color = m_fgvideoram[tile_index + m_fg_colbase];
	// TODO: guess, based on the other layers
	int const fxy = (code & 0xc00) >> 10;
	tileinfo.set(0,
			code,
			color & 0x3f,
			TILE_FLIPXY(fxy));
}

void vastar_video_device::draw_sprites(bitmap_rgb32& bitmap, const rectangle& cliprect, uint16_t rambase, uint16_t tilebase)
{
	uint8_t const *const spriteram = &m_fgvideoram[rambase];
	for (int offs = 0; offs < 0x10; offs += 2)
	{
		const int code = ((spriteram[m_spr_code_x + offs] & 0xfc) >> 2) + ((spriteram[m_spr_attr + offs] & 0x01) << 6) + tilebase;

		const int sx = spriteram[m_spr_code_x + offs + 1];
		int sy = spriteram[m_spr_y_col + offs];
		const int color = spriteram[m_spr_y_col + offs + 1] & 0x3f;
		bool flipy, flipx;

		if (m_alt_spriteflip)
		{
			flipy = (spriteram[m_spr_code_x + offs] & 0x02) ? true : false;
			flipx = (spriteram[m_spr_code_x + offs] & 0x01) ? true : false;
		}
		else
		{
			flipy = (spriteram[m_spr_code_x + offs] & 0x01) ? true : false;
			flipx = (spriteram[m_spr_code_x + offs] & 0x02) ? true : false;
		}

		if (m_flip_screen)
		{
			flipx = !flipx;
			flipy = !flipy;
		}

		if (spriteram[m_spr_attr + offs] & 0x08)   // double width
		{
			if (!m_flip_screen)
				sy = 224 - sy;

			gfx(2)->transpen(bitmap, cliprect,
				code / 2,
				color,
				flipx, flipy,
				sx, sy, 0);
			// redraw with wraparound y
			gfx(2)->transpen(bitmap, cliprect,
				code / 2,
				color,
				flipx, flipy,
				sx, sy + 256, 0);

			// redraw with wraparound x
			gfx(2)->transpen(bitmap, cliprect,
				code / 2,
				color,
				flipx, flipy,
				sx - 256, sy, 0);

			// redraw with wraparound xy
			gfx(2)->transpen(bitmap, cliprect,
				code / 2,
				color,
				flipx, flipy,
				sx - 256, sy + 256, 0);
		}
		else
		{
			if (!m_flip_screen)
				sy = 240 - sy;

			gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx, sy, 0);

			// redraw with wraparound x
			gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx - 256, sy, 0);
		}
	}
}

uint32_t vastar_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 32; i++)
	{
		m_bg_tilemap[0]->set_scrolly(i, m_fgvideoram[m_bg_scroll0 + i]);
		m_bg_tilemap[1]->set_scrolly(i, m_fgvideoram[m_bg_scroll1 + i]);
	}

	// Looks like $ac00 is some kind of '46C mixer control.
	switch (m_fg_vregs)
	{
	case 0:
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect, 0x30, 0x80);
		draw_sprites(bitmap, cliprect, 0x10, 0x00);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;

	case 1: // ?? planet probe
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect, 0x30, 0x80);
		draw_sprites(bitmap, cliprect, 0x10, 0x00);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;

	case 2:
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect, 0x30, 0x80);
		draw_sprites(bitmap, cliprect, 0x10, 0x00);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;

	case 3:
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect, 0x30, 0x80);
		draw_sprites(bitmap, cliprect, 0x10, 0x00);
		break;

	case 4: // akazukin title screen
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect, 0x30, 0x80);
		draw_sprites(bitmap, cliprect, 0x10, 0x00);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		break;

	default:
		popmessage("Unimplemented priority %X\n", m_fg_vregs);
		break;
	}

	return 0;
}

void vastar_video_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(vastar_video_device::screen_update));
}

void vastar_video_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, "palette", palette_device::RGB_444_PROMS, "proms", 256);
}
