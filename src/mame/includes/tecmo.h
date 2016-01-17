// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/msm5205.h"
#include "video/tecmo_spr.h"

class tecmo_state : public driver_device
{
public:
	tecmo_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_txvideoram(*this, "txvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;

	required_shared_ptr<UINT8> m_txvideoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_spriteram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 m_fgscroll[3];
	UINT8 m_bgscroll[3];
	int m_adpcm_pos;
	int m_adpcm_end;
	int m_adpcm_data;
	int m_video_type;

	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_ack_w);
	DECLARE_WRITE8_MEMBER(adpcm_end_w);
	DECLARE_READ8_MEMBER(dswa_l_r);
	DECLARE_READ8_MEMBER(dswa_h_r);
	DECLARE_READ8_MEMBER(dswb_l_r);
	DECLARE_READ8_MEMBER(dswb_h_r);
	DECLARE_WRITE8_MEMBER(txvideoram_w);
	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(fgscroll_w);
	DECLARE_WRITE8_MEMBER(bgscroll_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(adpcm_start_w);
	DECLARE_WRITE8_MEMBER(adpcm_vol_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	DECLARE_DRIVER_INIT(silkworm);
	DECLARE_DRIVER_INIT(rygar);
	DECLARE_DRIVER_INIT(backfirt);
	DECLARE_DRIVER_INIT(gemini);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(gemini_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(gemini_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
