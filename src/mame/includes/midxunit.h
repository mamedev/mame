/*************************************************************************

    Driver for Midway X-unit games.

**************************************************************************/

class midxunit_state : public driver_device
{
public:
	midxunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT16>	m_nvram;
	UINT8 *m_decode_memory;
	UINT8 m_cmos_write_enable;
	UINT16 m_iodata[8];
	UINT8 m_ioshuffle[16];
	UINT8 m_analog_port;
	UINT8 m_uart[8];
	UINT8 m_security_bits;
};


/*----------- defined in machine/midxunit.c -----------*/

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
