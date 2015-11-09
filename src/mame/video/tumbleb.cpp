// license:BSD-3-Clause
// copyright-holders:David Haywood,Bryan McPhail
/***************************************************************************

   Tumblepop (bootlegs) and similar hardware
   Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

The original uses Data East custom chip 55 for backgrounds,
custom chip 52 for sprites.  The bootlegs use generic chips to perform similar
functions

Tumblepop is one of few games to take advantage of the playfields ability
to switch between 8*8 tiles and 16*16 tiles.

***************************************************************************/

#include "emu.h"
#include "includes/tumbleb.h"

/******************************************************************************/


/******************************************************************************/

WRITE16_MEMBER(tumbleb_state::bcstory_tilebank_w)
{
	data &= mem_mask;

	m_tilebank = data;
	m_pf1_tilemap->mark_all_dirty();
	m_pf1_alt_tilemap->mark_all_dirty();
	m_pf2_tilemap->mark_all_dirty();
}

WRITE16_MEMBER(tumbleb_state::chokchok_tilebank_w)
{
	data &= mem_mask;

	m_tilebank = data << 1;
	m_pf1_tilemap->mark_all_dirty();
	m_pf1_alt_tilemap->mark_all_dirty();
	m_pf2_tilemap->mark_all_dirty();
}

WRITE16_MEMBER(tumbleb_state::wlstar_tilebank_w)
{
	/* it just writes 0000 or ffff */
	m_tilebank = data & 0x4000;
	m_pf1_tilemap->mark_all_dirty();
	m_pf1_alt_tilemap->mark_all_dirty();
	m_pf2_tilemap->mark_all_dirty();
}


WRITE16_MEMBER(tumbleb_state::suprtrio_tilebank_w)
{
	data &= mem_mask;

	m_tilebank = data << 14; // shift it here, makes using bcstory_tilebank easier
	m_pf1_tilemap->mark_all_dirty();
	m_pf1_alt_tilemap->mark_all_dirty();
	m_pf2_tilemap->mark_all_dirty();
}


WRITE16_MEMBER(tumbleb_state::tumblepb_pf1_data_w)
{
	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset);
	m_pf1_alt_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tumbleb_state::tumblepb_pf2_data_w)
{
	COMBINE_DATA(&m_pf2_data[offset]);
	m_pf2_tilemap->mark_tile_dirty(offset);

	if (m_pf2_alt_tilemap)
		m_pf2_alt_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tumbleb_state::fncywld_pf1_data_w)
{
	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset / 2);
	m_pf1_alt_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(tumbleb_state::fncywld_pf2_data_w)
{
	COMBINE_DATA(&m_pf2_data[offset]);
	m_pf2_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(tumbleb_state::tumblepb_control_0_w)
{
	COMBINE_DATA(&m_control_0[offset]);
}


WRITE16_MEMBER(tumbleb_state::pangpang_pf1_data_w)
{
	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset / 2);
	m_pf1_alt_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(tumbleb_state::pangpang_pf2_data_w)
{
	COMBINE_DATA(&m_pf2_data[offset]);
	m_pf2_tilemap->mark_tile_dirty(offset / 2);

	if (m_pf2_alt_tilemap)
		m_pf2_alt_tilemap->mark_tile_dirty(offset / 2);
}

/******************************************************************************/

TILEMAP_MAPPER_MEMBER(tumbleb_state::tumblep_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

inline void tumbleb_state::get_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base)
{
	int data = gfx_base[tile_index];

	SET_TILE_INFO_MEMBER(gfx_bank,
			(data & 0x0fff) | (m_tilebank >> 2),
			data >> 12,
			0);
}

TILE_GET_INFO_MEMBER(tumbleb_state::get_bg1_tile_info){ get_bg_tile_info(tileinfo, tile_index, 2, m_pf1_data); }
TILE_GET_INFO_MEMBER(tumbleb_state::get_bg2_tile_info){ get_bg_tile_info(tileinfo, tile_index, 1, m_pf2_data); }

TILE_GET_INFO_MEMBER(tumbleb_state::get_fg_tile_info)
{
	int data = m_pf1_data[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(data & 0x0fff) | m_tilebank,
			data >> 12,
			0);
}

inline void tumbleb_state::get_fncywld_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base)
{
	int data = gfx_base[tile_index * 2];
	int attr = gfx_base[tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(gfx_bank,
			data & 0x1fff,
			attr & 0x1f,
			0);
}

TILE_GET_INFO_MEMBER(tumbleb_state::get_fncywld_bg1_tile_info){ get_fncywld_bg_tile_info(tileinfo, tile_index, 2, m_pf1_data); }
TILE_GET_INFO_MEMBER(tumbleb_state::get_fncywld_bg2_tile_info){ get_fncywld_bg_tile_info(tileinfo, tile_index, 1, m_pf2_data); }

TILE_GET_INFO_MEMBER(tumbleb_state::get_fncywld_fg_tile_info)
{
	int data = m_pf1_data[tile_index * 2];
	int attr = m_pf1_data[tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(0,
			data & 0x1fff,
			attr & 0x1f,
			0);
}


inline void tumbleb_state::pangpang_get_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base )
{
	int data = gfx_base[tile_index * 2 + 1];
	int attr = gfx_base[tile_index * 2];

	SET_TILE_INFO_MEMBER(gfx_bank,
			data & 0x1fff,
			(attr >>12) & 0xf,
			0);
}

inline void tumbleb_state::pangpang_get_bg2x_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base )
{
	int data = gfx_base[tile_index * 2 + 1];
	int attr = gfx_base[tile_index * 2];

	SET_TILE_INFO_MEMBER(gfx_bank,
			(data & 0xfff) + 0x1000,
			(attr >>12) & 0xf,
			0);
}


TILE_GET_INFO_MEMBER(tumbleb_state::pangpang_get_bg1_tile_info){ pangpang_get_bg_tile_info(tileinfo, tile_index, 2, m_pf1_data); }
TILE_GET_INFO_MEMBER(tumbleb_state::pangpang_get_bg2_tile_info){ pangpang_get_bg2x_tile_info(tileinfo, tile_index, 1, m_pf2_data); }

TILE_GET_INFO_MEMBER(tumbleb_state::pangpang_get_fg_tile_info)
{
	int data = m_pf1_data[tile_index * 2 + 1];
	int attr = m_pf1_data[tile_index * 2];

	SET_TILE_INFO_MEMBER(0,
			data & 0x1fff,
			(attr >> 12)& 0x1f,
			0);
}


void tumbleb_state::tumbleb_tilemap_redraw()
{
	m_pf1_tilemap->mark_all_dirty();
	m_pf1_alt_tilemap->mark_all_dirty();
	m_pf2_tilemap->mark_all_dirty();
	if (m_pf2_alt_tilemap)
		m_pf2_alt_tilemap->mark_all_dirty();
}

VIDEO_START_MEMBER(tumbleb_state,pangpang)
{
	m_pf1_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::pangpang_get_fg_tile_info),this),  TILEMAP_SCAN_ROWS, 8,  8, 64, 32);
	m_pf1_alt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::pangpang_get_bg1_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);
	m_pf2_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::pangpang_get_bg2_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);

	m_pf1_tilemap->set_transparent_pen(0);
	m_pf1_alt_tilemap->set_transparent_pen(0);

	machine().save().register_postload(save_prepost_delegate(FUNC(tumbleb_state::tumbleb_tilemap_redraw), this));
}


VIDEO_START_MEMBER(tumbleb_state,tumblepb)
{
	m_pf1_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_fg_tile_info),this),  TILEMAP_SCAN_ROWS, 8,  8, 64, 32);
	m_pf1_alt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_bg1_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);
	m_pf2_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_bg2_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);

	m_pf1_tilemap->set_transparent_pen(0);
	m_pf1_alt_tilemap->set_transparent_pen(0);

	machine().save().register_postload(save_prepost_delegate(FUNC(tumbleb_state::tumbleb_tilemap_redraw), this));
}

VIDEO_START_MEMBER(tumbleb_state,sdfight)
{
	m_pf1_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_fg_tile_info),this),  TILEMAP_SCAN_ROWS, 8,  8, 64, 64); // 64*64 to prevent bad tilemap wrapping? - check real behavior
	m_pf1_alt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_bg1_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);
	m_pf2_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_bg2_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);

	m_pf1_tilemap->set_transparent_pen(0);
	m_pf1_alt_tilemap->set_transparent_pen(0);

	machine().save().register_postload(save_prepost_delegate(FUNC(tumbleb_state::tumbleb_tilemap_redraw), this));
}

VIDEO_START_MEMBER(tumbleb_state,fncywld)
{
	m_pf1_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_fncywld_fg_tile_info),this),  TILEMAP_SCAN_ROWS, 8,  8, 64, 32);
	m_pf1_alt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_fncywld_bg1_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);
	m_pf2_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_fncywld_bg2_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);

	m_pf1_tilemap->set_transparent_pen(15);
	m_pf1_alt_tilemap->set_transparent_pen(15);

	machine().save().register_postload(save_prepost_delegate(FUNC(tumbleb_state::tumbleb_tilemap_redraw), this));
}


VIDEO_START_MEMBER(tumbleb_state,suprtrio)
{
	m_pf1_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_fg_tile_info),this),  TILEMAP_SCAN_ROWS, 8,  8, 64, 32);
	m_pf1_alt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_bg1_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);
	m_pf2_tilemap =     &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tumbleb_state::get_bg2_tile_info),this), tilemap_mapper_delegate(FUNC(tumbleb_state::tumblep_scan),this),     16, 16, 64, 32);

	m_pf1_alt_tilemap->set_transparent_pen(0);

	machine().save().register_postload(save_prepost_delegate(FUNC(tumbleb_state::tumbleb_tilemap_redraw), this));
}

/******************************************************************************/

void tumbleb_state::tumbleb_draw_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pf1x_offs, int pf1y_offs, int pf2x_offs, int pf2y_offs)
{
	m_pf1_tilemap->set_scrollx(0, m_control_0[1] + pf1x_offs);
	m_pf1_tilemap->set_scrolly(0, m_control_0[2] + pf1y_offs);
	m_pf1_alt_tilemap->set_scrollx(0, m_control_0[1] + pf1x_offs);
	m_pf1_alt_tilemap->set_scrolly(0, m_control_0[2] + pf1y_offs);
	m_pf2_tilemap->set_scrollx(0, m_control_0[3] + pf2x_offs);
	m_pf2_tilemap->set_scrolly(0, m_control_0[4] + pf2y_offs);

	m_pf2_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (m_control_0[6] & 0x80)
		m_pf1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		m_pf1_alt_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, m_spriteram.bytes()/2);
}

UINT32 tumbleb_state::screen_update_tumblepb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, offs2;

	flip_screen_set(m_control_0[0] & 0x80);

	if (flip_screen())
		offs = 1;
	else
		offs = -1;

	if (flip_screen())
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen, bitmap, cliprect, offs2, 0, offs, 0);

	return 0;
}

UINT32 tumbleb_state::screen_update_jumpkids(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, offs2;

	flip_screen_set(m_control_0[0] & 0x80);

	if (flip_screen())
		offs = 1;
	else
		offs = -1;

	if (flip_screen())
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen, bitmap, cliprect, offs2, 0, offs, 0);
	return 0;
}

UINT32 tumbleb_state::screen_update_semicom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, offs2;

	flip_screen_set(m_control_0[0] & 0x80);

	if (flip_screen())
		offs = 1;
	else
		offs = -1;

	if (flip_screen())
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen, bitmap, cliprect, offs2, 0, offs, 0);
	return 0;
}

UINT32 tumbleb_state::screen_update_semicom_altoffsets(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offsx, offsy, offsx2;

	flip_screen_set(m_control_0[0] & 0x80);

	offsx = -1;
	offsy = 2;
	offsx2 = -5;

	tumbleb_draw_common(screen, bitmap, cliprect, offsx2, 0, offsx, offsy);

	return 0;
}

UINT32 tumbleb_state::screen_update_bcstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, offs2;

	flip_screen_set(m_control_0[0] & 0x80);

	/* not sure of this */
	if (flip_screen())
		offs = 1;
	else
		offs = 8;

	/* not sure of this */
	if (flip_screen())
		offs2 = -3;
	else
		offs2 = 8;

	tumbleb_draw_common(screen, bitmap, cliprect, offs2, 0, offs, 0);
	return 0;
}

UINT32 tumbleb_state::screen_update_semibase(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, offs2;

	flip_screen_set(m_control_0[0] & 0x80);

	offs = -1;
	offs2 = -2;

	tumbleb_draw_common(screen, bitmap, cliprect, offs2, 0, offs, 0);

	return 0;
}

UINT32 tumbleb_state::screen_update_sdfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, offs2;

	flip_screen_set(m_control_0[0] & 0x80);

	offs = -1;
	offs2 = -5; // foreground scroll..

	tumbleb_draw_common(screen, bitmap, cliprect, offs2, -16, offs, 0);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, m_spriteram.bytes()/2);
	return 0;
}

UINT32 tumbleb_state::screen_update_fncywld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, offs2;

	flip_screen_set(m_control_0[0] & 0x80);

	if (flip_screen())
		offs = 1;
	else
		offs = -1;

	if (flip_screen())
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen, bitmap, cliprect, offs2, 0, offs, 0);

	return 0;
}

UINT32 tumbleb_state::screen_update_pangpang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, offs2;

	flip_screen_set(m_control_0[0] & 0x80);

	if (flip_screen())
		offs = 1;
	else
		offs = -1;

	if (flip_screen())
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen, bitmap, cliprect, offs2, 0, offs, 0);
	return 0;
}



UINT32 tumbleb_state::screen_update_suprtrio(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pf1_alt_tilemap->set_scrollx(0, -m_control[1] - 6);
	m_pf1_alt_tilemap->set_scrolly(0, -m_control[2]);
	m_pf2_tilemap->set_scrollx(0, -m_control[3] - 2);
	m_pf2_tilemap->set_scrolly(0, -m_control[4]);

	m_pf2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_pf1_alt_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, m_spriteram.bytes()/2);
	return 0;
}
