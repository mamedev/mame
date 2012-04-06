/*************************************************************************

    Atari Ultra Tank hardware

*************************************************************************/


class ultratnk_state : public driver_device
{
public:
	ultratnk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
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
};


/*----------- defined in video/ultratnk.c -----------*/

PALETTE_INIT( ultratnk );
VIDEO_START( ultratnk );
SCREEN_UPDATE_IND16( ultratnk );
SCREEN_VBLANK( ultratnk );

