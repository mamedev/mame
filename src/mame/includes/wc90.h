// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi

#include "machine/gen_latch.h"
#include "video/tecmo_spr.h"

class wc90_state : public driver_device
{
public:
	wc90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_scroll0xlo(*this, "scroll0xlo"),
		m_scroll0xhi(*this, "scroll0xhi"),
		m_scroll1xlo(*this, "scroll1xlo"),
		m_scroll1xhi(*this, "scroll1xhi"),
		m_scroll2xlo(*this, "scroll2xlo"),
		m_scroll2xhi(*this, "scroll2xhi"),
		m_scroll0ylo(*this, "scroll0ylo"),
		m_scroll0yhi(*this, "scroll0yhi"),
		m_scroll1ylo(*this, "scroll1ylo"),
		m_scroll1yhi(*this, "scroll1yhi"),
		m_scroll2ylo(*this, "scroll2ylo"),
		m_scroll2yhi(*this, "scroll2yhi"),
		m_spriteram(*this, "spriteram")
	{ }


	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_scroll0xlo;
	required_shared_ptr<uint8_t> m_scroll0xhi;
	required_shared_ptr<uint8_t> m_scroll1xlo;
	required_shared_ptr<uint8_t> m_scroll1xhi;
	required_shared_ptr<uint8_t> m_scroll2xlo;
	required_shared_ptr<uint8_t> m_scroll2xhi;
	required_shared_ptr<uint8_t> m_scroll0ylo;
	required_shared_ptr<uint8_t> m_scroll0yhi;
	required_shared_ptr<uint8_t> m_scroll1ylo;
	required_shared_ptr<uint8_t> m_scroll1yhi;
	required_shared_ptr<uint8_t> m_scroll2ylo;
	required_shared_ptr<uint8_t> m_scroll2yhi;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void txvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void track_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void track_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;
	void video_start_wc90t();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
