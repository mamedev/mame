// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, Uki
/*************************************************************************

    Ojanko High School & other Video System mahjong series

*************************************************************************/
#include "sound/msm5205.h"

class ojankohs_state : public driver_device
{
public:
	ojankohs_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	optional_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_colorram;
	optional_shared_ptr<UINT8> m_paletteram;

	/* video-related */
	tilemap_t  *m_tilemap;
	bitmap_ind16 m_tmpbitmap;
	int       m_gfxreg;
	int       m_flipscreen;
	int       m_flipscreen_old;
	int       m_scrollx;
	int       m_scrolly;
	int       m_screen_refresh;

	/* misc */
	int       m_portselect;
	int       m_adpcm_reset;
	int       m_adpcm_data;
	int       m_vclk_left;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(ojankohs_rombank_w);
	DECLARE_WRITE8_MEMBER(ojankoy_rombank_w);
	DECLARE_WRITE8_MEMBER(ojankohs_msm5205_w);
	DECLARE_WRITE8_MEMBER(ojankoc_ctrl_w);
	DECLARE_WRITE8_MEMBER(ojankohs_portselect_w);
	DECLARE_READ8_MEMBER(ojankohs_keymatrix_r);
	DECLARE_READ8_MEMBER(ojankoc_keymatrix_r);
	DECLARE_READ8_MEMBER(ccasino_dipsw3_r);
	DECLARE_READ8_MEMBER(ccasino_dipsw4_r);
	DECLARE_WRITE8_MEMBER(ojankoy_coinctr_w);
	DECLARE_WRITE8_MEMBER(ccasino_coinctr_w);
	DECLARE_WRITE8_MEMBER(ojankohs_palette_w);
	DECLARE_WRITE8_MEMBER(ccasino_palette_w);
	DECLARE_WRITE8_MEMBER(ojankoc_palette_w);
	DECLARE_WRITE8_MEMBER(ojankohs_videoram_w);
	DECLARE_WRITE8_MEMBER(ojankohs_colorram_w);
	DECLARE_WRITE8_MEMBER(ojankohs_gfxreg_w);
	DECLARE_WRITE8_MEMBER(ojankohs_flipscreen_w);
	DECLARE_WRITE8_MEMBER(ojankoc_videoram_w);
	DECLARE_WRITE8_MEMBER(ojankohs_adpcm_reset_w);
	DECLARE_READ8_MEMBER(ojankohs_ay8910_0_r);
	DECLARE_READ8_MEMBER(ojankohs_ay8910_1_r);
	TILE_GET_INFO_MEMBER(ojankohs_get_tile_info);
	TILE_GET_INFO_MEMBER(ojankoy_get_tile_info);
	virtual void machine_reset() override;
	DECLARE_MACHINE_START(ojankohs);
	DECLARE_VIDEO_START(ojankohs);
	DECLARE_MACHINE_START(ojankoy);
	DECLARE_VIDEO_START(ojankoy);
	DECLARE_PALETTE_INIT(ojankoy);
	DECLARE_MACHINE_START(ojankoc);
	DECLARE_VIDEO_START(ojankoc);
	DECLARE_MACHINE_START(common);
	UINT32 screen_update_ojankohs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ojankoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ojankoc_flipscreen( address_space &space, int data );
	DECLARE_WRITE_LINE_MEMBER(ojankohs_adpcm_int);
};
