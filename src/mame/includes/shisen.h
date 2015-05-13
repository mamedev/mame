// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "audio/m72.h"

class shisen_state : public driver_device
{
public:
	shisen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audio (*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_videoram;

	required_device<cpu_device> m_maincpu;
	required_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_gfxbank;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(sichuan2_dsw1_r);
	DECLARE_WRITE8_MEMBER(sichuan2_coin_w);
	DECLARE_WRITE8_MEMBER(sichuan2_videoram_w);
	DECLARE_WRITE8_MEMBER(sichuan2_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sichuan2_paletteram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	UINT32 screen_update_sichuan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
