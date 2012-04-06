class ampoker2_state : public driver_device
{
public:
	ampoker2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(ampoker2_port30_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port31_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port32_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port33_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port34_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port35_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port36_w);
	DECLARE_WRITE8_MEMBER(ampoker2_watchdog_reset_w);
	DECLARE_WRITE8_MEMBER(ampoker2_videoram_w);
};


/*----------- defined in video/ampoker2.c -----------*/

PALETTE_INIT( ampoker2 );
VIDEO_START( ampoker2 );
VIDEO_START( sigma2k );
SCREEN_UPDATE_IND16( ampoker2 );
