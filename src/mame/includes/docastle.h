

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
	DECLARE_READ8_MEMBER(docastle_shared0_r);
	DECLARE_READ8_MEMBER(docastle_shared1_r);
	DECLARE_WRITE8_MEMBER(docastle_shared0_w);
	DECLARE_WRITE8_MEMBER(docastle_shared1_w);
	DECLARE_WRITE8_MEMBER(docastle_nmitrigger_w);
	DECLARE_WRITE8_MEMBER(docastle_videoram_w);
	DECLARE_WRITE8_MEMBER(docastle_colorram_w);
	DECLARE_READ8_MEMBER(docastle_flipscreen_off_r);
	DECLARE_READ8_MEMBER(docastle_flipscreen_on_r);
	DECLARE_WRITE8_MEMBER(docastle_flipscreen_off_w);
	DECLARE_WRITE8_MEMBER(docastle_flipscreen_on_w);
};


/*----------- defined in machine/docastle.c -----------*/


/*----------- defined in video/docastle.c -----------*/


PALETTE_INIT( docastle );
VIDEO_START( docastle );
VIDEO_START( dorunrun );
SCREEN_UPDATE_IND16( docastle );

