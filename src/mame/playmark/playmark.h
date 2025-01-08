// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Pierpaolo Prazzoli, Quench
#ifndef MAME_PLAYMARK_PLAYMARK_H
#define MAME_PLAYMARK_PLAYMARK_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "tilemap.h"

class playmark_base_state : public driver_device // base class for powerbal.cpp, too
{
public:
	playmark_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_sprtranspen(0),
		m_oki(*this, "oki"),
		m_okibank(*this, "okibank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

protected:
	// memory pointers
	required_shared_ptr<u16> m_bgvideoram;
	required_shared_ptr<u16> m_spriteram;

	// video-related
	tilemap_t *m_bg_tilemap;

	s8 m_xoffset;
	s8 m_yoffset;
	u8 m_sprtranspen;

	// misc
	u8 m_oki_numbanks;
	void configure_oki_banks();

	// devices
	required_device<okim6295_device> m_oki;
	required_memory_bank m_okibank;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

class playmark_state : public playmark_base_state
{
public:
	playmark_state(const machine_config &mconfig, device_type type, const char *tag) :
		playmark_base_state(mconfig, type, tag),
		m_fgvideoram(*this, "fgvideoram"),
		m_txtvideoram(*this, "txtvideoram"),
		m_rowscroll(*this, "rowscroll"),
		m_audio_pic(*this, "audiopic"),
		m_audio_mcs(*this, "audiomcs"),
		m_soundlatch(*this, "soundlatch"),
		m_eeprom(*this, "eeprom"),
		m_ticket(*this, "ticket"),
		m_token(*this, "token")
	{ }

	void wbeachvl_mcs(machine_config &config);
	void wbeachvl_pic(machine_config &config);
	void hrdtimes(machine_config &config);
	void luckboomh(machine_config &config);
	void bigtwin(machine_config &config);
	void hotmind(machine_config &config);
	void bigtwinb(machine_config &config);
	void excelsr(machine_config &config);

	void init_pic_decode();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u16> m_fgvideoram;
	required_shared_ptr<u16> m_txtvideoram;
	optional_shared_ptr<u16> m_rowscroll;// wbeachvl

	// video-related
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;

	// bigtwin and excelsr
	s32 m_bgscrollx = 0;
	s16 m_bgscrolly = 0;
	bool m_bg_enable = false;
	bool m_bg_full_size = false;

	// wbeachvl
	u32 m_fgscrollx = 0;
	bool m_fg_rowscroll_enable = false;

	// all
	u16 m_pri_masks[3];
	u16 m_scroll[7];

	// misc
	// all
	u8 m_snd_command = 0;
	u8 m_snd_flag = 0;
	u8 m_oki_control = 0;
	u8 m_oki_command = 0;

	u8 m_dispenser_latch = 0; // hotmind luckboomh

	// devices
	optional_device<pic16c57_device> m_audio_pic; // all but wbeachvla;
	optional_device<i87c51_device> m_audio_mcs; // wbeachvla
	optional_device<generic_latch_8_device> m_soundlatch; // wbeachvla
	optional_device<eeprom_serial_93cxx_device> m_eeprom; // wbeachvl hotmind
	optional_device<ticket_dispenser_device> m_ticket; // hotmind luckboomh
	optional_device<ticket_dispenser_device> m_token; // hotmind luckboomh

	void coinctrl_w(u8 data); // bigtwin excelsr hrdtimes
	void wbeachvl_coin_eeprom_w(u8 data); // wbeachvl
	void hotmind_coin_eeprom_w(u8 data); // hotmind
	void luckboomh_dispenser_w(u8 data); // hotmind luckboomh
	void playmark_snd_command_w(u8 data);
	u8 playmark_snd_command_r();
	u8 playmark_snd_flag_r();
	void playmark_oki_w(u8 data);
	void playmark_snd_control_w(u8 data);
	void hrdtimes_snd_control_w(u8 data); // hrdtimes, hotmind, luckboomh
	void wbeachvl_txvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0); // bigtwin, wbeachvl, excelsr
	void wbeachvl_fgvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0); // bigtwin, wbeachvl, excelsr
	void wbeachvl_bgvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0); // wbeachvl
	void hrdtimes_txvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0); // bigtwinb, hrdtimes, hotmind, luckboomh
	void hrdtimes_fgvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0); // bigtwinb, hrdtimes, hotmind, luckboomh
	void hrdtimes_bgvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0); // bigtwinb, hrdtimes, hotmind, luckboomh
	void bigtwin_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0); // bigtwin
	void wbeachvl_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0); // wbeachvk
	void excelsr_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0); // excelsr
	void hrdtimes_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0); // bigtwinb, hrdtimes, hotmind, luckboomh
	void playmark_oki_banking_w(u8 data);
	uint8_t wbeachvla_snd_command_r(); // wbeachvla
	void wbeachvla_snd_control_w(uint8_t data); // wbeachvla
	TILE_GET_INFO_MEMBER(bigtwin_get_tx_tile_info); // bigtwin, excelsr
	TILE_GET_INFO_MEMBER(bigtwin_get_fg_tile_info); // bigtwin, excelsr
	TILE_GET_INFO_MEMBER(wbeachvl_get_tx_tile_info); // wbeachvk
	TILE_GET_INFO_MEMBER(wbeachvl_get_fg_tile_info); // wbeachvk
	TILE_GET_INFO_MEMBER(wbeachvl_get_bg_tile_info); // wbeachvk
	TILE_GET_INFO_MEMBER(hrdtimes_get_tx_tile_info); // hrdtimes, hotmind, luckboomh
	TILE_GET_INFO_MEMBER(bigtwinb_get_tx_tile_info); // bigtwinb
	TILE_GET_INFO_MEMBER(hrdtimes_get_fg_tile_info); // bigtwinb, hrdtimes, hotmind, luckboomh
	TILE_GET_INFO_MEMBER(hrdtimes_get_bg_tile_info); // bigtwinb, hrdtimes, hotmind, luckboomh
	DECLARE_VIDEO_START(bigtwin);
	DECLARE_VIDEO_START(bigtwinb);
	DECLARE_VIDEO_START(wbeachvl);
	DECLARE_VIDEO_START(excelsr);
	DECLARE_VIDEO_START(hotmind);
	DECLARE_VIDEO_START(hrdtimes);
	DECLARE_VIDEO_START(luckboomh);
	TILEMAP_MAPPER_MEMBER(playmark_tilemap_scan_pages); // hrdtimes
	u32 screen_update_bigtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_bigtwinb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_wbeachvl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_excelsr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_hrdtimes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect); // hrdtimes, hotmind, luckboomh
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int codeshift); // all but bigtwinb
	void bigtwinb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int codeshift); // bigtwinb
	void draw_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect); // bigtwin, excelsr
	u8 playmark_asciitohex(u8 data);
	void playmark_decode_pic_hex_dump(void);

	void bigtwin_main_map(address_map &map) ATTR_COLD;
	void bigtwinb_main_map(address_map &map) ATTR_COLD;
	void excelsr_main_map(address_map &map) ATTR_COLD;
	void hotmind_main_map(address_map &map) ATTR_COLD;
	void hrdtimes_main_map(address_map &map) ATTR_COLD;
	void luckboomh_main_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
	void wbeachvl_main_map(address_map &map) ATTR_COLD;
	void wbeachvla_main_map(address_map &map) ATTR_COLD;

	void wbeachvl_base(machine_config &config);
};

#endif // MAME_PLAYMARK_PLAYMARK_H
