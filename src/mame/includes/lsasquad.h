/*----------- defined in machine/daikaiju.c -----------*/

READ8_HANDLER( daikaiju_mcu_r);
WRITE8_HANDLER( daikaiju_mcu_w);
READ8_HANDLER( daikaiju_mcu_status_r);

MACHINE_RESET(daikaiju);


/*----------- defined in machine/lsasquad.c -----------*/

extern int lsasquad_invertcoin;
extern int lsasquad_sound_pending;

WRITE8_HANDLER( lsasquad_sh_nmi_disable_w );
WRITE8_HANDLER( lsasquad_sh_nmi_enable_w );
WRITE8_HANDLER( lsasquad_sound_command_w );
READ8_HANDLER( lsasquad_sh_sound_command_r );
WRITE8_HANDLER( lsasquad_sh_result_w );
READ8_HANDLER( lsasquad_sound_result_r );
READ8_HANDLER( lsasquad_sound_status_r );

READ8_HANDLER( lsasquad_68705_portA_r );
WRITE8_HANDLER( lsasquad_68705_portA_w );
WRITE8_HANDLER( lsasquad_68705_ddrA_w );
READ8_HANDLER( lsasquad_68705_portB_r );
WRITE8_HANDLER( lsasquad_68705_portB_w );
WRITE8_HANDLER( lsasquad_68705_ddrB_w );
WRITE8_HANDLER( lsasquad_mcu_w );
READ8_HANDLER( lsasquad_mcu_r );
READ8_HANDLER( lsasquad_mcu_status_r );

READ8_HANDLER( daikaiju_sound_status_r );
READ8_HANDLER( daikaiju_sh_sound_command_r );


/*----------- defined in video/lsasquad.c -----------*/

extern UINT8 *lsasquad_scrollram;

VIDEO_UPDATE( lsasquad );
VIDEO_UPDATE( daikaiju );
