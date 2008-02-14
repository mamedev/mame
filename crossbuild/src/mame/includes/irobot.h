/*************************************************************************

    Atari I, Robot hardware

*************************************************************************/

/*----------- defined in machine/irobot.c -----------*/

extern UINT8 irobot_vg_clear;
extern UINT8 irobot_bufsel;
extern UINT8 irobot_alphamap;
extern UINT8 *irobot_combase;

DRIVER_INIT( irobot );
MACHINE_RESET( irobot );

READ8_HANDLER( irobot_status_r );
WRITE8_HANDLER( irobot_statwr_w );
WRITE8_HANDLER( irobot_out0_w );
WRITE8_HANDLER( irobot_rom_banksel_w );
READ8_HANDLER( irobot_control_r );
WRITE8_HANDLER( irobot_control_w );
READ8_HANDLER( irobot_sharedmem_r );
WRITE8_HANDLER( irobot_sharedmem_w );


/*----------- defined in video/irobot.c -----------*/

PALETTE_INIT( irobot );
VIDEO_START( irobot );
VIDEO_UPDATE( irobot );

WRITE8_HANDLER( irobot_paletteram_w );

void irobot_poly_clear(void);
void irobot_run_video(void);
