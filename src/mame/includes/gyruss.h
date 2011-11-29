/*************************************************************************

    Gyruss

*************************************************************************/

class gyruss_state : public driver_device
{
public:
	gyruss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	UINT8 *    m_flipscreen;

	/* video-related */
	tilemap_t    *m_tilemap;

	/* devices */
	cpu_device *m_audiocpu;
	cpu_device *m_audiocpu_2;

	UINT8	   m_master_nmi_mask;
	UINT8      m_slave_irq_mask;
};


/*----------- defined in video/gyruss.c -----------*/

WRITE8_HANDLER( gyruss_spriteram_w );
READ8_HANDLER( gyruss_scanline_r );

PALETTE_INIT( gyruss );
VIDEO_START( gyruss );
SCREEN_UPDATE( gyruss );
