#include "audio/seibu.h"
#include "sound/msm5205.h"

class cabal_state : public driver_device
{
public:
	cabal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_adpcm1(*this, "adpcm1"),
		m_adpcm2(*this, "adpcm2"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_colorram;
	required_shared_ptr<UINT16> m_videoram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	optional_device<seibu_adpcm_device> m_adpcm1;
	optional_device<seibu_adpcm_device> m_adpcm2;
	optional_device<msm5205_device> m_msm1;
	optional_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	tilemap_t *m_background_layer;
	tilemap_t *m_text_layer;
	int m_sound_command1;
	int m_sound_command2;
	int m_last[4];
	DECLARE_WRITE16_MEMBER(cabalbl_sndcmd_w);
	DECLARE_WRITE16_MEMBER(track_reset_w);
	DECLARE_READ16_MEMBER(track_r);
	DECLARE_WRITE16_MEMBER(cabal_sound_irq_trigger_word_w);
	DECLARE_WRITE16_MEMBER(cabalbl_sound_irq_trigger_word_w);
	DECLARE_READ8_MEMBER(cabalbl_snd2_r);
	DECLARE_READ8_MEMBER(cabalbl_snd1_r);
	DECLARE_WRITE8_MEMBER(cabalbl_coin_w);
	DECLARE_WRITE16_MEMBER(cabal_flipscreen_w);
	DECLARE_WRITE16_MEMBER(cabal_background_videoram16_w);
	DECLARE_WRITE16_MEMBER(cabal_text_videoram16_w);
	DECLARE_WRITE8_MEMBER(cabalbl_1_adpcm_w);
	DECLARE_WRITE8_MEMBER(cabalbl_2_adpcm_w);
	DECLARE_DRIVER_INIT(cabal);
	DECLARE_DRIVER_INIT(cabalbl2);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	virtual void video_start();
	DECLARE_MACHINE_RESET(cabalbl);
	UINT32 screen_update_cabal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void seibu_sound_bootleg(const char *cpu,int length);
};
