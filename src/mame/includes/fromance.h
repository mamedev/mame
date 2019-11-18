// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi. Bryan McPhail, Nicola Salmoria, Aaron Giles
/***************************************************************************

    Game Driver for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

***************************************************************************/
#ifndef MAME_INCLUDES_FROMANCE_H
#define MAME_INCLUDES_FROMANCE_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "video/vsystem_gga.h"
#include "video/vsystem_spr2.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class fromance_state : public driver_device
{
public:
	fromance_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gga(*this, "gga"),
		m_spr_old(*this, "vsystem_spr_old"),
		m_videoram(*this, "videoram"),
		m_sublatch(*this, "sublatch"),
		m_msm(*this, "msm")
	{ }

	void nekkyoku(machine_config &config);
	void fromance(machine_config &config);
	void idolmj(machine_config &config);

	void init_common();

	DECLARE_WRITE8_MEMBER(fromance_gga_data_w);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<vsystem_gga_device> m_gga;
	optional_device<vsystem_spr2_device> m_spr_old; // only used by pipe dream, split this state up and clean things...

	DECLARE_WRITE8_MEMBER(fromance_gfxreg_w);
	DECLARE_READ8_MEMBER(fromance_videoram_r);
	DECLARE_WRITE8_MEMBER(fromance_videoram_w);
	DECLARE_WRITE8_MEMBER(fromance_scroll_w);

	uint32_t   m_scrolly_ofs;
	uint32_t   m_scrollx_ofs;
	uint32_t   m_scrollx[2];
	uint32_t   m_scrolly[2];
	uint8_t    m_gfxreg;
	uint8_t    m_flipscreen;
	uint8_t    m_flipscreen_old;
	uint8_t    m_selected_videoram;
	uint8_t    m_selected_paletteram;

	DECLARE_VIDEO_START(hatris);
	DECLARE_VIDEO_START(pipedrm);

	uint32_t screen_update_fromance(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pipedrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	/* memory pointers (used by pipedrm) */
	optional_shared_ptr<uint8_t> m_videoram;

	optional_device<generic_latch_8_device> m_sublatch;
	optional_device<msm5205_device> m_msm;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	std::unique_ptr<uint8_t[]>   m_local_videoram[2];
	std::unique_ptr<uint8_t[]>  m_local_paletteram;

	emu_timer *m_crtc_timer;

	/* misc */
	uint8_t    m_portselect;
	uint8_t    m_adpcm_reset;
	uint8_t    m_adpcm_data;
	uint8_t    m_vclk_left;

	/* devices */
	DECLARE_READ8_MEMBER(fromance_busycheck_main_r);
	DECLARE_READ8_MEMBER(fromance_busycheck_sub_r);
	DECLARE_WRITE8_MEMBER(fromance_rombank_w);
	DECLARE_WRITE8_MEMBER(fromance_adpcm_w);
	DECLARE_WRITE8_MEMBER(fromance_portselect_w);
	DECLARE_READ8_MEMBER(fromance_keymatrix_r);
	DECLARE_WRITE8_MEMBER(fromance_coinctr_w);
	DECLARE_READ8_MEMBER(fromance_paletteram_r);
	DECLARE_WRITE8_MEMBER(fromance_paletteram_w);
	DECLARE_WRITE8_MEMBER(fromance_adpcm_reset_w);
	TILE_GET_INFO_MEMBER(get_fromance_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fromance_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_nekkyoku_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_nekkyoku_fg_tile_info);
	DECLARE_MACHINE_START(fromance);
	DECLARE_MACHINE_RESET(fromance);
	DECLARE_VIDEO_START(nekkyoku);
	DECLARE_VIDEO_START(fromance);
	TIMER_CALLBACK_MEMBER(crtc_interrupt_gen);
	inline void get_fromance_tile_info(tile_data &tileinfo, int tile_index, int layer);
	inline void get_nekkyoku_tile_info(tile_data &tileinfo, int tile_index, int layer);
	void crtc_refresh();
	DECLARE_WRITE_LINE_MEMBER(fromance_adpcm_int);
	void fromance_main_map(address_map &map);
	void fromance_sub_io_map(address_map &map);
	void fromance_sub_map(address_map &map);
	void idolmj_sub_io_map(address_map &map);
	void nekkyoku_main_map(address_map &map);
	void nekkyoku_sub_io_map(address_map &map);
	void nekkyoku_sub_map(address_map &map);
};

#endif // MAME_INCLUDES_FROMANCE_H
