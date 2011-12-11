class mystwarr_state : public driver_device
{
public:
	mystwarr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT16 *m_gx_workram;
	UINT8 m_mw_irq_control;
	int m_cur_sound_region;
	int m_layer_colorbase[6];
	int m_oinprion;
	int m_cbparam;
	int m_sprite_colorbase;
	int m_sub1_colorbase;
	int m_last_psac_colorbase;
	int m_gametype;
	int m_roz_enable;
	int m_roz_rombank;
	tilemap_t *m_ult_936_tilemap;
	UINT16 m_clip;
	UINT16 *m_spriteram;

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in video/mystwarr.c -----------*/

VIDEO_START( gaiapols );
VIDEO_START( dadandrn );
VIDEO_START( viostorm );
VIDEO_START( metamrph );
VIDEO_START( martchmp );
VIDEO_START( mystwarr );
SCREEN_UPDATE( dadandrn );
SCREEN_UPDATE( mystwarr );
SCREEN_UPDATE( metamrph );
SCREEN_UPDATE( martchmp );

WRITE16_HANDLER( ddd_053936_enable_w );
WRITE16_HANDLER( ddd_053936_clip_w );
READ16_HANDLER( gai_053936_tilerom_0_r );
READ16_HANDLER( ddd_053936_tilerom_0_r );
READ16_HANDLER( ddd_053936_tilerom_1_r );
READ16_HANDLER( gai_053936_tilerom_2_r );
READ16_HANDLER( ddd_053936_tilerom_2_r );
