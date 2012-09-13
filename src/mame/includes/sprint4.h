class sprint4_state : public driver_device
{
public:
	sprint4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_videoram;
	int m_da_latch;
	int m_steer_FF1[4];
	int m_steer_FF2[4];
	int m_gear[4];
	UINT8 m_last_wheel[4];
	int m_collision[4];
	tilemap_t* m_playfield;
	bitmap_ind16 m_helper;
	DECLARE_READ8_MEMBER(sprint4_wram_r);
	DECLARE_READ8_MEMBER(sprint4_analog_r);
	DECLARE_READ8_MEMBER(sprint4_coin_r);
	DECLARE_READ8_MEMBER(sprint4_collision_r);
	DECLARE_READ8_MEMBER(sprint4_options_r);
	DECLARE_WRITE8_MEMBER(sprint4_wram_w);
	DECLARE_WRITE8_MEMBER(sprint4_collision_reset_w);
	DECLARE_WRITE8_MEMBER(sprint4_da_latch_w);
	DECLARE_WRITE8_MEMBER(sprint4_lamp_w);
	DECLARE_WRITE8_MEMBER(sprint4_lockout_w);
	DECLARE_WRITE8_MEMBER(sprint4_video_ram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(get_lever);
	DECLARE_CUSTOM_INPUT_MEMBER(get_wheel);
	DECLARE_CUSTOM_INPUT_MEMBER(get_collision);
	DECLARE_WRITE8_MEMBER(sprint4_screech_1_w);
	DECLARE_WRITE8_MEMBER(sprint4_screech_2_w);
	DECLARE_WRITE8_MEMBER(sprint4_screech_3_w);
	DECLARE_WRITE8_MEMBER(sprint4_screech_4_w);
	DECLARE_WRITE8_MEMBER(sprint4_bang_w);
	DECLARE_WRITE8_MEMBER(sprint4_attract_w);
	TILE_GET_INFO_MEMBER(sprint4_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/sprint4.c -----------*/



SCREEN_VBLANK( sprint4 );

SCREEN_UPDATE_IND16( sprint4 );

