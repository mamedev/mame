/*----------- defined in machine/lkage.c -----------*/

READ8_HANDLER( lkage_68705_portA_r );
WRITE8_HANDLER( lkage_68705_portA_w );
READ8_HANDLER( lkage_68705_portB_r );
WRITE8_HANDLER( lkage_68705_portB_w );
READ8_HANDLER( lkage_68705_portC_r );
WRITE8_HANDLER( lkage_68705_portC_w );
WRITE8_HANDLER( lkage_68705_ddrA_w );
WRITE8_HANDLER( lkage_68705_ddrB_w );
WRITE8_HANDLER( lkage_68705_ddrC_w );
WRITE8_HANDLER( lkage_mcu_w );
READ8_HANDLER( lkage_mcu_r );
READ8_HANDLER( lkage_mcu_status_r );


/*----------- defined in video/lkage.c -----------*/

extern UINT8 *lkage_scroll, *lkage_vreg;
WRITE8_HANDLER( lkage_videoram_w );
VIDEO_START( lkage );
VIDEO_UPDATE( lkage );

