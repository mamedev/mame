// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, Uki
/*************************************************************************

    Ojanko High School & other Video System mahjong series

*************************************************************************/
#include "sound/msm5205.h"

class ojankohs_state : public driver_device
{
public:
	ojankohs_state(const machine_config &mconfig, device_type type, const char *tag)
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
	optional_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_paletteram;

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

	void ojankohs_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankoy_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankohs_msm5205_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankoc_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankohs_portselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ojankohs_keymatrix_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ojankoc_keymatrix_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ccasino_dipsw3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ccasino_dipsw4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ojankoy_coinctr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ccasino_coinctr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankohs_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ccasino_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankoc_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankohs_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankohs_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankohs_gfxreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankohs_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankoc_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ojankohs_adpcm_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ojankohs_ay8910_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ojankohs_ay8910_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ojankohs_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ojankoy_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	void machine_start_ojankohs();
	void video_start_ojankohs();
	void machine_start_ojankoy();
	void video_start_ojankoy();
	void palette_init_ojankoy(palette_device &palette);
	void machine_start_ojankoc();
	void video_start_ojankoc();
	void machine_start_common();
	uint32_t screen_update_ojankohs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ojankoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ojankoc_flipscreen( address_space &space, int data );
	void ojankohs_adpcm_int(int state);
};
