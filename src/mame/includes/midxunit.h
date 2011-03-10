/*************************************************************************

    Driver for Midway X-unit games.

**************************************************************************/

class midxunit_state : public driver_device
{
public:
	midxunit_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT16>	m_nvram;
};


/*----------- defined in machine/midxunit.c -----------*/

extern UINT8 *midxunit_decode_memory;

READ16_HANDLER( midxunit_cmos_r );
WRITE16_HANDLER( midxunit_cmos_w );

WRITE16_HANDLER( midxunit_io_w );
WRITE16_HANDLER( midxunit_unknown_w );

READ16_HANDLER( midxunit_io_r );
READ16_HANDLER( midxunit_analog_r );
WRITE16_HANDLER( midxunit_analog_select_w );
READ16_HANDLER( midxunit_status_r );

READ16_HANDLER( midxunit_uart_r );
WRITE16_HANDLER( midxunit_uart_w );

DRIVER_INIT( revx );

MACHINE_RESET( midxunit );

READ16_HANDLER( midxunit_security_r );
WRITE16_HANDLER( midxunit_security_w );
WRITE16_HANDLER( midxunit_security_clock_w );

READ16_HANDLER( midxunit_sound_r );
WRITE16_HANDLER( midxunit_sound_w );
