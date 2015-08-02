// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "sound/okim6295.h"
#include "video/excellent_spr.h"

class aquarium_state : public driver_device
{
public:
	aquarium_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mid_videoram(*this, "mid_videoram"),
		m_bak_videoram(*this, "bak_videoram"),
		m_txt_videoram(*this, "txt_videoram"),
		m_scroll(*this, "scroll"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_mid_videoram;
	required_shared_ptr<UINT16> m_bak_videoram;
	required_shared_ptr<UINT16> m_txt_videoram;
	required_shared_ptr<UINT16> m_scroll;

	/* video-related */
	tilemap_t  *m_txt_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_bak_tilemap;

	/* misc */
	int m_aquarium_snd_ack;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	DECLARE_READ16_MEMBER(aquarium_coins_r);
	DECLARE_WRITE8_MEMBER(aquarium_snd_ack_w);
	DECLARE_WRITE16_MEMBER(aquarium_sound_w);
	DECLARE_WRITE8_MEMBER(aquarium_z80_bank_w);
	DECLARE_READ8_MEMBER(aquarium_oki_r);
	DECLARE_WRITE8_MEMBER(aquarium_oki_w);
	DECLARE_WRITE16_MEMBER(aquarium_txt_videoram_w);
	DECLARE_WRITE16_MEMBER(aquarium_mid_videoram_w);
	DECLARE_WRITE16_MEMBER(aquarium_bak_videoram_w);
	DECLARE_DRIVER_INIT(aquarium);
	TILE_GET_INFO_MEMBER(get_aquarium_txt_tile_info);
	TILE_GET_INFO_MEMBER(get_aquarium_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_aquarium_bak_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_aquarium(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	UINT8 aquarium_snd_bitswap( UINT8 scrambled_data );
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<excellent_spr_device> m_sprgen;

};
