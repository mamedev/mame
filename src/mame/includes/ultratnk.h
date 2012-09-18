/*************************************************************************

    Atari Ultra Tank hardware

*************************************************************************/


class ultratnk_state : public driver_device
{
public:
	ultratnk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_videoram;
	int m_da_latch;
	int m_collision[4];
	tilemap_t* m_playfield;
	bitmap_ind16 m_helper;
	DECLARE_READ8_MEMBER(ultratnk_wram_r);
	DECLARE_READ8_MEMBER(ultratnk_analog_r);
	DECLARE_READ8_MEMBER(ultratnk_coin_r);
	DECLARE_READ8_MEMBER(ultratnk_collision_r);
	DECLARE_READ8_MEMBER(ultratnk_options_r);
	DECLARE_WRITE8_MEMBER(ultratnk_wram_w);
	DECLARE_WRITE8_MEMBER(ultratnk_collision_reset_w);
	DECLARE_WRITE8_MEMBER(ultratnk_da_latch_w);
	DECLARE_WRITE8_MEMBER(ultratnk_led_1_w);
	DECLARE_WRITE8_MEMBER(ultratnk_led_2_w);
	DECLARE_WRITE8_MEMBER(ultratnk_lockout_w);
	DECLARE_WRITE8_MEMBER(ultratnk_video_ram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(get_collision);
	DECLARE_CUSTOM_INPUT_MEMBER(get_joystick);
	DECLARE_WRITE8_MEMBER(ultratnk_fire_1_w);
	DECLARE_WRITE8_MEMBER(ultratnk_fire_2_w);
	DECLARE_WRITE8_MEMBER(ultratnk_attract_w);
	DECLARE_WRITE8_MEMBER(ultratnk_explosion_w);
	TILE_GET_INFO_MEMBER(ultratnk_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_ultratnk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_ultratnk(screen_device &screen, bool state);
};
