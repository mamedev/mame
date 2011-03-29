/***************************************************************************

    Atari Star Wars hardware

***************************************************************************/

#include "machine/6532riot.h"


class starwars_state : public driver_device
{
public:
	starwars_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 sound_data;
	UINT8 main_data;
	device_t *riot;
	UINT8 *slapstic_source;
	UINT8 *slapstic_base;
	UINT8 slapstic_current_bank;
	offs_t slapstic_last_pc;
	offs_t slapstic_last_address;
	UINT8 is_esb;
	UINT8 *mathram;
	UINT8 control_num;
	int MPA;
	int BIC;
	UINT16 dvd_shift;
	UINT16 quotient_shift;
	UINT16 divisor;
	UINT16 dividend;
	UINT8 *PROM_STR;
	UINT8 *PROM_MAS;
	UINT8 *PROM_AM;
	int math_run;
	emu_timer *math_timer;
	INT16 A;
	INT16 B;
	INT16 C;
	INT32 ACC;
};


/*----------- defined in machine/starwars.c -----------*/

WRITE8_HANDLER( starwars_nstore_w );

WRITE8_HANDLER( starwars_out_w );
CUSTOM_INPUT( matrix_flag_r );

READ8_HANDLER( starwars_adc_r );
WRITE8_HANDLER( starwars_adc_select_w );

void starwars_mproc_init(running_machine &machine);
void starwars_mproc_reset(running_machine &machine);

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
