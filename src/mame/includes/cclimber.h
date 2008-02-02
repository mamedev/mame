/*----------- defined in machine/cclimber.c -----------*/

DRIVER_INIT( cclimber );
DRIVER_INIT( cclimbrj );
DRIVER_INIT( mshuttle );
DRIVER_INIT( cannonb );
DRIVER_INIT( cannonb2 );
DRIVER_INIT( ckongb );

/*----------- defined in video/cclimber.c -----------*/

extern UINT8 *cclimber_bsvideoram;
extern size_t cclimber_bsvideoram_size;
extern UINT8 *cclimber_bigspriteram;
extern UINT8 *cclimber_column_scroll;

extern UINT8 *swimmer_bgcolor;
extern UINT8 *swimmer_sidepanel_enabled;
extern UINT8 *swimmer_palettebank;

extern UINT8 *toprollr_videoram2;
extern UINT8 *toprollr_videoram3;
extern UINT8 *toprollr_videoram4;

WRITE8_HANDLER( cclimber_colorram_w );
PALETTE_INIT( cclimber );
VIDEO_UPDATE( cclimber );

VIDEO_UPDATE( cannonb );

VIDEO_UPDATE( yamato );
VIDEO_START( toprollr );
VIDEO_UPDATE( toprollr );

PALETTE_INIT( swimmer );
VIDEO_UPDATE( swimmer );


/*----------- defined in audio/cclimber.c -----------*/

extern const struct AY8910interface cclimber_ay8910_interface;
extern const struct Samplesinterface cclimber_samples_interface;
WRITE8_HANDLER( cclimber_sample_trigger_w );
WRITE8_HANDLER( cclimber_sample_rate_w );
WRITE8_HANDLER( cclimber_sample_volume_w );
