// license:BSD-3-Clause
// copyright-holders:David Haywood, Farfetch'd
#include "sound/okim6295.h"

class speedspn_state : public driver_device
{
public:
	speedspn_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_prgbank(*this, "prgbank"),
		m_okibank(*this, "okibank"),
		m_attram(*this, "attram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_memory_bank m_prgbank;
	required_memory_bank m_okibank;
	required_shared_ptr<UINT8> m_attram;

	tilemap_t *m_tilemap;
	bool m_display_disable;
	UINT32 m_bank_vidram;
	std::vector<UINT8> m_vidram;
	DECLARE_READ8_MEMBER(irq_ack_r);
	DECLARE_WRITE8_MEMBER(rombank_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(vidram_w);
	DECLARE_WRITE8_MEMBER(attram_w);
	DECLARE_READ8_MEMBER(vidram_r);
	DECLARE_WRITE8_MEMBER(vidram_bank_w);
	DECLARE_WRITE8_MEMBER(display_disable_w);
	DECLARE_WRITE8_MEMBER(okibank_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
