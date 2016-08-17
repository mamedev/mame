// license:BSD-3-Clause
// copyright-holders:smf, David Haywood

#include "audio/irem.h"

class m62_state : public driver_device
{
public:
	m62_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_m62_tileram(*this, "m62_tileram"),
		m_m62_textram(*this, "m62_textram"),
		m_scrollram(*this, "scrollram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_audio(*this, "irem_audio")
	{ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;

	required_shared_ptr<UINT8> m_m62_tileram;
	optional_shared_ptr<UINT8> m_m62_textram;
	optional_shared_ptr<UINT8> m_scrollram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	tilemap_t*             m_fg_tilemap;
	int                  m_flipscreen;

	const UINT8          *m_sprite_height_prom;
	INT32                m_m62_background_hscroll;
	INT32                m_m62_background_vscroll;
	UINT8                m_kidniki_background_bank;
	INT32                m_kidniki_text_vscroll;
	int                  m_ldrun3_topbottom_mask;
	INT32                m_spelunkr_palbank;

	/* misc */
	int                 m_ldrun2_bankswap;  //ldrun2
	int                 m_bankcontrol[2];   //ldrun2
	DECLARE_READ8_MEMBER(ldrun2_bankswitch_r);
	DECLARE_WRITE8_MEMBER(ldrun2_bankswitch_w);
	DECLARE_READ8_MEMBER(ldrun3_prot_5_r);
	DECLARE_READ8_MEMBER(ldrun3_prot_7_r);
	DECLARE_WRITE8_MEMBER(ldrun4_bankswitch_w);
	DECLARE_WRITE8_MEMBER(kidniki_bankswitch_w);
	DECLARE_WRITE8_MEMBER(spelunkr_bankswitch_w);
	DECLARE_WRITE8_MEMBER(spelunk2_bankswitch_w);
	DECLARE_WRITE8_MEMBER(youjyudn_bankswitch_w);
	DECLARE_WRITE8_MEMBER(m62_flipscreen_w);
	DECLARE_WRITE8_MEMBER(m62_hscroll_low_w);
	DECLARE_WRITE8_MEMBER(m62_hscroll_high_w);
	DECLARE_WRITE8_MEMBER(m62_vscroll_low_w);
	DECLARE_WRITE8_MEMBER(m62_vscroll_high_w);
	DECLARE_WRITE8_MEMBER(m62_tileram_w);
	DECLARE_WRITE8_MEMBER(m62_textram_w);
	DECLARE_WRITE8_MEMBER(kungfum_tileram_w);
	DECLARE_WRITE8_MEMBER(ldrun3_topbottom_mask_w);
	DECLARE_WRITE8_MEMBER(kidniki_text_vscroll_low_w);
	DECLARE_WRITE8_MEMBER(kidniki_text_vscroll_high_w);
	DECLARE_WRITE8_MEMBER(kidniki_background_bank_w);
	DECLARE_WRITE8_MEMBER(spelunkr_palbank_w);
	DECLARE_WRITE8_MEMBER(spelunk2_gfxport_w);
	DECLARE_WRITE8_MEMBER(horizon_scrollram_w);
	DECLARE_DRIVER_INIT(youjyudn);
	DECLARE_DRIVER_INIT(spelunkr);
	DECLARE_DRIVER_INIT(ldrun2);
	DECLARE_DRIVER_INIT(ldrun4);
	DECLARE_DRIVER_INIT(spelunk2);
	DECLARE_DRIVER_INIT(kidniki);
	DECLARE_DRIVER_INIT(battroad);
	TILE_GET_INFO_MEMBER(get_kungfum_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_ldrun_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_ldrun2_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_battroad_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_battroad_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_ldrun4_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_lotlot_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_lotlot_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_kidniki_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_kidniki_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_spelunkr_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_spelunkr_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_spelunk2_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_youjyudn_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_youjyudn_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_horizon_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(m62);
	DECLARE_VIDEO_START(kungfum);
	DECLARE_VIDEO_START(battroad);
	DECLARE_PALETTE_INIT(battroad);
	DECLARE_VIDEO_START(ldrun2);
	DECLARE_VIDEO_START(ldrun4);
	DECLARE_VIDEO_START(lotlot);
	DECLARE_PALETTE_INIT(lotlot);
	DECLARE_VIDEO_START(kidniki);
	DECLARE_VIDEO_START(spelunkr);
	DECLARE_VIDEO_START(spelunk2);
	DECLARE_PALETTE_INIT(spelunk2);
	DECLARE_VIDEO_START(youjyudn);
	DECLARE_VIDEO_START(horizon);
	UINT32 screen_update_ldrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kungfum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_battroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ldrun3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ldrun4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_lotlot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kidniki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spelunkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spelunk2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_youjyudn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_horizon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void m62_amplify_contrast(palette_t *palette, UINT32 numcolors);
	void register_savestate(  );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int colormask, int prioritymask, int priority );
	void m62_start( tilemap_get_info_delegate tile_get_info, int rows, int cols, int x1, int y1, int x2, int y2 );
	void m62_textlayer( tilemap_get_info_delegate tile_get_info, int rows, int cols, int x1, int y1, int x2, int y2 );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<irem_audio_device> m_audio;
};
