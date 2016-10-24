// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "video/tecmo_spr.h"

class tbowl_state : public driver_device
{
public:
	tbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch"),
		m_txvideoram(*this, "txvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_spriteram(*this, "spriteram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_bg2videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	uint16_t m_xscroll;
	uint16_t m_yscroll;
	uint16_t m_bg2xscroll;
	uint16_t m_bg2yscroll;
	int m_adpcm_pos[2];
	int m_adpcm_end[2];
	int m_adpcm_data[2];

	void coincounter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void boardb_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void boardc_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trigger_nmi(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_end_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_vol_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void txvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg2videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgxscroll_lo(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgxscroll_hi(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgyscroll_lo(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgyscroll_hi(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg2xscroll_lo(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg2xscroll_hi(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg2yscroll_lo(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg2yscroll_hi(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void adpcm_int(msm5205_device *device, int chip);
	void adpcm_int_1(int state);
	void adpcm_int_2(int state);
};
