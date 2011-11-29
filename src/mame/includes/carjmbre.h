/***************************************************************************

    carjmbre

***************************************************************************/

class carjmbre_state : public driver_device
{
public:
	carjmbre_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_videoram;
	UINT8 * m_spriteram;
	size_t  m_spriteram_size;

	/* video-related */
	tilemap_t *m_cj_tilemap;
	UINT8   m_flipscreen;
	UINT16  m_bgcolor;

	UINT8	m_nmi_mask;
};



/*----------- defined in video/carjmbre.c -----------*/

WRITE8_HANDLER( carjmbre_flipscreen_w );
WRITE8_HANDLER( carjmbre_bgcolor_w );
WRITE8_HANDLER( carjmbre_8806_w );
WRITE8_HANDLER( carjmbre_videoram_w );

PALETTE_INIT( carjmbre );
VIDEO_START( carjmbre );
SCREEN_UPDATE( carjmbre );


