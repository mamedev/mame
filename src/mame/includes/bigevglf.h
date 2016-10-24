// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Tomasz Slanina
#include "sound/msm5232.h"

class bigevglf_state : public driver_device
{
public:
	bigevglf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_paletteram(*this, "paletteram"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_spriteram1;
	required_shared_ptr<uint8_t> m_spriteram2;

	/* video-related */
	bitmap_ind16 m_tmp_bitmap[4];
	std::unique_ptr<uint8_t[]>    m_vidram;
	uint32_t   m_vidram_bank;
	uint32_t   m_plane_selected;
	uint32_t   m_plane_visible;

	/* sound-related */
	int      m_sound_nmi_enable;
	int      m_pending_nmi;
	uint8_t    m_for_sound;
	uint8_t    m_from_sound;
	uint8_t    m_sound_state;

	/* MCU related */
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
	int      m_mcu_coin_bit5;

	/* misc */
	uint32_t   m_beg_bank;
	uint8_t    m_beg13_ls74[2];
	uint8_t    m_port_select;     /* for muxed controls */

	/* devices */
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	void beg_banking_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void beg_fromsound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t beg_fromsound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t beg_soundstate_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t soundstate_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nmi_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void beg13_a_clr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void beg13_b_clr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void beg13_a_set_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void beg13_b_set_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t beg_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t beg_trackball_x_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t beg_trackball_y_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void beg_port08_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sub_cpu_mcu_coin_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bigevglf_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bigevglf_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bigevglf_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bigevglf_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bigevglf_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bigevglf_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bigevglf_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bigevglf_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bigevglf_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bigevglf_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bigevglf_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bigevglf_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bigevglf_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bigevglf_gfxcontrol_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bigevglf_vidram_addr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bigevglf_vidram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bigevglf_vidram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_bigevglf();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_bigevglf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void from_sound_latch_callback(void *ptr, int32_t param);
	void nmi_callback(void *ptr, int32_t param);
	void deferred_ls74_w(void *ptr, int32_t param);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
