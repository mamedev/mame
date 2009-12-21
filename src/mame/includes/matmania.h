/*----------- defined in machine/maniach.c -----------*/

READ8_HANDLER( maniach_68705_portA_r );
WRITE8_HANDLER( maniach_68705_portA_w );
READ8_HANDLER( maniach_68705_portB_r );
WRITE8_HANDLER( maniach_68705_portB_w );
READ8_HANDLER( maniach_68705_portC_r );
WRITE8_HANDLER( maniach_68705_portC_w );
WRITE8_HANDLER( maniach_68705_ddrA_w );
WRITE8_HANDLER( maniach_68705_ddrB_w );
WRITE8_HANDLER( maniach_68705_ddrC_w );
WRITE8_HANDLER( maniach_mcu_w );
READ8_HANDLER( maniach_mcu_r );
READ8_HANDLER( maniach_mcu_status_r );


/*----------- defined in video/matmania.c -----------*/

extern UINT8 *matmania_videoram,*matmania_colorram;
extern size_t matmania_videoram_size;
extern UINT8 *matmania_videoram2,*matmania_colorram2;
extern size_t matmania_videoram2_size;
extern UINT8 *matmania_videoram3,*matmania_colorram3;
extern size_t matmania_videoram3_size;
extern UINT8 *matmania_scroll;
extern UINT8 *matmania_pageselect;

WRITE8_HANDLER( matmania_paletteram_w );
PALETTE_INIT( matmania );
VIDEO_UPDATE( maniach );
VIDEO_START( matmania );
VIDEO_UPDATE( matmania );
