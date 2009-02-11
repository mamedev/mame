/*----------- defined in machine/simpsons.c -----------*/

extern int simpsons_firq_enabled;

READ8_HANDLER( simpsons_eeprom_r );
WRITE8_HANDLER( simpsons_eeprom_w );
WRITE8_HANDLER( simpsons_coin_counter_w );
READ8_HANDLER( simpsons_sound_interrupt_r );
READ8_DEVICE_HANDLER( simpsons_sound_r );
MACHINE_RESET( simpsons );
NVRAM_HANDLER( simpsons );

/*----------- defined in video/simpsons.c -----------*/

extern UINT8 *simpsons_xtraram;

void simpsons_video_banking( running_machine *machine, int select );
VIDEO_START( simpsons );
VIDEO_UPDATE( simpsons );
