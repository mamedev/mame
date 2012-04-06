class shootout_state : public driver_device
{
public:
	shootout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_background;
	tilemap_t *m_foreground;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	UINT8 *m_textram;
	int m_bFlicker;
	DECLARE_WRITE8_MEMBER(shootout_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sound_cpu_command_w);
	DECLARE_WRITE8_MEMBER(shootout_flipscreen_w);
	DECLARE_WRITE8_MEMBER(shootout_coin_counter_w);
	DECLARE_WRITE8_MEMBER(shootout_videoram_w);
	DECLARE_WRITE8_MEMBER(shootout_textram_w);
};


/*----------- defined in video/shootout.c -----------*/


PALETTE_INIT( shootout );
VIDEO_START( shootout );
SCREEN_UPDATE_IND16( shootout );
SCREEN_UPDATE_IND16( shootouj );
