// license:BSD-3-Clause
// copyright-holders:Paul Leaman

#include "video/bufsprite.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class lwings_state : public driver_device
{
public:
	lwings_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_fgvideoram(*this, "fgvideoram"),
		m_bg1videoram(*this, "bg1videoram"),
		m_soundlatch2(*this, "soundlatch_2"),
		m_nmi_mask(0),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "5205"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bg1videoram;
	optional_shared_ptr<uint8_t> m_soundlatch2;

	/* video-related */
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_bg1_tilemap;
	tilemap_t  *m_bg2_tilemap;
	uint8_t    m_bg2_image;
	int      m_bg2_avenger_hw;
	int      m_spr_avenger_hw;
	uint8_t    m_scroll_x[2];
	uint8_t    m_scroll_y[2];

	/* misc */
	uint8_t    m_param[4];
	int      m_palette_pen;
	uint8_t    m_soundstate;
	uint8_t    m_adpcm;
	uint8_t    m_nmi_mask;
	int      m_sprbank;

	void avengers_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t avengers_adpcm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lwings_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void avengers_protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void avengers_prot_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t avengers_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t avengers_soundlatch2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lwings_fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lwings_bg1videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lwings_bg1_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lwings_bg1_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trojan_bg2_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trojan_bg2_image_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void msm5205_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fball_oki_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	tilemap_memory_index get_bg2_memory_offset(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void lwings_get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void trojan_get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void init_avengersb();
	void video_start_trojan();
	void video_start_avengers();
	void video_start_avengersb();
	uint32_t screen_update_lwings(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_trojan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void lwings_interrupt(device_t &device);
	void avengers_interrupt(device_t &device);
	inline int is_sprite_on( uint8_t *buffered_spriteram, int offs );
	void lwings_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void trojan_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	int avengers_fetch_paldata(  );
	required_device<cpu_device> m_maincpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};
