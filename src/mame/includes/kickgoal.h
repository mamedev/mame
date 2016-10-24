// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Kick Goal - Action Hollywood

*************************************************************************/

#include "sound/okim6295.h"
#include "machine/eepromser.h"

class kickgoal_state : public driver_device
{
public:
	kickgoal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_bg2ram(*this, "bg2ram"),
		m_scrram(*this, "scrram"),
		m_spriteram(*this, "spriteram"),
		m_adpcm(*this, "oki"),
		m_eeprom(*this, "eeprom") ,
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_fgram;
	required_shared_ptr<uint16_t> m_bgram;
	required_shared_ptr<uint16_t> m_bg2ram;
	required_shared_ptr<uint16_t> m_scrram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	tilemap_t     *m_fgtm;
	tilemap_t     *m_bgtm;
	tilemap_t     *m_bg2tm;

	/* misc */
	int         m_melody_loop;
	int         m_snd_new;
	int         m_snd_sam[4];
	int         m_m6295_comm;
	int         m_m6295_bank;
	uint16_t      m_m6295_key_delay;

	int m_fg_base;

	int m_bg_base;
	int m_bg_mask;

	int m_bg2_base;
	int m_bg2_mask;
	int m_bg2_region;

	int m_sprbase;

	/* devices */
	required_device<okim6295_device> m_adpcm;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	uint16_t kickgoal_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void kickgoal_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void kickgoal_fgram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void kickgoal_bgram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void kickgoal_bg2ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void actionhw_snd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_kickgoal();
	void get_kickgoal_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_kickgoal_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_kickgoal_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index tilemap_scan_kicksfg(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index tilemap_scan_kicksbg(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index tilemap_scan_kicksbg2(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index tilemap_scan_actionhwbg2(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_kickgoal();
	void video_start_actionhw();
	uint32_t screen_update_kickgoal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void kickgoal_interrupt(device_t &device);
	void kickgoal_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
