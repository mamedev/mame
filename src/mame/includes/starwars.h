/***************************************************************************

    Atari Star Wars hardware

***************************************************************************/

#include "machine/6532riot.h"


/*----------- defined in drivers/starwars.c -----------*/

extern UINT8 starwars_is_esb;


/*----------- defined in machine/starwars.c -----------*/

extern UINT8 *starwars_mathram;

WRITE8_DEVICE_HANDLER( starwars_nstore_w );

WRITE8_HANDLER( starwars_out_w );
CUSTOM_INPUT( matrix_flag_r );

READ8_HANDLER( starwars_adc_r );
WRITE8_HANDLER( starwars_adc_select_w );

void starwars_mproc_init(running_machine *machine);
void starwars_mproc_reset(void);

READ8_HANDLER( starwars_prng_r );
READ8_HANDLER( starwars_div_reh_r );
READ8_HANDLER( starwars_div_rel_r );

WRITE8_HANDLER( starwars_math_w );


/*----------- defined in audio/starwars.c -----------*/

extern const riot6532_interface starwars_riot6532_intf;

SOUND_START( starwars );

READ8_HANDLER( starwars_main_read_r );
READ8_HANDLER( starwars_main_ready_flag_r );
WRITE8_HANDLER( starwars_main_wr_w );
WRITE8_HANDLER( starwars_soundrst_w );

READ8_HANDLER( starwars_sin_r );
WRITE8_HANDLER( starwars_sout_w );
