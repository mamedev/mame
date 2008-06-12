/*----------- defined in machine/retofinv.c -----------*/

READ8_HANDLER( retofinv_68705_portA_r );
WRITE8_HANDLER( retofinv_68705_portA_w );
WRITE8_HANDLER( retofinv_68705_ddrA_w );
READ8_HANDLER( retofinv_68705_portB_r );
WRITE8_HANDLER( retofinv_68705_portB_w );
WRITE8_HANDLER( retofinv_68705_ddrB_w );
READ8_HANDLER( retofinv_68705_portC_r );
WRITE8_HANDLER( retofinv_68705_portC_w );
WRITE8_HANDLER( retofinv_68705_ddrC_w );
WRITE8_HANDLER( retofinv_mcu_w );
READ8_HANDLER( retofinv_mcu_r );
READ8_HANDLER( retofinv_mcu_status_r );


/*----------- defined in video/retofinv.c -----------*/

extern UINT8 *retofinv_fg_videoram;
extern UINT8 *retofinv_bg_videoram;
extern UINT8 *retofinv_sharedram;

VIDEO_START( retofinv );
PALETTE_INIT( retofinv );
VIDEO_UPDATE( retofinv );
WRITE8_HANDLER( retofinv_bg_videoram_w );
WRITE8_HANDLER( retofinv_fg_videoram_w );
WRITE8_HANDLER( retofinv_gfx_ctrl_w );
