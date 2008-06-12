/*----------- defined in drivers/nycaptor.c -----------*/

extern UINT8 *nycaptor_sharedram;
extern int nyc_gametype;


/*----------- defined in machine/nycaptor.c -----------*/

READ8_HANDLER( nycaptor_mcu_r );
READ8_HANDLER( nycaptor_mcu_status_r1 );
READ8_HANDLER( nycaptor_mcu_status_r2 );
READ8_HANDLER( nycaptor_68705_portC_r );
READ8_HANDLER( nycaptor_68705_portB_r );
READ8_HANDLER( nycaptor_68705_portA_r );

WRITE8_HANDLER( nycaptor_mcu_w );
WRITE8_HANDLER( nycaptor_68705_portA_w );
WRITE8_HANDLER( nycaptor_68705_portB_w );
WRITE8_HANDLER( nycaptor_68705_portC_w );
WRITE8_HANDLER( nycaptor_68705_ddrA_w );
WRITE8_HANDLER( nycaptor_68705_ddrB_w );
WRITE8_HANDLER( nycaptor_68705_ddrC_w );


/*----------- defined in video/nycaptor.c -----------*/

extern UINT8 *nycaptor_scrlram;

VIDEO_START( nycaptor );
VIDEO_UPDATE( nycaptor );


READ8_HANDLER( nycaptor_videoram_r );
READ8_HANDLER( nycaptor_spriteram_r );
READ8_HANDLER( nycaptor_palette_r );
READ8_HANDLER( nycaptor_gfxctrl_r );
READ8_HANDLER( nycaptor_scrlram_r );

WRITE8_HANDLER( nycaptor_videoram_w );
WRITE8_HANDLER( nycaptor_spriteram_w );
WRITE8_HANDLER( nycaptor_palette_w );
WRITE8_HANDLER( nycaptor_gfxctrl_w );
WRITE8_HANDLER( nycaptor_scrlram_w );
