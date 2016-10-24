// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino

#include "machine/gen_latch.h"

class lkage_state : public driver_device
{
public:
	lkage_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vreg(*this, "vreg"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	required_shared_ptr<uint8_t> m_vreg;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	uint8_t m_bg_tile_bank;
	uint8_t m_fg_tile_bank;
	uint8_t m_tx_tile_bank;

	int m_sprite_dx;

	/* misc */
	int m_sound_nmi_enable;
	int m_pending_nmi;

	/* mcu */
	uint8_t m_from_main;
	uint8_t m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;
	uint8_t m_port_a_in;
	uint8_t m_port_a_out;
	uint8_t m_ddr_a;
	uint8_t m_port_b_in;
	uint8_t m_port_b_out;
	uint8_t m_ddr_b;
	uint8_t m_port_c_in;
	uint8_t m_port_c_out;
	uint8_t m_ddr_c;

	/* lkageb fake mcu */
	uint8_t m_mcu_val;
	int m_mcu_ready;    /* cpu data/mcu ready status */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void lkage_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lkage_sh_nmi_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lkage_sh_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port_fetch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t fake_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fake_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fake_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lkage_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lkage_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lkage_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lkage_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lkage_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lkage_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lkage_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lkage_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lkage_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lkage_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lkage_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lkage_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lkage_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_bygone();
	void init_lkage();
	void init_lkageb();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_lkage(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmi_callback(void *ptr, int32_t param);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
};
