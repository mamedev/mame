/*************************************************************************

    Ginga NinkyouDen

*************************************************************************/

class ginganin_state : public driver_device
{
public:
	ginganin_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
	/* sound-related */
	UINT8       m_MC6840_index0;
	UINT8       m_MC6840_register0;
	UINT8       m_MC6840_index1;
	UINT8       m_MC6840_register1;
	int         m_S_TEMPO;
	int         m_S_TEMPO_OLD;
	int         m_MC6809_CTR;
	int         m_MC6809_FLAG;

	/* devices */
	device_t *m_audiocpu;
};



/*----------- defined in video/ginganin.c -----------*/

WRITE16_HANDLER( ginganin_fgram16_w );
WRITE16_HANDLER( ginganin_txtram16_w );
WRITE16_HANDLER( ginganin_vregs16_w );

VIDEO_START( ginganin );
SCREEN_UPDATE( ginganin );
