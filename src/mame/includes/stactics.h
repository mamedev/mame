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

	required_shared_ptr<UINT8> m_palette_val;
	required_shared_ptr<UINT8> m_motor_on;
	required_shared_ptr<UINT8> m_lamps;
	required_shared_ptr<UINT8> m_display_buffer;
	required_shared_ptr<UINT8> m_videoram_b;
	required_shared_ptr<UINT8> m_videoram_d;
	required_shared_ptr<UINT8> m_videoram_e;
	required_shared_ptr<UINT8> m_videoram_f;

	/* machine state */
	int    m_vert_pos;
	int    m_horiz_pos;

	/* video state */
	UINT8  m_y_scroll_d;
	UINT8  m_y_scroll_e;
	UINT8  m_y_scroll_f;
	UINT8  m_frame_count;
	UINT8  m_shot_standby;
	UINT8  m_shot_arrive;
	UINT16 m_beam_state;
	UINT16 m_old_beam_state;
	UINT16 m_beam_states_per_frame;

	DECLARE_READ8_MEMBER(vert_pos_r);
	DECLARE_READ8_MEMBER(horiz_pos_r);
	DECLARE_WRITE8_MEMBER(coinlockout_w);
	DECLARE_WRITE8_MEMBER(scroll_ram_w);
	DECLARE_WRITE8_MEMBER(speed_latch_w);
	DECLARE_WRITE8_MEMBER(shot_trigger_w);
	DECLARE_WRITE8_MEMBER(shot_flag_clear_w);

	DECLARE_CUSTOM_INPUT_MEMBER(get_frame_count_d3);
	DECLARE_CUSTOM_INPUT_MEMBER(get_shot_standby);
	DECLARE_CUSTOM_INPUT_MEMBER(get_not_shot_arrive);
	DECLARE_CUSTOM_INPUT_MEMBER(get_motor_not_ready);
	DECLARE_CUSTOM_INPUT_MEMBER(get_rng);
	INTERRUPT_GEN_MEMBER(interrupt);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(stactics);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_beam();
	inline int get_pixel_on_plane(UINT8 *videoram, UINT8 y, UINT8 x, UINT8 y_scroll);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_indicator_leds(int data, const char *output_name, int base_index);
	void update_artwork();
	void move_motor();
};
/*----------- defined in video/stactics.c -----------*/
MACHINE_CONFIG_EXTERN( stactics_video );
