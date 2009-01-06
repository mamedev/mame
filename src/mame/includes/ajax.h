/*----------- defined in machine/ajax.c -----------*/

WRITE8_HANDLER( ajax_bankswitch_2_w );
READ8_HANDLER( ajax_ls138_f10_r );
WRITE8_HANDLER( ajax_ls138_f10_w );
MACHINE_START( ajax );
MACHINE_RESET( ajax );
INTERRUPT_GEN( ajax_interrupt );


/*----------- defined in video/ajax.c -----------*/

extern UINT8 ajax_priority;

VIDEO_START( ajax );
VIDEO_UPDATE( ajax );
