/*************************************************************************

    Driver for Midway X-unit games.

**************************************************************************/

class midxunit_state : public midtunit_state
{
public:
	midxunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: midtunit_state(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT16>	m_nvram;
	UINT8 *m_decode_memory;
	UINT8 m_cmos_write_enable;
	UINT16 m_iodata[8];
	UINT8 m_ioshuffle[16];
	UINT8 m_analog_port;
	UINT8 m_uart[8];
	UINT8 m_security_bits;
	DECLARE_READ16_MEMBER(midxunit_cmos_r);
	DECLARE_WRITE16_MEMBER(midxunit_cmos_w);
	DECLARE_WRITE16_MEMBER(midxunit_io_w);
	DECLARE_WRITE16_MEMBER(midxunit_unknown_w);
	DECLARE_READ16_MEMBER(midxunit_io_r);
	DECLARE_READ16_MEMBER(midxunit_analog_r);
	DECLARE_WRITE16_MEMBER(midxunit_analog_select_w);
	DECLARE_READ16_MEMBER(midxunit_status_r);
	DECLARE_READ16_MEMBER(midxunit_uart_r);
	DECLARE_WRITE16_MEMBER(midxunit_uart_w);
	DECLARE_READ16_MEMBER(midxunit_security_r);
	DECLARE_WRITE16_MEMBER(midxunit_security_w);
	DECLARE_WRITE16_MEMBER(midxunit_security_clock_w);
	DECLARE_READ16_MEMBER(midxunit_sound_r);
	DECLARE_READ16_MEMBER(midxunit_sound_state_r);
	DECLARE_WRITE16_MEMBER(midxunit_sound_w);
};


/*----------- defined in machine/midxunit.c -----------*/





DRIVER_INIT( revx );

MACHINE_RESET( midxunit );


