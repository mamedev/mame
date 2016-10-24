// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina

#include "machine/gen_latch.h"
#include "sound/msm5232.h"

class nycaptor_state : public driver_device
{
public:
	nycaptor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrlram(*this, "scrlram"),
		m_sharedram(*this, "sharedram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrlram;
	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	std::vector<uint8_t> m_paletteram;
	std::vector<uint8_t> m_paletteram_ext;
	uint8_t m_gfxctrl;
	uint8_t m_char_bank;
	uint8_t m_palette_bank;

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

	/* misc */
	int m_generic_control_reg;
	int m_sound_nmi_enable;
	int m_pending_nmi;
	uint8_t m_snd_data;
	int m_vol_ctrl[16];
	int  m_gametype;
	int m_mask;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_mcu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	void sub_cpu_halt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t from_snd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void to_main_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_by_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_bx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_cpu_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_generic_control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nycaptor_generic_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cyclshtg_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t cyclshtg_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cyclshtg_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cyclshtg_mcu_status_r1(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cyclshtg_generic_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t unk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nycaptor_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nycaptor_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nycaptor_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nycaptor_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nycaptor_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nycaptor_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nycaptor_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_mcu_status_r1(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_mcu_status_r2(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nycaptor_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nycaptor_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_palette_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nycaptor_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nycaptor_gfxctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nycaptor_scrlram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void unk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_cyclshtg();
	void init_colt();
	void init_bronx();
	void init_nycaptor();
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void machine_reset_ta7630();
	uint32_t screen_update_nycaptor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmi_callback(void *ptr, int32_t param);
	int nycaptor_spot(  );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
};
