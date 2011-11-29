/*************************************************************************

    Pinball Action

*************************************************************************/

class pbaction_state : public driver_device
{
public:
	pbaction_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_videoram2;
	UINT8 *    m_colorram;
	UINT8 *    m_colorram2;
	UINT8 *    m_work_ram;
	UINT8 *    m_spriteram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	int        m_scroll;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;

	UINT8      m_nmi_mask;
};


/*----------- defined in video/pbaction.c -----------*/

extern WRITE8_HANDLER( pbaction_videoram_w );
extern WRITE8_HANDLER( pbaction_colorram_w );
extern WRITE8_HANDLER( pbaction_videoram2_w );
extern WRITE8_HANDLER( pbaction_colorram2_w );
extern WRITE8_HANDLER( pbaction_flipscreen_w );
extern WRITE8_HANDLER( pbaction_scroll_w );

extern VIDEO_START( pbaction );
extern SCREEN_UPDATE( pbaction );
