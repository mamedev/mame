// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria

#include "machine/gen_latch.h"
#include "sound/upd7759.h"
#include "video/snk68_spr.h"

class snk68_state : public driver_device
{
public:
	snk68_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_sprites(*this, "sprites"),
		m_soundlatch(*this, "soundlatch"),
		m_pow_fg_videoram(*this, "pow_fg_videoram"),
		m_spriteram(*this, "spriteram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<snk68_spr_device> m_sprites;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_pow_fg_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	uint8_t m_invert_controls;
	bool m_sprite_flip_axis;
	tilemap_t *m_fg_tilemap;
	uint32_t m_fg_tile_offset;

	// common
	void sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void D7759_write_port_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void D7759_upd_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// pow and streetsm
	uint16_t pow_fg_videoram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pow_fg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pow_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t control_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	// searchar and ikari3
	void searchar_fg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void searchar_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t protcontrols_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t rotary_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t rotary_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t rotary_lsb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void get_pow_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_searchar_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;
	void video_start_searchar();
	void common_video_start();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tile_callback_pow(int &tile, int& fx, int& fy, int& region);
	void tile_callback_notpow(int &tile, int& fx, int& fy, int& region);

};
