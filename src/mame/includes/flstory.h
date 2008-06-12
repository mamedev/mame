/*----------- defined in drivers/flstory.c -----------*/

extern UINT8 *onna34ro_workram;
extern UINT8 *victnine_workram;


/*----------- defined in machine/flstory.c -----------*/

READ8_HANDLER( flstory_68705_portA_r );
WRITE8_HANDLER( flstory_68705_portA_w );
READ8_HANDLER( flstory_68705_portB_r );
WRITE8_HANDLER( flstory_68705_portB_w );
READ8_HANDLER( flstory_68705_portC_r );
WRITE8_HANDLER( flstory_68705_portC_w );
WRITE8_HANDLER( flstory_68705_ddrA_w );
WRITE8_HANDLER( flstory_68705_ddrB_w );
WRITE8_HANDLER( flstory_68705_ddrC_w );
WRITE8_HANDLER( flstory_mcu_w );
READ8_HANDLER( flstory_mcu_r );
READ8_HANDLER( flstory_mcu_status_r );
WRITE8_HANDLER( onna34ro_mcu_w );
READ8_HANDLER( onna34ro_mcu_r );
READ8_HANDLER( onna34ro_mcu_status_r );
WRITE8_HANDLER( victnine_mcu_w );
READ8_HANDLER( victnine_mcu_r );
READ8_HANDLER( victnine_mcu_status_r );


/*----------- defined in video/flstory.c -----------*/

extern UINT8 *flstory_scrlram;

VIDEO_START( flstory );
VIDEO_UPDATE( flstory );
VIDEO_START( victnine );
VIDEO_UPDATE( victnine );

WRITE8_HANDLER( flstory_videoram_w );
READ8_HANDLER( flstory_palette_r );
WRITE8_HANDLER( flstory_palette_w );
WRITE8_HANDLER( flstory_gfxctrl_w );
WRITE8_HANDLER( flstory_scrlram_w );
READ8_HANDLER( victnine_gfxctrl_r );
WRITE8_HANDLER( victnine_gfxctrl_w );
