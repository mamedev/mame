/*************************************************************************

    Cross Pang

*************************************************************************/

class crospang_state : public driver_device
{
public:
	crospang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_bg_videoram;
	UINT16 *  m_fg_videoram;
	UINT16 *  m_spriteram;
//  UINT16 *  m_paletteram;       // currently this uses generic palette handling
	size_t    m_spriteram_size;

	/* video-related */
	tilemap_t   *m_bg_layer;
	tilemap_t   *m_fg_layer;
	int       m_xsproff;
	int       m_ysproff;
	int       m_bestri_tilebank;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/crospang.c -----------*/

VIDEO_START( crospang );
SCREEN_UPDATE( crospang );

WRITE16_HANDLER ( crospang_fg_scrolly_w );
WRITE16_HANDLER ( crospang_bg_scrolly_w );
WRITE16_HANDLER ( crospang_fg_scrollx_w );
WRITE16_HANDLER ( crospang_bg_scrollx_w );

WRITE16_HANDLER ( bestri_fg_scrolly_w );
WRITE16_HANDLER ( bestri_bg_scrolly_w );
WRITE16_HANDLER ( bestri_fg_scrollx_w );
WRITE16_HANDLER ( bestri_bg_scrollx_w );

WRITE16_HANDLER ( crospang_fg_videoram_w );
WRITE16_HANDLER ( crospang_bg_videoram_w );
WRITE16_HANDLER ( bestri_tilebank_w );
