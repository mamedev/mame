// license:BSD-3-Clause
// copyright-holders:David Haywood

/*************************************************************************

    Success Joe / Ashita no Joe

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class ashnojoe_state : public driver_device
{
public:
	ashnojoe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tileram_3(*this, "tileram_3"),
		m_tileram_4(*this, "tileram_4"),
		m_tileram_5(*this, "tileram_5"),
		m_tileram_2(*this, "tileram_2"),
		m_tileram_6(*this, "tileram_6"),
		m_tileram_7(*this, "tileram_7"),
		m_tileram(*this, "tileram"),
		m_tilemap_reg(*this, "tilemap_reg"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	uint16_t *    m_tileram_1;
	required_shared_ptr<uint16_t> m_tileram_3;
	required_shared_ptr<uint16_t> m_tileram_4;
	required_shared_ptr<uint16_t> m_tileram_5;
	required_shared_ptr<uint16_t> m_tileram_2;
	required_shared_ptr<uint16_t> m_tileram_6;
	required_shared_ptr<uint16_t> m_tileram_7;
	required_shared_ptr<uint16_t> m_tileram;
	required_shared_ptr<uint16_t> m_tilemap_reg;

	/* video-related */
	tilemap_t     *m_joetilemap;
	tilemap_t     *m_joetilemap2;
	tilemap_t     *m_joetilemap3;
	tilemap_t     *m_joetilemap4;
	tilemap_t     *m_joetilemap5;
	tilemap_t     *m_joetilemap6;
	tilemap_t     *m_joetilemap7;

	/* sound-related */
	uint8_t       m_adpcm_byte;
	int         m_soundlatch_status;
	int         m_msm5205_vclk_toggle;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	uint16_t fake_4a00a_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ashnojoe_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_latch_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ashnojoe_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ashnojoe_tileram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ashnojoe_tileram3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ashnojoe_tileram4_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ashnojoe_tileram5_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ashnojoe_tileram6_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ashnojoe_tileram7_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void joe_tilemaps_xscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void joe_tilemaps_yscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ym2203_write_a(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ym2203_write_b(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_ashnojoe();
	void get_joe_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_joe_tile_info_2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_joe_tile_info_3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_joe_tile_info_4(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_joe_tile_info_5(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_joe_tile_info_6(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_joe_tile_info_7(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_ashnojoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ashnojoe_vclk_cb(int state);
};
