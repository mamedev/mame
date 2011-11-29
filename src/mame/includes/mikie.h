/*************************************************************************

    Mikie

*************************************************************************/

class mikie_state : public driver_device
{
public:
	mikie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int        m_palettebank;

	/* misc */
	int        m_last_irq;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;

	UINT8      m_irq_mask;
};


/*----------- defined in video/mikie.c -----------*/

WRITE8_HANDLER( mikie_videoram_w );
WRITE8_HANDLER( mikie_colorram_w );
WRITE8_HANDLER( mikie_palettebank_w );
WRITE8_HANDLER( mikie_flipscreen_w );

PALETTE_INIT( mikie );
VIDEO_START( mikie );
SCREEN_UPDATE( mikie );
