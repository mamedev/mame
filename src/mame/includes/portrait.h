// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Pierpaolo Prazzoli
#include "sound/tms5220.h"

class portrait_state : public driver_device
{
public:
	portrait_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_tms(*this, "tms"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tms5200_device> m_tms;

	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_scroll;
	tilemap_t *m_foreground;
	tilemap_t *m_background;

	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void positive_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void negative_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgvideo_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgvideo_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;
	void palette_init_portrait(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info( tile_data &tileinfo, int tile_index, const uint8_t *source );
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
