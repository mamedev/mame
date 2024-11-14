// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Punch Out / Super Punch Out / Arm Wrestling

***************************************************************************/
#ifndef MAME_NINTENDO_PUNCHOUT_H
#define MAME_NINTENDO_PUNCHOUT_H

#pragma once

#include "cpu/m6502/rp2a03.h"
#include "machine/rp5c01.h"
#include "machine/rp5h01.h"
#include "sound/vlm5030.h"
#include "emupal.h"
#include "tilemap.h"

class punchout_state : public driver_device
{
public:
	punchout_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_rtc(*this, "rtc"),
		m_rp5h01(*this, "rp5h01"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bg_top_videoram(*this, "bg_top_videoram"),
		m_spr1_ctrlram(*this, "spr1_ctrlram"),
		m_spr2_ctrlram(*this, "spr2_ctrlram"),
		m_palettebank(*this, "palettebank"),
		m_spr1_videoram(*this, "spr1_videoram"),
		m_spr2_videoram(*this, "spr2_videoram"),
		m_bg_bot_videoram(*this, "bg_bot_videoram"),
		m_armwrest_fg_videoram(*this, "armwrest_fgram")
	{ }

	void spnchout(machine_config &config);
	void armwrest(machine_config &config);
	void punchout(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<rp2a03_device> m_audiocpu;
	optional_device<rp5c01_device> m_rtc;
	optional_device<rp5h01_device> m_rp5h01;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bg_top_videoram;
	required_shared_ptr<uint8_t> m_spr1_ctrlram;
	required_shared_ptr<uint8_t> m_spr2_ctrlram;
	required_shared_ptr<uint8_t> m_palettebank;
	required_shared_ptr<uint8_t> m_spr1_videoram;
	required_shared_ptr<uint8_t> m_spr2_videoram;
	required_shared_ptr<uint8_t> m_bg_bot_videoram;
	optional_shared_ptr<uint8_t> m_armwrest_fg_videoram;

	tilemap_t *m_bg_top_tilemap = nullptr;
	tilemap_t *m_bg_bot_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_spr1_tilemap = nullptr;
	tilemap_t *m_spr1_tilemap_flipx = nullptr;
	tilemap_t *m_spr2_tilemap = nullptr;

	bool m_nmi_mask = false;
	uint8_t spunchout_exp_r(offs_t offset);
	void spunchout_exp_w(offs_t offset, uint8_t data);
	void spunchout_rp5h01_reset_w(uint8_t data);
	void spunchout_rp5h01_clock_w(uint8_t data);
	void nmi_mask_w(int state);
	void punchout_bg_top_videoram_w(offs_t offset, uint8_t data);
	void punchout_bg_bot_videoram_w(offs_t offset, uint8_t data);
	void armwrest_fg_videoram_w(offs_t offset, uint8_t data);
	void punchout_spr1_videoram_w(offs_t offset, uint8_t data);
	void punchout_spr2_videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(top_get_info);
	TILE_GET_INFO_MEMBER(armwrest_top_get_info);
	TILE_GET_INFO_MEMBER(bot_get_info);
	TILE_GET_INFO_MEMBER(armwrest_bot_get_info);
	TILE_GET_INFO_MEMBER(bs1_get_info);
	TILE_GET_INFO_MEMBER(bs2_get_info);
	TILE_GET_INFO_MEMBER(armwrest_fg_get_info);
	TILEMAP_MAPPER_MEMBER(armwrest_bs1_scan);
	TILEMAP_MAPPER_MEMBER(armwrest_bs1_scan_flipx);
	virtual void video_start() override ATTR_COLD;
	DECLARE_VIDEO_START(armwrest);
	DECLARE_MACHINE_RESET(spnchout);
	uint32_t screen_update_punchout_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_punchout_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_armwrest_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_armwrest_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_big_sprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palette);
	void armwrest_draw_big_sprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palette);
	void drawbs2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void punchout_copy_top_palette(int bank);
	void punchout_copy_bot_palette(int bank);
	void armwrest_map(address_map &map) ATTR_COLD;
	void punchout_io_map(address_map &map) ATTR_COLD;
	void punchout_map(address_map &map) ATTR_COLD;
	void punchout_sound_map(address_map &map) ATTR_COLD;
	void punchout_vlm_map(address_map &map) ATTR_COLD;
	void spnchout_io_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NINTENDO_PUNCHOUT_H
