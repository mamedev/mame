// license:BSD-3-Clause
// copyright-holders:David Graves, R. Belmont

#include "sound/okim6295.h"
#include "sound/msm5205.h"
#include "video/excellent_spr.h"

class gcpinbal_state : public driver_device
{
public:
	enum
	{
		TIMER_GCPINBAL_INTERRUPT1,
		TIMER_GCPINBAL_INTERRUPT3
	};

	gcpinbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_msm(*this, "msm"),
		m_tilemapram(*this, "tilemapram"),
		m_ioc_ram(*this, "ioc_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<msm6585_device> m_msm;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_tilemapram;
	required_shared_ptr<uint16_t> m_ioc_ram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t     *m_tilemap[3];
	uint16_t      m_scrollx[3];
	uint16_t      m_scrolly[3];
	uint16_t      m_bg0_gfxset;
	uint16_t      m_bg1_gfxset;
#ifdef MAME_DEBUG
	uint8_t       m_dislayer[4];
#endif

	/* sound-related */
	uint32_t      m_msm_start;
	uint32_t      m_msm_end;
	uint32_t      m_msm_bank;
	uint32_t      m_adpcm_start;
	uint32_t      m_adpcm_end;
	uint32_t      m_adpcm_idle;
	uint8_t       m_adpcm_trigger;
	uint8_t       m_adpcm_data;

	uint16_t ioc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ioc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gcpinbal_tilemaps_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gcpinbal_tilemaps_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void get_bg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_gcpinbal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gcpinbal_interrupt(device_t &device);
	void gcpinbal_core_vh_start(  );
	void gcp_adpcm_int(int state);
	required_device<excellent_spr_device> m_sprgen;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
