// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi. Bryan McPhail, Nicola Salmoria, Aaron Giles
/***************************************************************************

    Game Driver for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

***************************************************************************/
#ifndef MAME_VSYSTEM_FROMANCE_H
#define MAME_VSYSTEM_FROMANCE_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "vsystem_gga.h"
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
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gga(*this, "gga"),
		m_rombank(*this, "rombank"),
		m_sublatch(*this, "sublatch"),
		m_msm(*this, "msm")
	{ }

	void nekkyoku(machine_config &config);
	void fromance(machine_config &config);
	void idolmj(machine_config &config);

	void init_common();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(fromance);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<vsystem_gga_device> m_gga;
	required_memory_bank m_rombank;

	void fromance_gfxreg_w(uint8_t data);
	uint8_t fromance_videoram_r(offs_t offset);
	void fromance_videoram_w(offs_t offset, uint8_t data);
	void fromance_scroll_w(offs_t offset, uint8_t data);
	void fromance_gga_data_w(offs_t offset, uint8_t data);

	uint32_t   m_scrolly_ofs = 0;
	uint32_t   m_scrollx_ofs = 0;
	uint32_t   m_scrollx[2]{};
	uint32_t   m_scrolly[2]{};
	uint8_t    m_gfxreg = 0;
	uint8_t    m_flipscreen = 0;
	uint8_t    m_flipscreen_old = 0;
	uint8_t    m_selected_videoram = 0;
	uint8_t    m_selected_paletteram = 0;
	tilemap_t  *m_bg_tilemap = nullptr;
	tilemap_t  *m_fg_tilemap = nullptr;

	uint32_t screen_update_fromance(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	optional_device<generic_latch_8_device> m_sublatch;
	optional_device<msm5205_device> m_msm;

	// video-related
	std::unique_ptr<uint8_t[]>   m_local_videoram[2];
	std::unique_ptr<uint8_t[]>  m_local_paletteram;

	emu_timer *m_crtc_timer;

	// misc
	uint8_t    m_portselect = 0;
	uint8_t    m_adpcm_reset = 0;
	uint8_t    m_adpcm_data = 0;
	uint8_t    m_vclk_left = 0;

	// devices
	uint8_t fromance_busycheck_main_r();
	uint8_t fromance_busycheck_sub_r();
	void fromance_rombank_w(uint8_t data);
	void fromance_adpcm_w(uint8_t data);
	void fromance_portselect_w(uint8_t data);
	uint8_t fromance_keymatrix_r();
	void fromance_coinctr_w(uint8_t data);
	uint8_t fromance_paletteram_r(offs_t offset);
	void fromance_paletteram_w(offs_t offset, uint8_t data);
	void fromance_adpcm_reset_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_fromance_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fromance_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_nekkyoku_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_nekkyoku_fg_tile_info);
	DECLARE_VIDEO_START(nekkyoku);
	TIMER_CALLBACK_MEMBER(crtc_interrupt_gen);
	inline void get_fromance_tile_info(tile_data &tileinfo, int tile_index, int layer);
	inline void get_nekkyoku_tile_info(tile_data &tileinfo, int tile_index, int layer);
	void crtc_refresh();
	void fromance_adpcm_int(int state);
	void fromance_main_map(address_map &map);
	void fromance_sub_io_map(address_map &map);
	void fromance_sub_map(address_map &map);
	void idolmj_sub_io_map(address_map &map);
	void nekkyoku_main_map(address_map &map);
	void nekkyoku_sub_io_map(address_map &map);
	void nekkyoku_sub_map(address_map &map);
};

#endif // MAME_VSYSTEM_FROMANCE_H
