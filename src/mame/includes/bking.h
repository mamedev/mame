// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Zsolt Vasvari

#include "machine/buggychl.h"
#include "machine/gen_latch.h"

class bking_state : public driver_device
{
public:
	bking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_bmcu(*this, "bmcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_playfield_ram;

	/* video-related */
	bitmap_ind16    m_colmap_bg;
	bitmap_ind16    m_colmap_ball;
	tilemap_t     *m_bg_tilemap;
	int         m_pc3259_output[4];
	int         m_pc3259_mask;
	uint8_t       m_xld1;
	uint8_t       m_xld2;
	uint8_t       m_xld3;
	uint8_t       m_yld1;
	uint8_t       m_yld2;
	uint8_t       m_yld3;
	int         m_ball1_pic;
	int         m_ball2_pic;
	int         m_crow_pic;
	int         m_crow_flip;
	int         m_palette_bank;
	int         m_controller;
	int         m_hit;

	/* sound-related */
	int         m_sound_nmi_enable;
	int         m_pending_nmi;

	/* misc */
	int         m_addr_h;
	int         m_addr_l;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	optional_device<buggychl_mcu_device> m_bmcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

#if 0
	/* 68705 */
	uint8_t m_port_a_in;
	uint8_t m_port_a_out;
	uint8_t m_ddr_a;
	uint8_t m_port_b_in;
	uint8_t m_port_b_out;
	uint8_t m_ddr_b;
#endif
	uint8_t bking_sndnmi_disable_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bking_sndnmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking3_addr_l_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking3_addr_h_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bking3_extrarom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bking3_ext_check_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bking_xld1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_yld1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_xld2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_yld2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_xld3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_yld3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_cont1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_cont2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_cont3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_msk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_hitclr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bking_playfield_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bking_input_port_5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bking_input_port_6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bking_pos_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void unk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_bking(palette_device &palette);
	void machine_start_bking3();
	void machine_reset_bking3();
	void machine_reset_common();
	uint32_t screen_update_bking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_bking(screen_device &screen, bool state);
};
