/*----------- defined in machine/bigevglf.c -----------*/

READ8_HANDLER( bigevglf_68705_portA_r );
WRITE8_HANDLER( bigevglf_68705_portA_w );
READ8_HANDLER( bigevglf_68705_portB_r );
WRITE8_HANDLER( bigevglf_68705_portB_w );
READ8_HANDLER( bigevglf_68705_portC_r );
WRITE8_HANDLER( bigevglf_68705_portC_w );
WRITE8_HANDLER( bigevglf_68705_ddrA_w );
WRITE8_HANDLER( bigevglf_68705_ddrB_w );
WRITE8_HANDLER( bigevglf_68705_ddrC_w );

WRITE8_HANDLER( bigevglf_mcu_w );
READ8_HANDLER( bigevglf_mcu_r );
READ8_HANDLER( bigevglf_mcu_status_r );


/*----------- defined in video/bigevglf.c -----------*/

extern UINT8 *bigevglf_spriteram1;
extern UINT8 *bigevglf_spriteram2;

VIDEO_START( bigevglf );
VIDEO_UPDATE( bigevglf );

READ8_HANDLER( bigevglf_vidram_r );
WRITE8_HANDLER( bigevglf_vidram_w );
WRITE8_HANDLER( bigevglf_vidram_addr_w );

WRITE8_HANDLER( bigevglf_gfxcontrol_w );
WRITE8_HANDLER( bigevglf_palette_w );
