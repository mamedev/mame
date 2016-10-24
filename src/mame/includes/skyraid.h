// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
#include "sound/discrete.h"

class skyraid_state : public driver_device
{
public:
	skyraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_pos_ram(*this, "pos_ram"),
		m_alpha_num_ram(*this, "alpha_num_ram"),
		m_obj_ram(*this, "obj_ram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_analog_range;
	int m_analog_offset;

	int m_scroll;

	required_shared_ptr<uint8_t> m_pos_ram;
	required_shared_ptr<uint8_t> m_alpha_num_ram;
	required_shared_ptr<uint8_t> m_obj_ram;
	required_device<discrete_device> m_discrete;
	bitmap_ind16 m_helper;
	uint8_t skyraid_port_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void skyraid_range_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skyraid_offset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skyraid_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void video_start() override;
	void palette_init_skyraid(palette_device &palette);
	uint32_t screen_update_skyraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void skyraid_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void draw_text(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_terrain(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_missiles(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_trapezoid(bitmap_ind16& dst, bitmap_ind16& src);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

/*----------- defined in audio/skyraid.c -----------*/
DISCRETE_SOUND_EXTERN( skyraid );
