// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

class lsasquad_state : public driver_device
{
public:
	lsasquad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* misc */
	int m_sound_pending;
	int m_sound_nmi_enable;
	int m_pending_nmi;
	int m_sound_cmd;
	int m_sound_result;

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

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void lsasquad_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lsasquad_sh_nmi_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lsasquad_sh_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lsasquad_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lsasquad_sh_sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lsasquad_sh_result_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lsasquad_sound_result_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lsasquad_sound_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t daikaiju_sh_sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t daikaiju_sound_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lsasquad_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lsasquad_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lsasquad_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lsasquad_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lsasquad_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lsasquad_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lsasquad_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lsasquad_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lsasquad_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t daikaiju_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void unk(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void machine_start_lsasquad();
	void machine_reset_lsasquad();
	uint32_t screen_update_lsasquad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_daikaiju(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmi_callback(void *ptr, int32_t param);
	void draw_layer( bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *scrollram );
	int draw_layer_daikaiju( bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int  * previd, int type );
	void drawbg( bitmap_ind16 &bitmap, const rectangle &cliprect, int type );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
