// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)

****************************************************************************/


class stactics_state : public driver_device
{
public:
	stactics_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette_val(*this, "paletteram"),
		m_motor_on(*this, "motor_on"),
		m_lamps(*this, "lamps"),
		m_display_buffer(*this, "display_buffer"),
		m_videoram_b(*this, "videoram_b"),
		m_videoram_d(*this, "videoram_d"),
		m_videoram_e(*this, "videoram_e"),
		m_videoram_f(*this, "videoram_f") { }

	required_device<cpu_device> m_maincpu;

	required_shared_ptr<uint8_t> m_palette_val;
	required_shared_ptr<uint8_t> m_motor_on;
	required_shared_ptr<uint8_t> m_lamps;
	required_shared_ptr<uint8_t> m_display_buffer;
	required_shared_ptr<uint8_t> m_videoram_b;
	required_shared_ptr<uint8_t> m_videoram_d;
	required_shared_ptr<uint8_t> m_videoram_e;
	required_shared_ptr<uint8_t> m_videoram_f;

	/* machine state */
	int    m_vert_pos;
	int    m_horiz_pos;

	/* video state */
	uint8_t  m_y_scroll_d;
	uint8_t  m_y_scroll_e;
	uint8_t  m_y_scroll_f;
	uint8_t  m_frame_count;
	uint8_t  m_shot_standby;
	uint8_t  m_shot_arrive;
	uint16_t m_beam_state;
	uint16_t m_old_beam_state;
	uint16_t m_beam_states_per_frame;

	uint8_t vert_pos_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t horiz_pos_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void coinlockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void speed_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shot_trigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shot_flag_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value get_frame_count_d3(ioport_field &field, void *param);
	ioport_value get_shot_standby(ioport_field &field, void *param);
	ioport_value get_not_shot_arrive(ioport_field &field, void *param);
	ioport_value get_motor_not_ready(ioport_field &field, void *param);
	ioport_value get_rng(ioport_field &field, void *param);
	void interrupt(device_t &device);

	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_stactics(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_beam();
	inline int get_pixel_on_plane(uint8_t *videoram, uint8_t y, uint8_t x, uint8_t y_scroll);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_indicator_leds(int data, const char *output_name, int base_index);
	void update_artwork();
	void move_motor();
};
/*----------- defined in video/stactics.c -----------*/
MACHINE_CONFIG_EXTERN( stactics_video );
