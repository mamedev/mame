#include "machine/8255ppi.h"
#include "includes/galaxold.h"

class scramble_state : public galaxold_state
{
public:
	scramble_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxold_state(mconfig, type, tag) { }

	UINT8 *m_soundram;
	UINT8 m_cavelon_bank;
	UINT8 m_security_2B_counter;
	UINT8 m_xb;
	DECLARE_CUSTOM_INPUT_MEMBER(darkplnt_custom_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ckongs_coinage_r);
};


/*----------- defined in machine/scramble.c -----------*/

extern const ppi8255_interface scramble_ppi_0_intf;
extern const ppi8255_interface scramble_ppi_1_intf;
extern const ppi8255_interface stratgyx_ppi_1_intf;
extern const ppi8255_interface scramble_protection_ppi_1_intf;
extern const ppi8255_interface mrkougar_ppi_1_intf;


DRIVER_INIT( scramble_ppi );
DRIVER_INIT( stratgyx );
DRIVER_INIT( tazmani2 );
DRIVER_INIT( ckongs );
DRIVER_INIT( mariner );
DRIVER_INIT( devilfsh );
DRIVER_INIT( mars );
DRIVER_INIT( hotshock );
DRIVER_INIT( cavelon );
DRIVER_INIT( darkplnt );
DRIVER_INIT( mimonkey );
DRIVER_INIT( mimonsco );
DRIVER_INIT( mimonscr );
DRIVER_INIT( rescue );
DRIVER_INIT( minefld );
DRIVER_INIT( hustler );
DRIVER_INIT( hustlerd );
DRIVER_INIT( billiard );
DRIVER_INIT( mrkougar );
DRIVER_INIT( mrkougb );
DRIVER_INIT( ad2083 );

MACHINE_RESET( scramble );
MACHINE_RESET( explorer );

READ8_HANDLER( triplep_pip_r );
READ8_HANDLER( triplep_pap_r );

READ8_HANDLER( hunchbks_mirror_r );
WRITE8_HANDLER( hunchbks_mirror_w );


/*----------- defined in audio/scramble.c -----------*/

void scramble_sh_init(running_machine &machine);
WRITE_LINE_DEVICE_HANDLER( scramble_sh_7474_q_callback );

WRITE8_HANDLER( scramble_filter_w );
WRITE8_HANDLER( frogger_filter_w );

READ8_DEVICE_HANDLER( scramble_portB_r );
READ8_DEVICE_HANDLER( frogger_portB_r );

READ8_DEVICE_HANDLER( hotshock_soundlatch_r );

WRITE8_DEVICE_HANDLER( scramble_sh_irqtrigger_w );
WRITE8_DEVICE_HANDLER( mrkougar_sh_irqtrigger_w );
WRITE8_HANDLER( hotshock_sh_irqtrigger_w );

MACHINE_CONFIG_EXTERN( ad2083_audio );

