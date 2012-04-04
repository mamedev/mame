/*************************************************************************

    City Connection

*************************************************************************/

class citycon_state : public driver_device
{
public:
	citycon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_videoram;
	UINT8 *        m_linecolor;
	UINT8 *        m_scroll;
	UINT8 *        m_spriteram;
//  UINT8 *        m_paletteram;  // currently this uses generic palette handling
	size_t         m_spriteram_size;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	int            m_bg_image;

	/* devices */
	device_t *m_maincpu;
	DECLARE_READ8_MEMBER(citycon_in_r);
	DECLARE_READ8_MEMBER(citycon_irq_ack_r);
};


/*----------- defined in video/citycon.c -----------*/

WRITE8_HANDLER( citycon_videoram_w );
WRITE8_HANDLER( citycon_linecolor_w );
WRITE8_HANDLER( citycon_background_w );

SCREEN_UPDATE_IND16( citycon );
VIDEO_START( citycon );
