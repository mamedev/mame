// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class wc90b_state : public driver_device
{
public:
	wc90b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_scroll1x(*this, "scroll1x"),
		m_scroll2x(*this, "scroll2x"),
		m_scroll1y(*this, "scroll1y"),
		m_scroll2y(*this, "scroll2y"),
		m_scroll_x_lo(*this, "scroll_x_lo"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_scroll1x;
	required_shared_ptr<uint8_t> m_scroll2x;
	required_shared_ptr<uint8_t> m_scroll1y;
	required_shared_ptr<uint8_t> m_scroll2y;
	required_shared_ptr<uint8_t> m_scroll_x_lo;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_msm5205next;
	int m_toggle;

	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void txvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_int(int state);
	uint8_t master_irq_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void slave_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
};
