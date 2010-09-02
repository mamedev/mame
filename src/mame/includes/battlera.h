/*----------- defined in video/battlera.c -----------*/

VIDEO_UPDATE( battlera );
VIDEO_START( battlera );
INTERRUPT_GEN( battlera_interrupt );

READ8_HANDLER( HuC6270_register_r );
WRITE8_HANDLER( HuC6270_register_w );
//READ8_HANDLER( HuC6270_data_r );
WRITE8_HANDLER( HuC6270_data_w );
WRITE8_HANDLER( battlera_palette_w );

READ8_HANDLER( HuC6270_debug_r );
WRITE8_HANDLER( HuC6270_debug_w );
