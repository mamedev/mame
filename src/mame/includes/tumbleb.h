// license:BSD-3-Clause
// copyright-holders:David Haywood,Bryan McPhail

#include "video/decospr.h"
#include "sound/okim6295.h"

class tumbleb_state : public driver_device
{
public:
	tumbleb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_pf1_data(*this, "pf1_data"),
		m_pf2_data(*this, "pf2_data"),
		m_control(*this, "control"),
		m_sprgen(*this, "spritegen"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	optional_shared_ptr<UINT16> m_mainram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_pf1_data;
	required_shared_ptr<UINT16> m_pf2_data;
	optional_shared_ptr<UINT16> m_control;
	optional_device<decospr_device> m_sprgen;

	/* misc */
	int         m_music_command;
	int         m_music_bank;
	int         m_music_is_playing;

	/* video-related */
	tilemap_t   *m_pf1_tilemap;
	tilemap_t   *m_pf1_alt_tilemap;
	tilemap_t   *m_pf2_tilemap;
	tilemap_t   *m_pf2_alt_tilemap;
	UINT16      m_control_0[8];
	UINT16      m_tilebank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT8 m_semicom_prot_offset;
	UINT16 m_protbase;
	DECLARE_WRITE16_MEMBER(tumblepb_oki_w);
	DECLARE_READ16_MEMBER(tumblepb_prot_r);
	DECLARE_READ16_MEMBER(tumblepopb_controls_r);
	DECLARE_READ16_MEMBER(semibase_unknown_r);
	DECLARE_WRITE16_MEMBER(jumpkids_sound_w);
	DECLARE_WRITE16_MEMBER(semicom_soundcmd_w);
	DECLARE_WRITE8_MEMBER(oki_sound_bank_w);
	DECLARE_WRITE8_MEMBER(jumpkids_oki_bank_w);
	DECLARE_READ8_MEMBER(prot_io_r);
	DECLARE_WRITE8_MEMBER(prot_io_w);
	DECLARE_READ16_MEMBER(bcstory_1a0_read);
	DECLARE_WRITE16_MEMBER(bcstory_tilebank_w);
	DECLARE_WRITE16_MEMBER(chokchok_tilebank_w);
	DECLARE_WRITE16_MEMBER(wlstar_tilebank_w);
	DECLARE_WRITE16_MEMBER(suprtrio_tilebank_w);
	DECLARE_WRITE16_MEMBER(tumblepb_pf1_data_w);
	DECLARE_WRITE16_MEMBER(tumblepb_pf2_data_w);
	DECLARE_WRITE16_MEMBER(fncywld_pf1_data_w);
	DECLARE_WRITE16_MEMBER(fncywld_pf2_data_w);
	DECLARE_WRITE16_MEMBER(tumblepb_control_0_w);
	DECLARE_WRITE16_MEMBER(pangpang_pf1_data_w);
	DECLARE_WRITE16_MEMBER(pangpang_pf2_data_w);
	DECLARE_WRITE16_MEMBER(tumbleb2_soundmcu_w);
	DECLARE_DRIVER_INIT(dquizgo);
	DECLARE_DRIVER_INIT(jumpkids);
	DECLARE_DRIVER_INIT(htchctch);
	DECLARE_DRIVER_INIT(wlstar);
	DECLARE_DRIVER_INIT(suprtrio);
	DECLARE_DRIVER_INIT(tumblepb);
	DECLARE_DRIVER_INIT(tumblepba);
	DECLARE_DRIVER_INIT(bcstory);
	DECLARE_DRIVER_INIT(wondl96);
	DECLARE_DRIVER_INIT(tumbleb2);
	DECLARE_DRIVER_INIT(chokchok);
	DECLARE_DRIVER_INIT(fncywld);
	DECLARE_DRIVER_INIT(carket);
	TILEMAP_MAPPER_MEMBER(tumblep_scan);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fncywld_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_fncywld_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fncywld_fg_tile_info);
	TILE_GET_INFO_MEMBER(pangpang_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(pangpang_get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(pangpang_get_fg_tile_info);
	DECLARE_MACHINE_START(tumbleb);
	DECLARE_MACHINE_RESET(tumbleb);
	DECLARE_VIDEO_START(tumblepb);
	DECLARE_VIDEO_START(fncywld);
	DECLARE_MACHINE_RESET(htchctch);
	DECLARE_VIDEO_START(suprtrio);
	DECLARE_VIDEO_START(pangpang);
	DECLARE_VIDEO_START(sdfight);
	UINT32 screen_update_tumblepb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_jumpkids(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_fncywld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_semicom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_suprtrio(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pangpang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_semicom_altoffsets(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bcstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_semibase(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sdfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(tumbleb2_interrupt);
	void tumbleb_tilemap_redraw();
	inline void get_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base);
	inline void get_fncywld_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base);
	inline void pangpang_get_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base );
	inline void pangpang_get_bg2x_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base );
	void tumbleb_draw_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pf1x_offs, int pf1y_offs, int pf2x_offs, int pf2y_offs);
	void tumbleb2_set_music_bank( int bank );
	void tumbleb2_play_sound( okim6295_device *oki, int data );
	void process_tumbleb2_music_command( okim6295_device *oki, int data );
	void tumblepb_gfx_rearrange(int rgn);
	void suprtrio_decrypt_code();
	void suprtrio_decrypt_gfx();
};
