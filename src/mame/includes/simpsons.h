/*----------- defined in machine/simpsons.c -----------*/

extern int simpsons_firq_enabled;

WRITE8_HANDLER( simpsons_eeprom_w );
WRITE8_HANDLER( simpsons_coin_counter_w );
READ8_HANDLER( simpsons_sound_interrupt_r );
READ8_DEVICE_HANDLER( simpsons_sound_r );
MACHINE_RESET( simpsons );

/*----------- defined in video/simpsons.c -----------*/

extern UINT8 *simpsons_xtraram;

void simpsons_video_banking( running_machine *machine, int select );
VIDEO_UPDATE( simpsons );

extern void simpsons_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void simpsons_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);
