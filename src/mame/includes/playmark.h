// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Pierpaolo Prazzoli, Quench
#include "sound/okim6295.h"
#include "machine/eepromser.h"
#include "cpu/pic16c5x/pic16c5x.h"

class playmark_state : public driver_device
{
public:
	playmark_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_spriteram(*this, "spriteram"),
		m_rowscroll(*this, "rowscroll"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

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

	/* misc */
	uint16_t      m_snd_command;
	uint16_t      m_snd_flag;
	uint8_t       m_oki_control;
	uint8_t       m_oki_command;
	int         m_old_oki_bank;
	uint8_t       m_dispenser_latch;

	/* devices */
	required_device<okim6295_device> m_oki;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	void coinctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wbeachvl_coin_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hotmind_coin_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void luckboomh_dispenser_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hrdtimes_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void playmark_snd_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t playmark_snd_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t playmark_snd_flag_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void playmark_oki_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void playmark_snd_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hrdtimes_snd_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int PIC16C5X_T0_clk_r();
	void wbeachvl_txvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wbeachvl_fgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wbeachvl_bgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hrdtimes_txvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hrdtimes_fgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hrdtimes_bgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bigtwin_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wbeachvl_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void excelsr_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hrdtimes_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void playmark_oki_banking_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_pic_decode();
	void bigtwin_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bigtwin_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void wbeachvl_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void wbeachvl_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void wbeachvl_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void hrdtimes_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bigtwinb_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void hrdtimes_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void hrdtimes_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_playmark();
	void machine_reset_playmark();
	void video_start_bigtwin();
	void video_start_bigtwinb();
	void video_start_wbeachvl();
	void video_start_excelsr();
	void video_start_hotmind();
	void video_start_hrdtimes();
	void video_start_luckboomh();
	tilemap_memory_index playmark_tilemap_scan_pages(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
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
};
