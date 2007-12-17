/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/


#define BALSENTE_MASTER_CLOCK	(20000000)
#define BALSENTE_CPU_CLOCK		(BALSENTE_MASTER_CLOCK / 16)
#define BALSENTE_PIXEL_CLOCK	(BALSENTE_MASTER_CLOCK / 4)
#define BALSENTE_HTOTAL			(0x140)
#define BALSENTE_HBEND			(0x000)
#define BALSENTE_HBSTART		(0x100)
#define BALSENTE_VTOTAL			(0x108)
#define BALSENTE_VBEND			(0x010)
#define BALSENTE_VBSTART		(0x100)


/*----------- defined in machine/balsente.c -----------*/

extern UINT8 balsente_shooter;
extern UINT8 balsente_shooter_x;
extern UINT8 balsente_shooter_y;
extern UINT8 balsente_adc_shift;
extern UINT16 *shrike_shared;
extern UINT16 *shrike_io;

MACHINE_RESET( balsente );

void balsente_noise_gen(int chip, int count, short *buffer);

WRITE8_HANDLER( balsente_random_reset_w );
READ8_HANDLER( balsente_random_num_r );

WRITE8_HANDLER( balsente_rombank_select_w );
WRITE8_HANDLER( balsente_rombank2_select_w );

WRITE8_HANDLER( balsente_misc_output_w );

READ8_HANDLER( balsente_m6850_r );
WRITE8_HANDLER( balsente_m6850_w );

READ8_HANDLER( balsente_m6850_sound_r );
WRITE8_HANDLER( balsente_m6850_sound_w );

INTERRUPT_GEN( balsente_update_analog_inputs );
READ8_HANDLER( balsente_adc_data_r );
WRITE8_HANDLER( balsente_adc_select_w );

READ8_HANDLER( balsente_counter_8253_r );
WRITE8_HANDLER( balsente_counter_8253_w );

READ8_HANDLER( balsente_counter_state_r );
WRITE8_HANDLER( balsente_counter_control_w );

WRITE8_HANDLER( balsente_chip_select_w );
WRITE8_HANDLER( balsente_dac_data_w );
WRITE8_HANDLER( balsente_register_addr_w );

READ8_HANDLER( nstocker_port2_r );
WRITE8_HANDLER( spiker_expand_w );
READ8_HANDLER( spiker_expand_r );
READ8_HANDLER( grudge_steering_r );

READ8_HANDLER( shrike_shared_6809_r );
WRITE8_HANDLER( shrike_shared_6809_w );

READ16_HANDLER( shrike_io_68k_r );
WRITE16_HANDLER( shrike_io_68k_w );

/*----------- defined in video/balsente.c -----------*/

VIDEO_START( balsente );
VIDEO_UPDATE( balsente );

WRITE8_HANDLER( balsente_videoram_w );
WRITE8_HANDLER( balsente_paletteram_w );
WRITE8_HANDLER( balsente_palette_select_w );
WRITE8_HANDLER( shrike_sprite_select_w );
