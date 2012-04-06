/*************************************************************************

    Ginga NinkyouDen

*************************************************************************/

class ginganin_state : public driver_device
{
public:
	ginganin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_fgram;
	UINT16 *    m_txtram;
	UINT16 *    m_vregs;
	UINT16 *    m_spriteram;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_tx_tilemap;
	int         m_layers_ctrl;
	int         m_flipscreen;
#ifdef MAME_DEBUG
	int         m_posx;
	int         m_posy;
#endif

	/* devices */
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(ginganin_fgram16_w);
	DECLARE_WRITE16_MEMBER(ginganin_txtram16_w);
	DECLARE_WRITE16_MEMBER(ginganin_vregs16_w);
};



/*----------- defined in video/ginganin.c -----------*/


VIDEO_START( ginganin );
SCREEN_UPDATE_IND16( ginganin );
