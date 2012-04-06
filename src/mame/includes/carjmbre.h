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
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(carjmbre_flipscreen_w);
	DECLARE_WRITE8_MEMBER(carjmbre_bgcolor_w);
	DECLARE_WRITE8_MEMBER(carjmbre_8806_w);
	DECLARE_WRITE8_MEMBER(carjmbre_videoram_w);
};



/*----------- defined in video/carjmbre.c -----------*/


PALETTE_INIT( carjmbre );
VIDEO_START( carjmbre );
SCREEN_UPDATE_IND16( carjmbre );


