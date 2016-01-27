// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#include "sound/samples.h"

class gotya_state : public driver_device
{
public:
	gotya_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_samples(*this, "samples"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int      m_scroll_bit_8;

	/* sound-related */
	int      m_theme_playing;

	/* devices */
	required_device<samples_device> m_samples;
	DECLARE_WRITE8_MEMBER(gotya_videoram_w);
	DECLARE_WRITE8_MEMBER(gotya_colorram_w);
	DECLARE_WRITE8_MEMBER(gotya_video_control_w);
	DECLARE_WRITE8_MEMBER(gotya_soundlatch_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows_thehand);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(gotya);
	UINT32 screen_update_gotya(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_status_row( bitmap_ind16 &bitmap, const rectangle &cliprect, int sx, int col );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_status( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
