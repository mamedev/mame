// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    American Speedway

*************************************************************************/

#include "sound/2151intf.h"

class amspdwy_state : public driver_device
{
public:
	amspdwy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ym2151(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym2151_device> m_ym2151;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int m_flipscreen;

	/* misc */
	UINT8 m_wheel_old[2];
	UINT8 m_wheel_return[2];

	DECLARE_READ8_MEMBER(amspdwy_wheel_0_r);
	DECLARE_READ8_MEMBER(amspdwy_wheel_1_r);
	DECLARE_WRITE8_MEMBER(amspdwy_sound_w);
	DECLARE_WRITE8_MEMBER(amspdwy_flipscreen_w);
	DECLARE_WRITE8_MEMBER(amspdwy_videoram_w);
	DECLARE_WRITE8_MEMBER(amspdwy_colorram_w);
	DECLARE_READ8_MEMBER(amspdwy_sound_r);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_cols_back);

	UINT32 screen_update_amspdwy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	UINT8 amspdwy_wheel_r( int index );

	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};
