/***************************************************************************

    Atari Star Wars hardware

***************************************************************************/

#include "machine/6532riot.h"


class starwars_state : public driver_device
{
public:
	starwars_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_sound_data;
	UINT8 m_main_data;
	device_t *m_riot;
	UINT8 *m_slapstic_source;
	UINT8 *m_slapstic_base;
	UINT8 m_slapstic_current_bank;
	offs_t m_slapstic_last_pc;
	offs_t m_slapstic_last_address;
	UINT8 m_is_esb;
	UINT8 *m_mathram;
	UINT8 m_control_num;
	int m_MPA;
	int m_BIC;
	UINT16 m_dvd_shift;
	UINT16 m_quotient_shift;
	UINT16 m_divisor;
	UINT16 m_dividend;
	UINT8 *m_PROM_STR;
	UINT8 *m_PROM_MAS;
	UINT8 *m_PROM_AM;
	int m_math_run;
	emu_timer *m_math_timer;
	INT16 m_A;
	INT16 m_B;
	INT16 m_C;
	INT32 m_ACC;
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(esb_slapstic_r);
	DECLARE_WRITE8_MEMBER(esb_slapstic_w);
	DECLARE_WRITE8_MEMBER(starwars_nstore_w);
	DECLARE_WRITE8_MEMBER(starwars_out_w);
	DECLARE_READ8_MEMBER(starwars_adc_r);
	DECLARE_WRITE8_MEMBER(starwars_adc_select_w);
	DECLARE_READ8_MEMBER(starwars_prng_r);
	DECLARE_READ8_MEMBER(starwars_div_reh_r);
	DECLARE_READ8_MEMBER(starwars_div_rel_r);
	DECLARE_WRITE8_MEMBER(starwars_math_w);
};


/*----------- defined in machine/starwars.c -----------*/


CUSTOM_INPUT( matrix_flag_r );


void starwars_mproc_init(running_machine &machine);
void starwars_mproc_reset(running_machine &machine);




/*----------- defined in audio/starwars.c -----------*/

extern const riot6532_interface starwars_riot6532_intf;

SOUND_START( starwars );

READ8_HANDLER( starwars_main_read_r );
READ8_HANDLER( starwars_main_ready_flag_r );
WRITE8_HANDLER( starwars_main_wr_w );
WRITE8_HANDLER( starwars_soundrst_w );

READ8_HANDLER( starwars_sin_r );
WRITE8_HANDLER( starwars_sout_w );
