// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

#include "machine/gen_latch.h"
#include "video/bufsprite.h"

class bbusters_state : public driver_device
{
public:
	bbusters_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_soundlatch(*this, "soundlatch"),
		m_eprom_data(*this, "eeprom"),
		m_ram(*this, "ram"),
		m_videoram(*this, "videoram"),
		m_pf1_data(*this, "pf1_data"),
		m_pf2_data(*this, "pf2_data"),
		m_pf1_scroll_data(*this, "pf1_scroll_data"),
		m_pf2_scroll_data(*this, "pf2_scroll_data") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_device<buffered_spriteram16_device> m_spriteram2;
	required_device<generic_latch_8_device> m_soundlatch;

	optional_shared_ptr<uint16_t> m_eprom_data;
	required_shared_ptr<uint16_t> m_ram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_pf1_data;
	required_shared_ptr<uint16_t> m_pf2_data;
	required_shared_ptr<uint16_t> m_pf1_scroll_data;
	required_shared_ptr<uint16_t> m_pf2_scroll_data;

	int m_sound_status;
	int m_gun_select;
	tilemap_t *m_fix_tilemap;
	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_pf2_tilemap;
	const uint8_t *m_scale_table_ptr;
	uint8_t m_scale_line_count;

	uint16_t sound_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_cpu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t eprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t control_3_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gun_select_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void two_gun_output_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void three_gun_output_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t kludge_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mechatt_gun_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void video_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pf1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pf2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	void video_start_bbuster();
	void video_start_mechatt();

	uint32_t screen_update_bbuster(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mechatt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_bbuster(screen_device &screen, bool state);
	inline const uint8_t *get_source_ptr(gfx_element *gfx, uint32_t sprite, int dx, int dy, int block);
	void draw_block(bitmap_ind16 &dest,int x,int y,int size,int flipx,int flipy,uint32_t sprite,int color,int bank,int block);
	void draw_sprites(bitmap_ind16 &bitmap, const uint16_t *source, int bank, int colval, int colmask);
};
