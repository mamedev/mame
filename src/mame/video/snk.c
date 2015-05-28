// license:???
// copyright-holders:Marco Cassili,Ernesto Corvi,Tim Lindquist,Carlos A. Lozano,Bryan McPhail,Jarek Parchanski,Nicola Salmoria,Tomasz Slanina,Phil Stroffolino,Acho A. Tang,Victor Trucco
#include "emu.h"
#include "includes/snk.h"

/*******************************************************************************
 Shadow Handling Notes
********************************************************************************
 Shadows are handled by changing palette bank.

 Games Not Using Shadows

 those using gwar_vh_screenrefresh (gwar, bermudat, psychos, chopper1)
    (0-15 , 15 is transparent)

 Games Using Shadows

 those using tnk3_vh_screenrefresh (tnk3, athena, fitegolf) sgladiat is similar
    (0-7  , 6  is shadow, 7  is transparent) * these are using aso colour prom convert *
 those using ikari_vh_screenrefresh (ikari, victroad)
    (0-7  , 6  is shadow, 7  is transparent)
 those using tdfever_vh_screenrefresh (tdfever, fsoccer)
    (0-15 , 14 is shadow, 15 is transparent)

*******************************************************************************/


/**************************************************************************************/

PALETTE_INIT_MEMBER(snk_state,tnk3)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	int num_colors = 0x400;

	for( i=0; i<num_colors; i++ )
	{
		int bit0=0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[i + 2*num_colors] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*num_colors] >> 2) & 0x01;
		bit1 = (color_prom[i + num_colors] >> 2) & 0x01;
		bit2 = (color_prom[i + num_colors] >> 3) & 0x01;
		bit3 = (color_prom[i] >> 0) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*num_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*num_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + num_colors] >> 0) & 0x01;
		bit3 = (color_prom[i + num_colors] >> 1) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}

/**************************************************************************************/

TILEMAP_MAPPER_MEMBER(snk_state::marvins_tx_scan_cols)
{
	// tilemap is 36x28, the central part is from the first RAM page and the
	// extra 4 columns are from the second page
	col -= 2;
	if (col & 0x20)
		return 0x400 + row + ((col & 0x1f) << 5);
	else
		return row + (col << 5);
}

TILE_GET_INFO_MEMBER(snk_state::marvins_get_tx_tile_info)
{
	int code = m_tx_videoram[tile_index];
	int color = code >> 5;

	SET_TILE_INFO_MEMBER(0,
			m_tx_tile_offset + code,
			color,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

TILE_GET_INFO_MEMBER(snk_state::ikari_get_tx_tile_info)
{
	int code = m_tx_videoram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			m_tx_tile_offset + code,
			0,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

TILE_GET_INFO_MEMBER(snk_state::gwar_get_tx_tile_info)
{
	int code = m_tx_videoram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			m_tx_tile_offset + code,
			0,
			0);
}


TILE_GET_INFO_MEMBER(snk_state::marvins_get_fg_tile_info)
{
	int code = m_fg_videoram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code,
			0,
			0);
}

TILE_GET_INFO_MEMBER(snk_state::marvins_get_bg_tile_info)
{
	int code = m_bg_videoram[tile_index];

	SET_TILE_INFO_MEMBER(2,
			code,
			0,
			0);
}


TILE_GET_INFO_MEMBER(snk_state::aso_get_bg_tile_info)
{
	int code = m_bg_videoram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			m_bg_tile_offset + code,
			0,
			0);
}

TILE_GET_INFO_MEMBER(snk_state::tnk3_get_bg_tile_info)
{
	int attr = m_bg_videoram[2*tile_index+1];
	int code = m_bg_videoram[2*tile_index] | ((attr & 0x30) << 4);
	int color = (attr & 0xf) ^ 8;

	SET_TILE_INFO_MEMBER(1,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(snk_state::ikari_get_bg_tile_info)
{
	int attr = m_bg_videoram[2*tile_index+1];
	int code = m_bg_videoram[2*tile_index] | ((attr & 0x03) << 8);
	int color = (attr & 0x70) >> 4;

	SET_TILE_INFO_MEMBER(1,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(snk_state::gwar_get_bg_tile_info)
{
	int attr = m_bg_videoram[2*tile_index+1];
	int code = m_bg_videoram[2*tile_index] | ((attr & 0x0f) << 8);
	int color = (attr & 0xf0) >> 4;

	if (m_is_psychos)   // psychos has a separate palette bank bit
		color &= 7;

	SET_TILE_INFO_MEMBER(1,
			code,
			color,
			0);

	// bermudat, tdfever use FFFF to blank the background.
	// (still call SET_TILE_INFO_MEMBER, otherwise problems might occur on boot when
	// the tile data hasn't been initialised)
	if (code >= m_gfxdecode->gfx(1)->elements())
		tileinfo.pen_data = m_empty_tile;
}


/**************************************************************************************/

VIDEO_START_MEMBER(snk_state,snk_3bpp_shadow)
{
	int i;

	if(!(m_palette->shadows_enabled()))
		fatalerror("driver should use VIDEO_HAS_SHADOWS\n");

	/* prepare shadow draw table */
	for(i = 0; i <= 5; i++) m_drawmode_table[i] = DRAWMODE_SOURCE;
	m_drawmode_table[6] = (m_palette->shadows_enabled()) ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;
	m_drawmode_table[7] = DRAWMODE_NONE;

	for (i = 0x000;i < 0x400;i++)
		m_palette->shadow_table()[i] = i | 0x200;
}

VIDEO_START_MEMBER(snk_state,snk_4bpp_shadow)
{
	int i;

	if(!(m_palette->shadows_enabled()))
		fatalerror("driver should use VIDEO_HAS_SHADOWS\n");

	/* prepare shadow draw table */
	for(i = 0; i <= 13; i++) m_drawmode_table[i] = DRAWMODE_SOURCE;
	m_drawmode_table[14] = DRAWMODE_SHADOW;
	m_drawmode_table[15] = DRAWMODE_NONE;

	/* all palette entries are not affected by shadow sprites... */
	for (i = 0x000;i < 0x400;i++)
		m_palette->shadow_table()[i] = i;
	/* ... except for tilemap colors */
	for (i = 0x200;i < 0x300;i++)
		m_palette->shadow_table()[i] = i + 0x100;
}


VIDEO_START_MEMBER(snk_state,marvins)
{
	VIDEO_START_CALL_MEMBER(snk_3bpp_shadow);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::marvins_get_tx_tile_info),this), tilemap_mapper_delegate(FUNC(snk_state::marvins_tx_scan_cols),this), 8, 8, 36, 28);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::marvins_get_fg_tile_info),this), TILEMAP_SCAN_COLS,    8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::marvins_get_bg_tile_info),this), TILEMAP_SCAN_COLS,    8, 8, 64, 32);

	m_tx_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_scrolldy(8, 8);

	m_fg_tilemap->set_transparent_pen(15);
	m_fg_tilemap->set_scrolldx(15,  31);
	m_fg_tilemap->set_scrolldy(8, -32);

	m_bg_tilemap->set_scrolldx(15,  31);
	m_bg_tilemap->set_scrolldy(8, -32);

	m_tx_tile_offset = 0;
}

VIDEO_START_MEMBER(snk_state,jcross)
{
	VIDEO_START_CALL_MEMBER(snk_3bpp_shadow);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::marvins_get_tx_tile_info),this), tilemap_mapper_delegate(FUNC(snk_state::marvins_tx_scan_cols),this), 8, 8, 36, 28);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::aso_get_bg_tile_info),this),     TILEMAP_SCAN_COLS,    8, 8, 64, 64);

	m_tx_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_scrolldy(8, 8);

	m_bg_tilemap->set_scrolldx(15, 24);
	m_bg_tilemap->set_scrolldy(8, -32);

	m_num_sprites = 25;
	m_yscroll_mask = 0x1ff;
	m_bg_tile_offset = 0;
	m_tx_tile_offset = 0;
}

VIDEO_START_MEMBER(snk_state,sgladiat)
{
	VIDEO_START_CALL_MEMBER(snk_3bpp_shadow);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::marvins_get_tx_tile_info),this), tilemap_mapper_delegate(FUNC(snk_state::marvins_tx_scan_cols),this), 8, 8, 36, 28);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::aso_get_bg_tile_info),this),     TILEMAP_SCAN_COLS,    8, 8, 64, 32);

	m_tx_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_scrolldy(8, 8);

	m_bg_tilemap->set_scrolldx(15, 24);
	m_bg_tilemap->set_scrolldy(8, -32);

	m_num_sprites = 25;
	m_yscroll_mask = 0x0ff;
	m_bg_tile_offset = 0;
	m_tx_tile_offset = 0;
}

VIDEO_START_MEMBER(snk_state,hal21)
{
	VIDEO_START_CALL_MEMBER(jcross);

	m_bg_tilemap->set_scrolldy(8, -32+256);

	m_num_sprites = 50;
	m_yscroll_mask = 0x1ff;
}

VIDEO_START_MEMBER(snk_state,aso)
{
	VIDEO_START_CALL_MEMBER(jcross);

	m_bg_tilemap->set_scrolldx(15+256, 24+256);

	m_num_sprites = 50;
	m_yscroll_mask = 0x1ff;
}


VIDEO_START_MEMBER(snk_state,tnk3)
{
	VIDEO_START_CALL_MEMBER(snk_3bpp_shadow);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::marvins_get_tx_tile_info),this), tilemap_mapper_delegate(FUNC(snk_state::marvins_tx_scan_cols),this), 8, 8, 36, 28);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::tnk3_get_bg_tile_info),this),    TILEMAP_SCAN_COLS,    8, 8, 64, 64);

	m_tx_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_scrolldy(8, 8);

	m_bg_tilemap->set_scrolldx(15, 24);
	m_bg_tilemap->set_scrolldy(8, -32);

	m_num_sprites = 50;
	m_yscroll_mask = 0x1ff;
	m_tx_tile_offset = 0;
}

VIDEO_START_MEMBER(snk_state,ikari)
{
	VIDEO_START_CALL_MEMBER(snk_3bpp_shadow);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::ikari_get_tx_tile_info),this), tilemap_mapper_delegate(FUNC(snk_state::marvins_tx_scan_cols),this),  8,  8, 36, 28);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::ikari_get_bg_tile_info),this), TILEMAP_SCAN_COLS,    16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_scrolldy(8, 8);

	m_bg_tilemap->set_scrolldx(15, 24);
	m_bg_tilemap->set_scrolldy(8, -32);

	m_tx_tile_offset = 0;
}

VIDEO_START_MEMBER(snk_state,gwar)
{
	int i;

	/* prepare drawmode table */
	for(i = 0; i <= 14; i++) m_drawmode_table[i] = DRAWMODE_SOURCE;
	m_drawmode_table[15] = DRAWMODE_NONE;

	memset(m_empty_tile, 0xf, sizeof(m_empty_tile));

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::gwar_get_tx_tile_info),this), TILEMAP_SCAN_COLS,  8,  8, 50, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk_state::gwar_get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(15);

	m_bg_tilemap->set_scrolldx(16, 143);
	m_bg_tilemap->set_scrolldy(0, -32);

	m_tx_tile_offset = 0;

	m_is_psychos = 0;
}

VIDEO_START_MEMBER(snk_state,psychos)
{
	VIDEO_START_CALL_MEMBER(gwar);
	m_is_psychos = 1;
}

VIDEO_START_MEMBER(snk_state,tdfever)
{
	VIDEO_START_CALL_MEMBER(gwar);
	VIDEO_START_CALL_MEMBER(snk_4bpp_shadow);
}

/**************************************************************************************/

WRITE8_MEMBER(snk_state::snk_tx_videoram_w)
{
	m_tx_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(snk_state::marvins_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(snk_state::marvins_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(snk_state::snk_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}


WRITE8_MEMBER(snk_state::snk_fg_scrollx_w)
{
	m_fg_scrollx = (m_fg_scrollx & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::snk_fg_scrolly_w)
{
	m_fg_scrolly = (m_fg_scrolly & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::snk_bg_scrollx_w)
{
	m_bg_scrollx = (m_bg_scrollx & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::snk_bg_scrolly_w)
{
	m_bg_scrolly = (m_bg_scrolly & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::snk_sp16_scrollx_w)
{
	m_sp16_scrollx = (m_sp16_scrollx & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::snk_sp16_scrolly_w)
{
	m_sp16_scrolly = (m_sp16_scrolly & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::snk_sp32_scrollx_w)
{
	m_sp32_scrollx = (m_sp32_scrollx & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::snk_sp32_scrolly_w)
{
	m_sp32_scrolly = (m_sp32_scrolly & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::snk_sprite_split_point_w)
{
	m_sprite_split_point = data;
}


WRITE8_MEMBER(snk_state::marvins_palette_bank_w)
{
	m_bg_tilemap->set_palette_offset(data & 0x70);
	m_fg_tilemap->set_palette_offset((data & 0x07) << 4);
}

WRITE8_MEMBER(snk_state::marvins_flipscreen_w)
{
	flip_screen_set(data & 0x80);

	// other bits unknown
}

WRITE8_MEMBER(snk_state::sgladiat_flipscreen_w)
{
	flip_screen_set(data & 0x80);

	m_bg_tilemap->set_palette_offset(((data & 0xf) ^ 8) << 4);

	// other bits unknown
}

WRITE8_MEMBER(snk_state::hal21_flipscreen_w)
{
	flip_screen_set(data & 0x80);

	m_bg_tilemap->set_palette_offset(((data & 0xf) ^ 8) << 4);
	if (m_bg_tile_offset != ((data & 0x20) << 3))
	{
		m_bg_tile_offset = (data & 0x20) << 3;
		m_bg_tilemap->mark_all_dirty();
	}

	// other bits unknown
}

WRITE8_MEMBER(snk_state::marvins_scroll_msb_w)
{
	m_bg_scrollx =   (m_bg_scrollx   & 0xff) | ((data & 0x04) << 6);
	m_fg_scrollx =   (m_fg_scrollx   & 0xff) | ((data & 0x02) << 7);
	m_sp16_scrollx = (m_sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_MEMBER(snk_state::jcross_scroll_msb_w)
{
	m_bg_scrolly =   (m_bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	m_sp16_scrolly = (m_sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	m_bg_scrollx =   (m_bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	m_sp16_scrollx = (m_sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_MEMBER(snk_state::sgladiat_scroll_msb_w)
{
	m_bg_scrollx =   (m_bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	m_sp16_scrollx = (m_sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_MEMBER(snk_state::aso_videoattrs_w)
{
	/*
	    video attributes:
	    X-------
	    -X------
	    --X-----    flip screen
	    ---X----    scrolly MSB (background)
	    ----X---    scrolly MSB (sprites)
	    -----X--
	    ------X-    scrollx MSB (background)
	    -------X    scrollx MSB (sprites)
	*/


	flip_screen_set(data & 0x20);

	m_bg_scrolly =   (m_bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	m_sp16_scrolly = (m_sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	m_bg_scrollx =   (m_bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	m_sp16_scrollx = (m_sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_MEMBER(snk_state::tnk3_videoattrs_w)
{
	/*
	    video attributes:
	    X-------    flip screen
	    -X------    character bank (for text layer)
	    --X-----
	    ---X----    scrolly MSB (background)
	    ----X---    scrolly MSB (sprites)
	    -----X--
	    ------X-    scrollx MSB (background)
	    -------X    scrollx MSB (sprites)
	*/


	flip_screen_set(data & 0x80);

	if (m_tx_tile_offset != ((data & 0x40) << 2))
	{
		m_tx_tile_offset = (data & 0x40) << 2;
		m_tx_tilemap->mark_all_dirty();
	}

	m_bg_scrolly =   (m_bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	m_sp16_scrolly = (m_sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	m_bg_scrollx =   (m_bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	m_sp16_scrollx = (m_sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_MEMBER(snk_state::aso_bg_bank_w)
{
	m_bg_tilemap->set_palette_offset(((data & 0xf) ^ 8) << 4);
	if (m_bg_tile_offset != ((data & 0x30) << 4))
	{
		m_bg_tile_offset = (data & 0x30) << 4;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(snk_state::ikari_bg_scroll_msb_w)
{
	m_bg_scrollx = (m_bg_scrollx & 0xff) | ((data & 0x02) << 7);
	m_bg_scrolly = (m_bg_scrolly & 0xff) | ((data & 0x01) << 8);
}

WRITE8_MEMBER(snk_state::ikari_sp_scroll_msb_w)
{
	m_sp32_scrollx = (m_sp32_scrollx & 0xff) | ((data & 0x20) << 3);
	m_sp16_scrollx = (m_sp16_scrollx & 0xff) | ((data & 0x10) << 4);
	m_sp32_scrolly = (m_sp32_scrolly & 0xff) | ((data & 0x08) << 5);
	m_sp16_scrolly = (m_sp16_scrolly & 0xff) | ((data & 0x04) << 6);
}

WRITE8_MEMBER(snk_state::ikari_unknown_video_w)
{
	/* meaning of 0xc980 uncertain.
	   Normally 0x20, ikaria/ikarijp sets it to 0x31 during test mode.
	   Changing char bank is necessary to fix the display during the
	   hard flags test and the test grid.
	   Changing palette bank is necessary to fix colors in test mode. */


if (data != 0x20 && // normal
	data != 0x31 && // ikari test
	data != 0xaa)   // victroad spurious during boot
	popmessage("attrs %02x contact MAMEDEV", data);

	m_tx_tilemap->set_palette_offset((data & 0x01) << 4);
	if (m_tx_tile_offset != ((data & 0x10) << 4))
	{
		m_tx_tile_offset = (data & 0x10) << 4;
		m_tx_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(snk_state::gwar_tx_bank_w)
{
	m_tx_tilemap->set_palette_offset((data & 0xf) << 4);
	if (m_tx_tile_offset != ((data & 0x30) << 4))
	{
		m_tx_tile_offset = (data & 0x30) << 4;
		m_tx_tilemap->mark_all_dirty();
	}

	if (m_is_psychos)
		m_bg_tilemap->set_palette_offset((data & 0x80));
}

WRITE8_MEMBER(snk_state::gwar_videoattrs_w)
{
	flip_screen_set(data & 0x04);

	m_sp32_scrollx = (m_sp32_scrollx & 0xff) | ((data & 0x80) << 1);
	m_sp16_scrollx = (m_sp16_scrollx & 0xff) | ((data & 0x40) << 2);
	m_sp32_scrolly = (m_sp32_scrolly & 0xff) | ((data & 0x20) << 3);
	m_sp16_scrolly = (m_sp16_scrolly & 0xff) | ((data & 0x10) << 4);
	m_bg_scrollx =   (m_bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	m_bg_scrolly =   (m_bg_scrolly   & 0xff) | ((data & 0x01) << 8);
}

WRITE8_MEMBER(snk_state::gwara_videoattrs_w)
{
	flip_screen_set(data & 0x10);

	m_bg_scrollx =   (m_bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	m_bg_scrolly =   (m_bg_scrolly   & 0xff) | ((data & 0x01) << 8);
}

WRITE8_MEMBER(snk_state::gwara_sp_scroll_msb_w)
{
	m_sp32_scrollx = (m_sp32_scrollx & 0xff) | ((data & 0x20) << 3);
	m_sp16_scrollx = (m_sp16_scrollx & 0xff) | ((data & 0x10) << 4);
	m_sp32_scrolly = (m_sp32_scrolly & 0xff) | ((data & 0x08) << 5);
	m_sp16_scrolly = (m_sp16_scrolly & 0xff) | ((data & 0x04) << 6);
}

WRITE8_MEMBER(snk_state::tdfever_sp_scroll_msb_w)
{
	m_sp32_scrolly = (m_sp32_scrolly & 0xff) | ((data & 0x80) << 1);
	m_sp32_scrollx = (m_sp32_scrollx & 0xff) | ((data & 0x40) << 2);
}

WRITE8_MEMBER(snk_state::tdfever_spriteram_w)
{
	/*  partial updates avoid flickers in the fsoccer radar. */
	if (offset < 0x80 && m_spriteram[offset] != data)
	{
		int vpos = m_screen->vpos();

		if (vpos > 0)
			m_screen->update_partial(vpos - 1);
	}

	m_spriteram[offset] = data;
}

/**************************************************************************************/

void snk_state::marvins_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const int scrollx, const int scrolly, const int from, const int to)
{
	gfx_element *gfx = m_gfxdecode->gfx(3);
	const UINT8 *source, *finish;

	source = m_spriteram + from*4;
	finish = m_spriteram + to*4;

	while( source<finish )
	{
		int attributes = source[3]; /* Y?F? CCCC */
		int tile_number = source[1];
		int sx =  scrollx + 301 - 15 - source[2] + ((attributes&0x80)?256:0);
		int sy = -scrolly - 8 + source[0];
		int color = attributes&0xf;
		int flipy = (attributes&0x20);
		int flipx = 0;

		if (flip_screen())
		{
			sx = 89 - 16 - sx;
			sy = 262 - 16 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sx &= 0x1ff;
		sy &= 0xff;
		if (sx > 512-16) sx -= 512;
		if (sy > 256-16) sy -= 256;

		gfx->transtable(bitmap,cliprect,
			tile_number,
			color,
			flipx, flipy,
			sx, sy,
			m_drawmode_table);

		source+=4;
	}
}


void snk_state::tnk3_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const int xscroll, const int yscroll)
{
	UINT8 *spriteram = m_spriteram;
	gfx_element *gfx = m_gfxdecode->gfx(2);
	const int size = gfx->width();
	int tile_number, attributes, color, sx, sy;
	int xflip,yflip;
	int offs;

	/* jcross and sgladiat have only 25 sprites, the others 50 */

	/* jcross has 256 tiles, attribute bit 6 is unused and bit 5 is y-flip */
	/* sgladiat and tnk3 have 512 tiles, bit 6 is bank and bit 5 is y-flip */
	/* athena has 1024 tiles, bit 6 and bit 5 are bank */

	for (offs = 0; offs < m_num_sprites*4; offs += 4)
	{
		tile_number = spriteram[offs+1];
		attributes  = spriteram[offs+3];
		color = attributes & 0xf;
		sx =  xscroll + 301 - size - spriteram[offs+2];
		sy = -yscroll + 7 - size + spriteram[offs];
		sx += (attributes & 0x80) << 1;
		sy += (attributes & 0x10) << 4;
		xflip = 0;
		yflip = 0;

		if (gfx->elements() > 256)  // all except jcross
		{
			tile_number |= (attributes & 0x40) << 2;
		}

		if (gfx->elements() > 512)  // athena
		{
			tile_number |= (attributes & 0x20) << 4;
		}
		else    // all others
		{
			yflip = attributes & 0x20;
		}

		if (flip_screen())
		{
			sx = 89 - size - sx;    // this causes slight misalignment in tnk3 but is correct for athena and fitegolf
			sy = 262 - size - sy;
			xflip = !xflip;
			yflip = !yflip;
		}

		sx &= 0x1ff;
		sy &= m_yscroll_mask;    // sgladiat apparently has only 256 pixels of vertical scrolling range
		if (sx > 512-size) sx -= 512;
		if (sy > (m_yscroll_mask+1)-size) sy -= (m_yscroll_mask+1);

		gfx->transtable(bitmap,cliprect,
				tile_number,
				color,
				xflip,yflip,
				sx,sy,
				m_drawmode_table);
	}
}


void snk_state::ikari_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const int start, const int xscroll, const int yscroll, const UINT8 *source, const int gfxnum )
{
	gfx_element *gfx = m_gfxdecode->gfx(gfxnum);
	const int size = gfx->width();
	int tile_number, attributes, color, sx, sy;
	int which, finish;

	finish = (start+25)*4;

	for (which = start*4; which < finish; which += 4)
	{
		tile_number = source[which+1];
		attributes  = source[which+3];
		color = attributes & 0xf;
		sx =  xscroll + 300 - size - source[which+2];
		sy = -yscroll + 7 - size + source[which];
		sx += (attributes & 0x80) << 1;
		sy += (attributes & 0x10) << 4;

		switch (size)
		{
			case 16:
				tile_number |= (attributes & 0x60) << 3;
				break;

			case 32:
				tile_number |= (attributes & 0x40) << 2;
				break;
		}

		sx &= 0x1ff;
		sy &= 0x1ff;
		if (sx > 512-size) sx -= 512;
		if (sy > 512-size) sy -= 512;

		gfx->transtable(bitmap,cliprect,
				tile_number,
				color,
				0,0,
				sx,sy,
				m_drawmode_table);
	}
}

/**************************************************************/

/*
Sprite Format
-------------
byte0: y offset
byte1: tile number
byte2: x offset
byte3: attributes

    32x32 attributes:

    76543210
    ----xxxx (color)
    ---x---- (y offset bit8)
    -xx----- (bank number)
    x------- (x offset bit8)

    16x16 attributes:

    76543210
    -----xxx (color)
    ---x---- (y offset bit8)
    -xx-x--- (bank number)
    x------- (x offset bit8)
*/
void snk_state::tdfever_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect,   const int xscroll, const int yscroll, const UINT8 *source, const int gfxnum,
										const int hw_xflip, const int from, const int to)
{
	gfx_element *gfx = m_gfxdecode->gfx(gfxnum);
	const int size = gfx->width();
	int tile_number, attributes, sx, sy, color;
	int which;
	int flipx, flipy;

	for(which = from*4; which < to*4; which+=4)
	{
		tile_number = source[which+1];
		attributes  = source[which+3];
		color = attributes & 0x0f;
		sx = -xscroll - 9 + source[which+2];
		sy = -yscroll + 1 - size + source[which];
		sx += (attributes & 0x80) << 1;
		sy += (attributes & 0x10) << 4;

		switch (size)
		{
			case 16:
				tile_number |= ((attributes & 0x08) << 5) | ((attributes & 0x60) << 4);
				color &= 7; // attribute bit 3 is used for bank select
				if (from == 0)
					color |= 8; // low priority sprites use the other palette bank
				break;

			case 32:
				tile_number |= (attributes & 0x60) << 3;
				break;
		}

		flipx = hw_xflip;
		flipy = 0;

		if (hw_xflip)
			sx = 495 - size - sx;

		if (flip_screen())
		{
			sx = 495 - size - sx;
			sy = 258 - size - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sx &= 0x1ff;
		sy &= 0x1ff;
		if (sx > 512-size) sx -= 512;
		if (sy > 512-size) sy -= 512;

		gfx->transtable(bitmap,cliprect,
				tile_number,
				color,
				flipx,flipy,
				sx,sy,
				m_drawmode_table);
	}
}

/**************************************************************/

UINT32 snk_state::screen_update_marvins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
	m_fg_tilemap->set_scrollx(0, m_fg_scrollx);
	m_fg_tilemap->set_scrolly(0, m_fg_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	marvins_draw_sprites(bitmap, cliprect, m_sp16_scrollx, m_sp16_scrolly, 0, m_sprite_split_point>>2);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	marvins_draw_sprites(bitmap, cliprect, m_sp16_scrollx, m_sp16_scrolly, m_sprite_split_point>>2, 25);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


UINT32 snk_state::screen_update_tnk3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	tnk3_draw_sprites(bitmap, cliprect, m_sp16_scrollx, m_sp16_scrolly);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


UINT32 snk_state::screen_update_ikari(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	ikari_draw_sprites(bitmap, cliprect,  0, m_sp16_scrollx, m_sp16_scrolly, m_spriteram + 0x800, 2 );
	ikari_draw_sprites(bitmap, cliprect,  0, m_sp32_scrollx, m_sp32_scrolly, m_spriteram,         3 );
	ikari_draw_sprites(bitmap, cliprect, 25, m_sp16_scrollx, m_sp16_scrolly, m_spriteram + 0x800, 2 );

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


UINT32 snk_state::screen_update_gwar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	tdfever_draw_sprites(bitmap, cliprect, m_sp16_scrollx, m_sp16_scrolly, m_spriteram + 0x800, 2, 0, 0, m_sprite_split_point );
	tdfever_draw_sprites(bitmap, cliprect, m_sp32_scrollx, m_sp32_scrolly, m_spriteram,         3, 0, 0, 32 );
	tdfever_draw_sprites(bitmap, cliprect, m_sp16_scrollx, m_sp16_scrolly, m_spriteram + 0x800, 2, 0, m_sprite_split_point, 64 );

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


UINT32 snk_state::screen_update_tdfever(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	tdfever_draw_sprites(bitmap, cliprect, m_sp32_scrollx, m_sp32_scrolly, m_spriteram, 2, 1, 0, 32 );

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
