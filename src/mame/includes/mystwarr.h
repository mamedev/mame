class mystwarr_state : public driver_device
{
public:
	mystwarr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *gx_workram;
	UINT8 mw_irq_control;
	int cur_sound_region;
	int layer_colorbase[6];
	int oinprion;
	int cbparam;
	int sprite_colorbase;
	int sub1_colorbase;
	int last_psac_colorbase;
	int gametype;
	int roz_enable;
	int roz_rombank;
	tilemap_t *ult_936_tilemap;
	UINT16 clip;
};


/*----------- defined in video/mystwarr.c -----------*/

VIDEO_START( gaiapols );
VIDEO_START( dadandrn );
VIDEO_START( viostorm );
VIDEO_START( metamrph );
VIDEO_START( martchmp );
VIDEO_START( mystwarr );
VIDEO_UPDATE( dadandrn );
VIDEO_UPDATE( mystwarr );
VIDEO_UPDATE( metamrph );
VIDEO_UPDATE( martchmp );

WRITE16_HANDLER( ddd_053936_enable_w );
WRITE16_HANDLER( ddd_053936_clip_w );
READ16_HANDLER( gai_053936_tilerom_0_r );
READ16_HANDLER( ddd_053936_tilerom_0_r );
READ16_HANDLER( ddd_053936_tilerom_1_r );
READ16_HANDLER( gai_053936_tilerom_2_r );
READ16_HANDLER( ddd_053936_tilerom_2_r );
