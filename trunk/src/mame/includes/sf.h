/*************************************************************************

    Street Fighter

*************************************************************************/

class sf_state : public driver_device
{
public:
	sf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_videoram;
	UINT16 *    m_objectram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling
	size_t      m_videoram_size;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_tx_tilemap;
	int         m_sf_active;
	UINT16      m_bgscroll;
	UINT16      m_fgscroll;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};


/*----------- defined in video/sf.c -----------*/

WRITE16_HANDLER( sf_bg_scroll_w );
WRITE16_HANDLER( sf_fg_scroll_w );
WRITE16_HANDLER( sf_videoram_w );
WRITE16_HANDLER( sf_gfxctrl_w );

VIDEO_START( sf );
SCREEN_UPDATE( sf );
