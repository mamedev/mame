/***************************************************************************

    Atari Star Wars hardware

***************************************************************************/

/*----------- defined in drivers/starwars.c -----------*/

extern UINT8 starwars_is_esb;


/*----------- defined in machine/starwars.c -----------*/

extern UINT8 *starwars_mathram;
extern UINT8 *starwars_ram_overlay;

WRITE8_HANDLER( starwars_nstore_w );

WRITE8_HANDLER( starwars_out_w );
READ8_HANDLER( starwars_input_1_r );

READ8_HANDLER( starwars_adc_r );
WRITE8_HANDLER( starwars_adc_select_w );

void swmathbox_init(void);
void swmathbox_reset(void);

READ8_HANDLER( swmathbx_prng_r );
READ8_HANDLER( swmathbx_reh_r );
READ8_HANDLER( swmathbx_rel_r );

WRITE8_HANDLER( swmathbx_w );


/*----------- defined in audio/starwars.c -----------*/

READ8_HANDLER( starwars_main_read_r );
READ8_HANDLER( starwars_main_ready_flag_r );
WRITE8_HANDLER( starwars_main_wr_w );
WRITE8_HANDLER( starwars_soundrst_w );

READ8_HANDLER( starwars_sin_r );
READ8_HANDLER( starwars_m6532_r );

WRITE8_HANDLER( starwars_sout_w );
WRITE8_HANDLER( starwars_m6532_w );
