// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "audio/m72.h"

class shisen_state : public driver_device
{
public:
	shisen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audio (*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_videoram;

	int m_gfxbank;
	tilemap_t *m_bg_tilemap;

	uint8_t dsw1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
