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
	bitmap_t* m_helper;
};


/*----------- defined in video/sprint4.c -----------*/

PALETTE_INIT( sprint4 );

SCREEN_EOF( sprint4 );
VIDEO_START( sprint4 );
SCREEN_UPDATE( sprint4 );

WRITE8_HANDLER( sprint4_video_ram_w );
