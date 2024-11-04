// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Kick Goal - Action Hollywood

*************************************************************************/
#ifndef MAME_TCH_KICKGOAL_H
#define MAME_TCH_KICKGOAL_H

#pragma once

#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "tilemap.h"

class kickgoal_state : public driver_device
{
public:
	kickgoal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_bg2ram(*this, "bg2ram"),
		m_scrram(*this, "scrram"),
		m_spriteram(*this, "spriteram"),
		m_eeprom(*this, "eeprom") ,
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_okibank(*this, "okibank"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void kickgoal(machine_config &config);
	void actionhw(machine_config &config);

	void init_kickgoal();
	void init_actionhw();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void fgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg2ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void actionhw_snd_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void soundio_port_a_w(u8 data);
	u8 soundio_port_b_r();
	void soundio_port_b_w(u8 data);
	u8 soundio_port_c_r();
	void soundio_port_c_w(u8 data);
	void to_pic_w(u16 data);

	TILE_GET_INFO_MEMBER(get_kickgoal_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_actionhw_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_8x8);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_16x16);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_32x32);
	DECLARE_VIDEO_START(kickgoal);
	DECLARE_VIDEO_START(actionhw);

	INTERRUPT_GEN_MEMBER(kickgoal_interrupt);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;

	/* video-related */
	tilemap_t     *m_fgtm = nullptr;
	tilemap_t     *m_bgtm = nullptr;
	tilemap_t     *m_bg2tm = nullptr;

	/* misc */
	int         m_snd_new = 0;
	int         m_snd_sam[4]{};

	u8 m_pic_portc = 0;
	u8 m_pic_portb = 0;
	int m_sound_command_sent = 0;

	int m_fg_base = 0;

	int m_bg_base = 0;
	int m_bg_mask = 0;

	int m_bg2_base = 0;
	int m_bg2_mask = 0;
	int m_bg2_region = 0;

	int m_sprbase = 0;

	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	/* memory pointers */
	required_shared_ptr<u16> m_fgram;
	required_shared_ptr<u16> m_bgram;
	required_shared_ptr<u16> m_bg2ram;
	required_shared_ptr<u16> m_scrram;
	required_shared_ptr<u16> m_spriteram;

	/* devices */
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<cpu_device> m_maincpu;
	required_device<pic16c57_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_memory_bank m_okibank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};

#endif // MAME_TCH_KICKGOAL_H
