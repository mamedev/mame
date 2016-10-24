// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "audio/seibu.h"
#include "video/bufsprite.h"

class dynduke_state : public driver_device
{
public:
	dynduke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") ,
		m_scroll_ram(*this, "scroll_ram"),
		m_videoram(*this, "videoram"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data") { }

	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<uint16_t> m_scroll_ram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_back_data;
	required_shared_ptr<uint16_t> m_fore_data;

	tilemap_t *m_bg_layer;
	tilemap_t *m_fg_layer;
	tilemap_t *m_tx_layer;
	int m_back_bankbase;
	int m_fore_bankbase;
	int m_back_enable;
	int m_fore_enable;
	int m_sprite_enable;
	int m_txt_enable;
	int m_old_back;
	int m_old_fore;

	void background_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void foreground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void text_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );

	void interrupt(device_t &device);
};
