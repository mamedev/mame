/*----------- defined in machine/slapfght.c -----------*/

MACHINE_RESET( slapfight );

extern UINT8 *slapfight_dpram;
extern size_t slapfight_dpram_size;
WRITE8_HANDLER( slapfight_dpram_w );
READ8_HANDLER( slapfight_dpram_r );

READ8_HANDLER( slapfight_port_00_r );
WRITE8_HANDLER( slapfight_port_00_w );
WRITE8_HANDLER( slapfight_port_01_w );
WRITE8_HANDLER( slapfight_port_06_w );
WRITE8_HANDLER( slapfight_port_07_w );
WRITE8_HANDLER( slapfight_port_08_w );
WRITE8_HANDLER( slapfight_port_09_w );

READ8_HANDLER( getstar_e803_r );

READ8_HANDLER ( tigerh_68705_portA_r );
WRITE8_HANDLER( tigerh_68705_portA_w );
READ8_HANDLER ( tigerh_68705_portB_r );
WRITE8_HANDLER( tigerh_68705_portB_w );
READ8_HANDLER ( tigerh_68705_portC_r );
WRITE8_HANDLER( tigerh_68705_portC_w );
WRITE8_HANDLER( tigerh_68705_ddrA_w );
WRITE8_HANDLER( tigerh_68705_ddrB_w );
WRITE8_HANDLER( tigerh_68705_ddrC_w );
WRITE8_HANDLER( tigerh_mcu_w );
READ8_HANDLER ( tigerh_mcu_r );
READ8_HANDLER ( tigerh_mcu_status_r );

WRITE8_HANDLER( getstar_sh_intenable_w );
INTERRUPT_GEN( getstar_interrupt );


/*----------- defined in video/slapfght.c -----------*/

extern UINT8 *slapfight_videoram;
extern UINT8 *slapfight_colorram;
extern size_t slapfight_videoram_size;
extern UINT8 *slapfight_scrollx_lo,*slapfight_scrollx_hi,*slapfight_scrolly;

VIDEO_UPDATE( slapfight );
VIDEO_UPDATE( perfrman );
VIDEO_START( slapfight );
VIDEO_START( perfrman );

WRITE8_HANDLER( slapfight_flipscreen_w );
WRITE8_HANDLER( slapfight_fixram_w );
WRITE8_HANDLER( slapfight_fixcol_w );
WRITE8_HANDLER( slapfight_videoram_w );
WRITE8_HANDLER( slapfight_colorram_w );
WRITE8_HANDLER( slapfight_palette_bank_w );
