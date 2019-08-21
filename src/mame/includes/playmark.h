// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Pierpaolo Prazzoli, Quench
#ifndef MAME_INCLUDES_PLAYMARK_H
#define MAME_INCLUDES_PLAYMARK_H

#pragma once

#include "sound/okim6295.h"
#include "machine/eepromser.h"
#include "machine/ticket.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "emupal.h"
#include "tilemap.h"

class playmark_state : public driver_device
{
public:
	playmark_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_spriteram(*this, "spriteram"),
		m_rowscroll(*this, "rowscroll"),
		m_sprtranspen(0),
		m_oki(*this, "oki"),
		m_okibank(*this, "okibank"),
		m_eeprom(*this, "eeprom"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ticket(*this, "ticket"),
		m_token(*this, "token")
	{ }

	void wbeachvl(machine_config &config);
	void hrdtimes(machine_config &config);
	void luckboomh(machine_config &config);
	void bigtwin(machine_config &config);
	void hotmind(machine_config &config);
	void bigtwinb(machine_config &config);
	void excelsr(machine_config &config);

	void init_pic_decode();

protected:
	/* memory pointers */
	optional_shared_ptr<uint16_t> m_bgvideoram;
	required_shared_ptr<uint16_t> m_videoram1;
	optional_shared_ptr<uint16_t> m_videoram2;
	optional_shared_ptr<uint16_t> m_videoram3;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_rowscroll;

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_bg_tilemap;
	int         m_bgscrollx;
	int         m_bgscrolly;
	int         m_bg_enable;
	int         m_bg_full_size;
	int         m_fgscrollx;
	int         m_fg_rowscroll_enable;

	int         m_xoffset;
	int         m_yoffset;
	int         m_pri_masks[3];
	uint16_t      m_scroll[7];
	int         m_sprtranspen;

	/* misc */
	uint16_t      m_snd_command;
	uint16_t      m_snd_flag;
	uint8_t       m_oki_control;
	uint8_t       m_oki_command;
	uint8_t       m_dispenser_latch;
	int         m_oki_numbanks;
	void configure_oki_banks();

	/* devices */
	required_device<okim6295_device> m_oki;
	optional_memory_bank m_okibank;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	DECLARE_WRITE16_MEMBER(coinctrl_w);
	DECLARE_WRITE16_MEMBER(wbeachvl_coin_eeprom_w);
	DECLARE_WRITE16_MEMBER(hotmind_coin_eeprom_w);
	DECLARE_WRITE16_MEMBER(luckboomh_dispenser_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_coin_w);
	DECLARE_WRITE16_MEMBER(playmark_snd_command_w);
	DECLARE_READ8_MEMBER(playmark_snd_command_r);
	DECLARE_READ8_MEMBER(playmark_snd_flag_r);
	DECLARE_WRITE8_MEMBER(playmark_oki_w);
	DECLARE_WRITE8_MEMBER(playmark_snd_control_w);
	DECLARE_WRITE8_MEMBER(hrdtimes_snd_control_w);
	DECLARE_WRITE16_MEMBER(wbeachvl_txvideoram_w);
	DECLARE_WRITE16_MEMBER(wbeachvl_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(wbeachvl_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_txvideoram_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(bigtwin_scroll_w);
	DECLARE_WRITE16_MEMBER(wbeachvl_scroll_w);
	DECLARE_WRITE16_MEMBER(excelsr_scroll_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_scroll_w);
	DECLARE_WRITE8_MEMBER(playmark_oki_banking_w);
	TILE_GET_INFO_MEMBER(bigtwin_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(bigtwin_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(wbeachvl_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(wbeachvl_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(wbeachvl_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(hrdtimes_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(bigtwinb_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(hrdtimes_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(hrdtimes_get_bg_tile_info);
	DECLARE_MACHINE_START(playmark);
	DECLARE_MACHINE_RESET(playmark);
	DECLARE_VIDEO_START(bigtwin);
	DECLARE_VIDEO_START(bigtwinb);
	DECLARE_VIDEO_START(wbeachvl);
	DECLARE_VIDEO_START(excelsr);
	DECLARE_VIDEO_START(hotmind);
	DECLARE_VIDEO_START(hrdtimes);
	DECLARE_VIDEO_START(luckboomh);
	TILEMAP_MAPPER_MEMBER(playmark_tilemap_scan_pages);
	uint32_t screen_update_bigtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bigtwinb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_wbeachvl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_excelsr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hrdtimes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int codeshift );
	void bigtwinb_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int codeshift );
	void draw_bitmap( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	uint8_t playmark_asciitohex(uint8_t data);
	void playmark_decode_pic_hex_dump(void);
	required_device<cpu_device> m_maincpu;
	optional_device<pic16c57_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<ticket_dispenser_device> m_ticket;
	optional_device<ticket_dispenser_device> m_token;

	void bigtwin_main_map(address_map &map);
	void bigtwinb_main_map(address_map &map);
	void excelsr_main_map(address_map &map);
	void hotmind_main_map(address_map &map);
	void hrdtimes_main_map(address_map &map);
	void luckboomh_main_map(address_map &map);
	void oki_map(address_map &map);
	void wbeachvl_main_map(address_map &map);
};

#endif // MAME_INCLUDES_PLAYMARK_H
