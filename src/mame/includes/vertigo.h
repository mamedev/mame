/*************************************************************************

    Exidy Vertigo hardware

*************************************************************************/

/*----------- defined in machine/vertigo.c -----------*/

void vertigo_update_irq(running_device *device);

extern const struct pit8253_config vertigo_pit8254_config;

READ16_HANDLER( vertigo_io_convert );
READ16_HANDLER( vertigo_io_adc );
READ16_HANDLER( vertigo_coin_r );
READ16_HANDLER( vertigo_sio_r );
WRITE16_HANDLER( vertigo_audio_w );
WRITE16_HANDLER( vertigo_motor_w );
WRITE16_HANDLER( vertigo_wsot_w );

INTERRUPT_GEN( vertigo_interrupt );
MACHINE_START( vertigo );
MACHINE_RESET( vertigo );

/*----------- defined in video/vertigo.c -----------*/

extern UINT16 *vertigo_vectorram;

void vertigo_vproc_init(running_machine *machine);
void vertigo_vproc_reset(running_machine *machine);
void vertigo_vproc(int cycles, int irq4);

