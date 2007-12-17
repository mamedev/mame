/*----------- defined in drivers/tnzs.c -----------*/

extern UINT8 *tnzs_objram, *tnzs_sharedram;
extern UINT8 *tnzs_vdcram, *tnzs_scrollram, *tnzs_objctrl, *tnzs_bg_flag;


/*----------- defined in machine/tnzs.c -----------*/

READ8_HANDLER( tnzs_port1_r );
READ8_HANDLER( tnzs_port2_r );
WRITE8_HANDLER( tnzs_port2_w );
READ8_HANDLER( arknoid2_sh_f000_r );

DRIVER_INIT( plumpop );
DRIVER_INIT( extrmatn );
DRIVER_INIT( arknoid2 );
DRIVER_INIT( drtoppel );
DRIVER_INIT( chukatai );
DRIVER_INIT( tnzs );
DRIVER_INIT( tnzsb );
DRIVER_INIT( kabukiz );
DRIVER_INIT( insectx );
DRIVER_INIT( kageki );

READ8_HANDLER( tnzs_mcu_r );
WRITE8_HANDLER( tnzs_mcu_w );
INTERRUPT_GEN( arknoid2_interrupt );
MACHINE_RESET( tnzs );
READ8_HANDLER( tnzs_sharedram_r );
WRITE8_HANDLER( tnzs_sharedram_w );
WRITE8_HANDLER( tnzs_bankswitch_w );
WRITE8_HANDLER( tnzs_bankswitch1_w );


/*----------- defined in video/tnzs.c -----------*/

PALETTE_INIT( arknoid2 );
VIDEO_UPDATE( tnzs );
VIDEO_EOF( tnzs );
