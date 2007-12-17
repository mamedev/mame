/*************************************************************************

    Exidy Vertigo hardware

*************************************************************************/

/*----------- defined in machine/vertigo.c -----------*/

READ16_HANDLER( vertigo_io_convert );
READ16_HANDLER( vertigo_io_adc );
READ16_HANDLER( vertigo_coin_r );
READ16_HANDLER( vertigo_sio_r );
WRITE16_HANDLER( vertigo_audio_w );
WRITE16_HANDLER( vertigo_motor_w );
WRITE16_HANDLER( vertigo_wsot_w );

INTERRUPT_GEN( vertigo_interrupt );
MACHINE_RESET( vertigo );

/*----------- defined in video/vertigo.c -----------*/

extern UINT16 *vertigo_vectorram;

void vertigo_vproc_init(void);
void vertigo_vproc(int cycles, int irq4);

