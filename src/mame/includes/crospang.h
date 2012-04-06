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
	int       m_bestri_tilebank;

	/* devices */
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(crospang_soundlatch_w);
	DECLARE_WRITE16_MEMBER(bestri_tilebank_w);
	DECLARE_WRITE16_MEMBER(bestri_bg_scrolly_w);
	DECLARE_WRITE16_MEMBER(bestri_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(bestri_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(bestri_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_scrolly_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_videoram_w);
};


/*----------- defined in video/crospang.c -----------*/

VIDEO_START( crospang );
SCREEN_UPDATE_IND16( crospang );



