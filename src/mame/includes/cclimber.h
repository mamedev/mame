class cclimber_state : public driver_device
{
public:
	cclimber_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_spriteram;
	UINT8 *m_bigsprite_videoram;
	UINT8 *m_bigsprite_control;
	UINT8 *m_column_scroll;
	UINT8 *m_flip_screen;
	UINT8 *m_swimmer_background_color;
	UINT8 *m_swimmer_side_background_enabled;
	UINT8 *m_swimmer_palettebank;
	UINT8 *m_toprollr_bg_videoram;
	UINT8 *m_toprollr_bg_coloram;
	UINT8 m_yamato_p0;
	UINT8 m_yamato_p1;
	UINT8 m_toprollr_rombank;
	UINT8 m_nmi_mask;
	tilemap_t *m_pf_tilemap;
	tilemap_t *m_bs_tilemap;
	tilemap_t *m_toproller_bg_tilemap;
	DECLARE_WRITE8_MEMBER(swimmer_sh_soundlatch_w);
	DECLARE_WRITE8_MEMBER(yamato_p0_w);
	DECLARE_WRITE8_MEMBER(yamato_p1_w);
	DECLARE_READ8_MEMBER(yamato_p0_r);
	DECLARE_READ8_MEMBER(yamato_p1_r);
	DECLARE_WRITE8_MEMBER(toprollr_rombank_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(cclimber_colorram_w);
	DECLARE_WRITE8_MEMBER(cannonb_flip_screen_w);
};


/*----------- defined in machine/cclimber.c -----------*/

DRIVER_INIT( cclimber );
DRIVER_INIT( cclimberj );
DRIVER_INIT( cannonb );
DRIVER_INIT( cannonb2 );
DRIVER_INIT( ckongb );

/*----------- defined in video/cclimber.c -----------*/


PALETTE_INIT( cclimber );
VIDEO_START( cclimber );
SCREEN_UPDATE_IND16( cclimber );

PALETTE_INIT( swimmer );
VIDEO_START( swimmer );
SCREEN_UPDATE_IND16( swimmer );

PALETTE_INIT( yamato );
SCREEN_UPDATE_IND16( yamato );

PALETTE_INIT( toprollr );
VIDEO_START( toprollr );
SCREEN_UPDATE_IND16( toprollr );
