// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,Jarek Parchanski
#include "sound/ay8910.h"
#include "sound/msm5205.h"

class gsword_state : public driver_device
{
public:
	gsword_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_ay0(*this, "ay1"),
		m_ay1(*this, "ay2"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritetile_ram(*this, "spritetile_ram"),
		m_spritexy_ram(*this, "spritexy_ram"),
		m_spriteattrib_ram(*this, "spriteattram"),
		m_videoram(*this, "videoram"),
		m_cpu2_ram(*this, "cpu2_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	required_device<ay8910_device> m_ay0;
	required_device<ay8910_device> m_ay1;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_spritetile_ram;
	required_shared_ptr<UINT8> m_spritexy_ram;
	required_shared_ptr<UINT8> m_spriteattrib_ram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_cpu2_ram;

	int m_coins; //currently initialized but not used
	int m_fake8910_0;
	int m_fake8910_1;
	int m_nmi_enable;
	int m_protect_hack;
	int m_charbank;
	int m_charpalbank;
	int m_flipscreen;
	tilemap_t *m_bg_tilemap;

	// common
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(charbank_w);
	DECLARE_WRITE8_MEMBER(videoctrl_w);
	DECLARE_WRITE8_MEMBER(scroll_w);
	DECLARE_WRITE8_MEMBER(adpcm_soundcommand_w);
	DECLARE_WRITE8_MEMBER(nmi_set_w);
	DECLARE_WRITE8_MEMBER(ay8910_control_port_0_w);
	DECLARE_WRITE8_MEMBER(ay8910_control_port_1_w);
	DECLARE_READ8_MEMBER(fake_0_r);
	DECLARE_READ8_MEMBER(fake_1_r);

	// gsword specific
	DECLARE_READ8_MEMBER(gsword_hack_r);
	DECLARE_WRITE8_MEMBER(gsword_adpcm_data_w);
	DECLARE_READ8_MEMBER(gsword_8741_2_r);
	DECLARE_READ8_MEMBER(gsword_8741_3_r);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	DECLARE_DRIVER_INIT(gsword);
	DECLARE_DRIVER_INIT(gsword2);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(gsword);
	DECLARE_PALETTE_INIT(josvolly);

	UINT32 screen_update_gsword(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gsword_snd_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
