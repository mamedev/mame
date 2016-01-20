// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Karate Champ

*************************************************************************/
#include "sound/msm5205.h"

class kchamp_state : public driver_device
{
public:
	kchamp_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_nmi_enable;
	int        m_sound_nmi_enable;
	int        m_msm_data;
	int        m_msm_play_lo_nibble;
	int        m_counter;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(sound_reset_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(sound_msm_w);
	DECLARE_READ8_MEMBER(sound_reset_r);
	DECLARE_WRITE8_MEMBER(kc_sound_control_w);
	DECLARE_WRITE8_MEMBER(kchamp_videoram_w);
	DECLARE_WRITE8_MEMBER(kchamp_colorram_w);
	DECLARE_WRITE8_MEMBER(kchamp_flipscreen_w);
	DECLARE_WRITE8_MEMBER(sound_control_w);
	DECLARE_DRIVER_INIT(kchampvs);
	DECLARE_DRIVER_INIT(kchampvs2);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(kchamp);
	DECLARE_MACHINE_START(kchampvs);
	DECLARE_MACHINE_START(kchamp);
	UINT32 screen_update_kchampvs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kchamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(kc_interrupt);
	INTERRUPT_GEN_MEMBER(sound_int);
	void kchamp_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void kchampvs_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void decrypt_code();
	DECLARE_WRITE_LINE_MEMBER(msmint);
	required_device<cpu_device> m_maincpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;
};
