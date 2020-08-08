// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, Uki
/*************************************************************************

    Ojanko High School & other Video System mahjong series

*************************************************************************/
#ifndef MAME_INCLUDES_OJANKOHS_H
#define MAME_INCLUDES_OJANKOHS_H

#pragma once

#include "sound/msm5205.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class ojankohs_state : public driver_device
{
public:
	ojankohs_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_coin(*this, "coin"),
		m_inputs_p1(*this, {"p1_0", "p1_1", "p1_2", "p1_3"}),
		m_inputs_p2(*this, {"p2_0", "p2_1", "p2_2", "p2_3"}),
		m_inputs_p1_extra(*this, "p1_4"),
		m_inputs_p2_extra(*this, "p2_4"),
		m_dsw1(*this, "dsw1"), m_dsw2(*this, "dsw2"),
		m_dsw3(*this, "dsw3"), m_dsw4(*this, "dsw4")
	{ }

	void ojankohs(machine_config &config);
	void ccasino(machine_config &config);
	void ojankoc(machine_config &config);
	void ojankoy(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
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
	uint8_t   m_port_select;
	int       m_adpcm_reset;
	int       m_adpcm_data;
	int       m_vclk_left;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport m_coin;
	required_ioport_array<4> m_inputs_p1;
	required_ioport_array<4> m_inputs_p2;
	optional_ioport m_inputs_p1_extra;
	optional_ioport m_inputs_p2_extra;
	required_ioport m_dsw1;
	required_ioport m_dsw2;
	optional_ioport m_dsw3;
	optional_ioport m_dsw4;

	void ojankohs_rombank_w(uint8_t data);
	void ojankoy_rombank_w(uint8_t data);
	void ojankohs_msm5205_w(uint8_t data);
	void ojankoc_ctrl_w(uint8_t data);
	void port_select_w(uint8_t data);
	uint8_t keymatrix_p1_r();
	uint8_t keymatrix_p2_r();
	uint8_t ojankoc_keymatrix_p1_r();
	uint8_t ojankoc_keymatrix_p2_r();
	uint8_t ccasino_dipsw3_r();
	uint8_t ccasino_dipsw4_r();
	void ojankoy_coinctr_w(uint8_t data);
	void ccasino_coinctr_w(uint8_t data);
	void ojankohs_palette_w(offs_t offset, uint8_t data);
	void ccasino_palette_w(offs_t offset, uint8_t data);
	void ojankoc_palette_w(offs_t offset, uint8_t data);
	void ojankohs_videoram_w(offs_t offset, uint8_t data);
	void ojankohs_colorram_w(offs_t offset, uint8_t data);
	void ojankohs_gfxreg_w(uint8_t data);
	void ojankohs_flipscreen_w(uint8_t data);
	void ojankoc_videoram_w(offs_t offset, uint8_t data);
	void ojankohs_adpcm_reset_w(uint8_t data);
	uint8_t ojankohs_dipsw1_r();
	uint8_t ojankohs_dipsw2_r();
	TILE_GET_INFO_MEMBER(ojankohs_get_tile_info);
	TILE_GET_INFO_MEMBER(ojankoy_get_tile_info);
	DECLARE_MACHINE_START(ojankohs);
	DECLARE_VIDEO_START(ojankohs);
	DECLARE_MACHINE_START(ojankoy);
	DECLARE_VIDEO_START(ojankoy);
	void ojankoy_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(ccasino);
	DECLARE_MACHINE_START(ojankoc);
	DECLARE_VIDEO_START(ojankoc);
	DECLARE_MACHINE_START(common);
	uint32_t screen_update_ojankohs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ojankoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ojankoc_flipscreen(int data);
	DECLARE_WRITE_LINE_MEMBER(ojankohs_adpcm_int);

	void ccasino_io_map(address_map &map);
	void ojankoc_io_map(address_map &map);
	void ojankoc_map(address_map &map);
	void ojankohs_io_map(address_map &map);
	void ojankohs_map(address_map &map);
	void ojankoy_io_map(address_map &map);
	void ojankoy_map(address_map &map);
};

#endif // MAME_INCLUDES_OJANKOHS_H
