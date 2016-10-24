// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Wolf Pack (prototype) driver

***************************************************************************/

#include "sound/s14001a.h"

class wolfpack_state : public driver_device
{
public:
	enum
	{
		TIMER_PERIODIC
	};

	wolfpack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_alpha_num_ram(*this, "alpha_num_ram"),
		m_maincpu(*this, "maincpu"),
		m_s14001a(*this, "speech"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	// devices, pointers
	required_shared_ptr<uint8_t> m_alpha_num_ram;
	required_device<cpu_device> m_maincpu;
	required_device<s14001a_device> m_s14001a;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	int m_collision;
	unsigned m_current_index;
	uint8_t m_video_invert;
	uint8_t m_ship_reflect;
	uint8_t m_pt_pos_select;
	uint8_t m_pt_horz;
	uint8_t m_pt_pic;
	uint8_t m_ship_h;
	uint8_t m_torpedo_pic;
	uint8_t m_ship_size;
	uint8_t m_ship_h_precess;
	uint8_t m_ship_pic;
	uint8_t m_torpedo_h;
	uint8_t m_torpedo_v;
	std::unique_ptr<uint8_t[]> m_LFSR;
	bitmap_ind16 m_helper;
	uint8_t wolfpack_misc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wolfpack_high_explo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_sonar_ping_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_sirlat_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_pt_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_launch_torpedo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_low_explo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_screw_cont_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_lamp_flash_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_warning_light_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_audamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_attract_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_credit_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_coldetres_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_ship_size_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_video_invert_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_ship_reflect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_pt_pos_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_pt_horz_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_pt_pic_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_ship_h_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_torpedo_pic_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_ship_h_precess_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_ship_pic_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_torpedo_h_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_torpedo_v_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value wolfpack_dial_r(ioport_field &field, void *param);
	void wolfpack_word_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wolfpack_start_speech_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_wolfpack(palette_device &palette);
	uint32_t screen_update_wolfpack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_wolfpack(screen_device &screen, bool state);
	void periodic_callback(void *ptr, int32_t param);
	void draw_ship(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_torpedo(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_pt(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_water(palette_device &palette, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
