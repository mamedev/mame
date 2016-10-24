// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
class sspeedr_state : public driver_device
{
public:
	sspeedr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	uint8_t m_led_TIME[2];
	uint8_t m_led_SCORE[24];
	int m_toggle;
	unsigned m_driver_horz;
	unsigned m_driver_vert;
	unsigned m_driver_pic;
	unsigned m_drones_horz;
	unsigned m_drones_vert[3];
	unsigned m_drones_mask;
	unsigned m_track_horz;
	unsigned m_track_vert[2];
	unsigned m_track_ice;
	void sspeedr_int_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_lamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_time_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_score_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_driver_horz_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_driver_horz_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_driver_vert_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_driver_pic_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_drones_horz_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_drones_horz_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_drones_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_drones_vert_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_track_horz_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_track_horz_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_track_vert_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspeedr_track_ice_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void video_start() override;
	void palette_init_sspeedr(palette_device &palette);
	uint32_t screen_update_sspeedr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_sspeedr(screen_device &screen, bool state);
	void draw_track(bitmap_ind16 &bitmap);
	void draw_drones(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_driver(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
