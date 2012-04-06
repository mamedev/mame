/*************************************************************************

    Bomb Jack

*************************************************************************/

class bombjack_state : public driver_device
{
public:
	bombjack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *		m_videoram;
	UINT8 *		m_colorram;
	UINT8 *		m_spriteram;
//  UINT8 *     m_paletteram;  // currently this uses generic palette handling
	size_t		m_spriteram_size;

	/* video-related */
	tilemap_t	*m_fg_tilemap;
	tilemap_t	*m_bg_tilemap;
	UINT8		m_background_image;

	/* sound-related */
	UINT8		m_latch;

	UINT8		m_nmi_mask;
	DECLARE_WRITE8_MEMBER(bombjack_soundlatch_w);
	DECLARE_READ8_MEMBER(bombjack_soundlatch_r);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(bombjack_videoram_w);
	DECLARE_WRITE8_MEMBER(bombjack_colorram_w);
	DECLARE_WRITE8_MEMBER(bombjack_background_w);
	DECLARE_WRITE8_MEMBER(bombjack_flipscreen_w);
};


/*----------- defined in video/bombjack.c -----------*/


VIDEO_START( bombjack );
SCREEN_UPDATE_IND16( bombjack );
