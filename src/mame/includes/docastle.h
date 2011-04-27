

class docastle_state : public driver_device
{
public:
	docastle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_spriteram;
	size_t   m_spriteram_size;

	/* video-related */
	tilemap_t  *m_do_tilemap;

	/* misc */
	int      m_adpcm_pos;
	int      m_adpcm_idle;
	int      m_adpcm_data;
	int      m_adpcm_status;
	UINT8    m_buffer0[9];
	UINT8    m_buffer1[9];

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_slave;
};


/*----------- defined in machine/docastle.c -----------*/

READ8_HANDLER( docastle_shared0_r );
READ8_HANDLER( docastle_shared1_r );
WRITE8_HANDLER( docastle_shared0_w );
WRITE8_HANDLER( docastle_shared1_w );
WRITE8_HANDLER( docastle_nmitrigger_w );

/*----------- defined in video/docastle.c -----------*/

WRITE8_HANDLER( docastle_videoram_w );
WRITE8_HANDLER( docastle_colorram_w );
READ8_HANDLER( docastle_flipscreen_off_r );
READ8_HANDLER( docastle_flipscreen_on_r );
WRITE8_HANDLER( docastle_flipscreen_off_w );
WRITE8_HANDLER( docastle_flipscreen_on_w );

PALETTE_INIT( docastle );
VIDEO_START( docastle );
VIDEO_START( dorunrun );
SCREEN_UPDATE( docastle );

