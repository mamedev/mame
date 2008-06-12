/*----------- defined in machine/buggychl.c -----------*/

MACHINE_RESET( buggychl );
READ8_HANDLER( buggychl_68705_portA_r );
WRITE8_HANDLER( buggychl_68705_portA_w );
WRITE8_HANDLER( buggychl_68705_ddrA_w );
READ8_HANDLER( buggychl_68705_portB_r );
WRITE8_HANDLER( buggychl_68705_portB_w );
WRITE8_HANDLER( buggychl_68705_ddrB_w );
READ8_HANDLER( buggychl_68705_portC_r );
WRITE8_HANDLER( buggychl_68705_portC_w );
WRITE8_HANDLER( buggychl_68705_ddrC_w );
WRITE8_HANDLER( buggychl_mcu_w );
READ8_HANDLER( buggychl_mcu_r );
READ8_HANDLER( buggychl_mcu_status_r );


/*----------- defined in video/buggychl.c -----------*/

extern UINT8 *buggychl_scrollv,*buggychl_scrollh;
extern UINT8 *buggychl_character_ram;

PALETTE_INIT( buggychl );
VIDEO_START( buggychl );
WRITE8_HANDLER( buggychl_chargen_w );
WRITE8_HANDLER( buggychl_sprite_lookup_bank_w );
WRITE8_HANDLER( buggychl_sprite_lookup_w );
WRITE8_HANDLER( buggychl_ctrl_w );
WRITE8_HANDLER( buggychl_bg_scrollx_w );
VIDEO_UPDATE( buggychl );
