// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria, Luca Elia
/***************************************************************************

 Lasso and similar hardware

***************************************************************************/

#include "sound/sn76496.h"

class lasso_state : public driver_device
{
public:
	lasso_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_back_color(*this, "back_color"),
		m_chip_data(*this, "chip_data"),
		m_bitmap_ram(*this, "bitmap_ram"),
		m_last_colors(*this, "last_colors"),
		m_track_scroll(*this, "track_scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_sn_1(*this, "sn76489.1"),
		m_sn_2(*this, "sn76489.2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_back_color;
	optional_shared_ptr<UINT8> m_chip_data;
	optional_shared_ptr<UINT8> m_bitmap_ram;    /* 0x2000 bytes for a 256 x 256 x 1 bitmap */
	optional_shared_ptr<UINT8> m_last_colors;
	optional_shared_ptr<UINT8> m_track_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_track_tilemap;
	UINT8    m_gfxbank;     /* used by lasso, chameleo, wwjgtin and pinbo */
	UINT8    m_track_enable;    /* used by wwjgtin */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<sn76489_device> m_sn_1;
	optional_device<sn76489_device> m_sn_2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_select_w);
	DECLARE_WRITE8_MEMBER(lasso_videoram_w);
	DECLARE_WRITE8_MEMBER(lasso_colorram_w);
	DECLARE_WRITE8_MEMBER(lasso_flip_screen_w);
	DECLARE_WRITE8_MEMBER(lasso_video_control_w);
	DECLARE_WRITE8_MEMBER(wwjgtin_video_control_w);
	DECLARE_WRITE8_MEMBER(pinbo_video_control_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILE_GET_INFO_MEMBER(lasso_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(wwjgtin_get_track_tile_info);
	TILE_GET_INFO_MEMBER(pinbo_get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(lasso);
	DECLARE_MACHINE_START(wwjgtin);
	DECLARE_MACHINE_RESET(wwjgtin);
	DECLARE_VIDEO_START(wwjgtin);
	DECLARE_PALETTE_INIT(wwjgtin);
	DECLARE_VIDEO_START(pinbo);
	UINT32 screen_update_lasso(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_chameleo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_wwjgtin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	rgb_t get_color( int data );
	void wwjgtin_set_last_four_colors();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int reverse );
	void draw_lasso( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
