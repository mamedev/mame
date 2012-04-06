class sprint4_state : public driver_device
{
public:
	sprint4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
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
};


/*----------- defined in video/sprint4.c -----------*/

PALETTE_INIT( sprint4 );

SCREEN_VBLANK( sprint4 );
VIDEO_START( sprint4 );
SCREEN_UPDATE_IND16( sprint4 );

