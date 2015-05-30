// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari

#include "sound/sp0256.h"


class sauro_state : public driver_device
{
public:
	sauro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sp0256(*this, "speech"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<sp0256_device> m_sp0256;

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	optional_shared_ptr<UINT8> m_videoram2;
	optional_shared_ptr<UINT8> m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_palette_bank;

	// common
	DECLARE_WRITE8_MEMBER(coin1_w);
	DECLARE_WRITE8_MEMBER(coin2_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(scroll_bg_w);

	// sauro specific
	DECLARE_WRITE8_MEMBER(sauro_sound_command_w);
	DECLARE_READ8_MEMBER(sauro_sound_command_r);
	DECLARE_WRITE8_MEMBER(sauro_palette_bank_w);
	DECLARE_WRITE8_MEMBER(sauro_scroll_fg_w);
	DECLARE_WRITE8_MEMBER(sauro_videoram2_w);
	DECLARE_WRITE8_MEMBER(sauro_colorram2_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);

	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_fg);

	DECLARE_DRIVER_INIT(tecfri);
	DECLARE_VIDEO_START(trckydoc);
	DECLARE_VIDEO_START(sauro);

	UINT32 screen_update_trckydoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sauro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sauro_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void trckydoc_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
