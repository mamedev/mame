/*----------- defined in drivers/namcos1.c -----------*/

void namcos1_init_DACs(void);


/*----------- defined in machine/namcos1.c -----------*/

extern UINT8 *namcos1_paletteram;

WRITE8_HANDLER( namcos1_bankswitch_w );
WRITE8_HANDLER( namcos1_subcpu_bank_w );

WRITE8_HANDLER( namcos1_cpu_control_w );
WRITE8_HANDLER( namcos1_watchdog_w );
WRITE8_HANDLER( namcos1_sound_bankswitch_w );

WRITE8_HANDLER( namcos1_mcu_bankswitch_w );
WRITE8_HANDLER( namcos1_mcu_patch_w );

MACHINE_RESET( namcos1 );

DRIVER_INIT( shadowld );
DRIVER_INIT( dspirit );
DRIVER_INIT( quester );
DRIVER_INIT( blazer );
DRIVER_INIT( pacmania );
DRIVER_INIT( galaga88 );
DRIVER_INIT( ws );
DRIVER_INIT( berabohm );
DRIVER_INIT( alice );
DRIVER_INIT( bakutotu );
DRIVER_INIT( wldcourt );
DRIVER_INIT( splatter );
DRIVER_INIT( faceoff );
DRIVER_INIT( rompers );
DRIVER_INIT( blastoff );
DRIVER_INIT( ws89 );
DRIVER_INIT( dangseed );
DRIVER_INIT( ws90 );
DRIVER_INIT( pistoldm );
DRIVER_INIT( soukobdx );
DRIVER_INIT( puzlclub );
DRIVER_INIT( tankfrce );
DRIVER_INIT( tankfrc4 );

/*----------- defined in video/namcos1.c -----------*/

READ8_HANDLER( namcos1_videoram_r );
WRITE8_HANDLER( namcos1_videoram_w );
WRITE8_HANDLER( namcos1_paletteram_w );
READ8_HANDLER( namcos1_spriteram_r );
WRITE8_HANDLER( namcos1_spriteram_w );

VIDEO_START( namcos1 );
VIDEO_UPDATE( namcos1 );
VIDEO_EOF( namcos1 );
