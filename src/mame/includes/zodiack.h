// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#include "emu.h"
#include "cpu/z80/z80.h"

class zodiack_state : public driver_device
{
public:
	zodiack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_videoram(*this, "videoram"),
			m_videoram_2(*this, "videoram_2"),
			m_attributeram(*this, "attributeram"),
			m_spriteram(*this, "spriteram"),
			m_bulletsram(*this, "bulletsram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette")
	{ }

	// in drivers/zodiack.c
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(master_soundlatch_w);
	DECLARE_WRITE8_MEMBER(control_w);

	// in video/zodiack.c
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(videoram2_w);
	DECLARE_WRITE8_MEMBER(attributes_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);

	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	// devices
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_audiocpu;

	// shared pointers
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram_2;
	required_shared_ptr<UINT8> m_attributeram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_bulletsram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// state
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_main_nmi_enabled;
	UINT8 m_sound_nmi_enabled;
	bool m_percuss_hardware;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	DECLARE_DRIVER_INIT(zodiack);
	DECLARE_DRIVER_INIT(percuss);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	DECLARE_PALETTE_INIT(zodiack);
	INTERRUPT_GEN_MEMBER(zodiack_sound_nmi_gen);
	INTERRUPT_GEN_MEMBER(zodiack_main_nmi_gen);
};
