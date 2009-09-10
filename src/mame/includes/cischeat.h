/*----------- defined in drivers/cischeat.c -----------*/

extern UINT16 scudhamm_motor_command;

READ16_HANDLER( scudhamm_motor_pos_r );
READ16_HANDLER( scudhamm_motor_status_r );
READ16_HANDLER( scudhamm_analog_r );


/*----------- defined in video/cischeat.c -----------*/

extern UINT16 *cischeat_roadram[2];
extern UINT16 *f1gpstr2_ioready;

READ16_HANDLER( bigrun_vregs_r );
READ16_HANDLER( cischeat_vregs_r );
READ16_HANDLER( f1gpstar_vregs_r );
READ16_HANDLER( f1gpstr2_vregs_r );
READ16_HANDLER( wildplt_vregs_r );

WRITE16_HANDLER( bigrun_vregs_w );
WRITE16_HANDLER( cischeat_vregs_w );
WRITE16_HANDLER( f1gpstar_vregs_w );
WRITE16_HANDLER( f1gpstr2_vregs_w );
WRITE16_HANDLER( scudhamm_vregs_w );

CUSTOM_INPUT( cischeat_shift_r );

VIDEO_START( bigrun );
VIDEO_START( cischeat );
VIDEO_START( f1gpstar );

VIDEO_UPDATE( bigrun );
VIDEO_UPDATE( cischeat );
VIDEO_UPDATE( f1gpstar );
VIDEO_UPDATE( scudhamm );
