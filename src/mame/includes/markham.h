/*************************************************************************

    Markham

*************************************************************************/

class markham_state : public driver_device
{
public:
	markham_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_xscroll;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	DECLARE_READ8_MEMBER(markham_e004_r);
};


/*----------- defined in video/markham.c -----------*/

WRITE8_HANDLER( markham_videoram_w );
WRITE8_HANDLER( markham_flipscreen_w );

PALETTE_INIT( markham );
VIDEO_START( markham );
SCREEN_UPDATE_IND16( markham );
