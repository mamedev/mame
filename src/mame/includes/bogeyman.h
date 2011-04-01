/*************************************************************************

    Bogey Manor

*************************************************************************/

class bogeyman_state : public driver_device
{
public:
	bogeyman_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_videoram2;
	UINT8 *    m_colorram;
	UINT8 *    m_colorram2;
	UINT8 *    m_spriteram;
//  UINT8 *    m_paletteram;  // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;

	/* misc */
	int        m_psg_latch;
	int        m_last_write;
	int        m_colbank;
};


/*----------- defined in video/bogeyman.c -----------*/

WRITE8_HANDLER( bogeyman_videoram_w );
WRITE8_HANDLER( bogeyman_colorram_w );
WRITE8_HANDLER( bogeyman_videoram2_w );
WRITE8_HANDLER( bogeyman_colorram2_w );
WRITE8_HANDLER( bogeyman_paletteram_w );

PALETTE_INIT( bogeyman );
VIDEO_START( bogeyman );
SCREEN_UPDATE( bogeyman );
