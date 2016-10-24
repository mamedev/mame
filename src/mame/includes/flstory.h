// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "machine/gen_latch.h"
#include "sound/msm5232.h"

class flstory_state : public driver_device
{
public:
	flstory_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scrlram(*this, "scrlram"),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrlram;
	optional_shared_ptr<uint8_t> m_workram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	std::vector<uint8_t> m_paletteram;
	std::vector<uint8_t> m_paletteram_ext;
	uint8_t    m_gfxctrl;
	uint8_t    m_char_bank;
	uint8_t    m_palette_bank;

	/* sound-related */
	uint8_t    m_snd_data;
	uint8_t    m_snd_flag;
	int      m_sound_nmi_enable;
	int      m_pending_nmi;
	int      m_vol_ctrl[16];
	uint8_t    m_snd_ctrl0;
	uint8_t    m_snd_ctrl1;
	uint8_t    m_snd_ctrl2;
	uint8_t    m_snd_ctrl3;

	/* protection */
	uint8_t    m_from_main;
	uint8_t    m_from_mcu;
	int      m_mcu_sent;
	int      m_main_sent;
	uint8_t    m_port_a_in;
	uint8_t    m_port_a_out;
	uint8_t    m_ddr_a;
	uint8_t    m_port_b_in;
	uint8_t    m_port_b_out;
	uint8_t    m_ddr_b;
	uint8_t    m_port_c_in;
	uint8_t    m_port_c_out;
	uint8_t    m_ddr_c;
	int      m_mcu_select;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* mcu */
	uint8_t m_mcu_cmd;
	uint8_t m_mcu_counter;
	uint8_t m_mcu_b4_cmd;
	uint8_t m_mcu_param;
	uint8_t m_mcu_b2_res;
	uint8_t m_mcu_b1_res;
	uint8_t m_mcu_bb_res;
	uint8_t m_mcu_b5_res;
	uint8_t m_mcu_b6_res;
	uint8_t from_snd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t snd_flag_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void to_main_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rumba_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rumba_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t flstory_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flstory_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flstory_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t flstory_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flstory_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flstory_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t flstory_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flstory_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flstory_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flstory_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t flstory_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t flstory_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void onna34ro_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t onna34ro_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t onna34ro_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void victnine_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t victnine_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t victnine_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flstory_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flstory_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t flstory_palette_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flstory_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t victnine_gfxctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void victnine_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flstory_scrlram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value victnine_mcu_status_bit01_r(ioport_field &field, void *param);
	void sound_control_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_control_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_control_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_control_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void victnine_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_rumba_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	void machine_reset_flstory();
	void video_start_flstory();
	void video_start_victnine();
	void machine_reset_rumba();
	void video_start_rumba();
	void machine_reset_ta7630();
	uint32_t screen_update_flstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_victnine(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rumba(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmi_callback(void *ptr, int32_t param);
	void flstory_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	void victnine_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
