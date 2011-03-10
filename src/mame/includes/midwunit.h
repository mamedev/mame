/*************************************************************************

    Driver for Midway Wolf-unit games.

**************************************************************************/

class midwunit_state : public driver_device
{
public:
	midwunit_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT16>	m_nvram;
};

/*----------- defined in machine/midwunit.c -----------*/

extern UINT8 *midwunit_decode_memory;

WRITE16_HANDLER( midwunit_cmos_enable_w );
WRITE16_HANDLER( midwunit_cmos_w );
READ16_HANDLER( midwunit_cmos_r );

WRITE16_HANDLER( midwunit_io_w );

READ16_HANDLER( midwunit_io_r );

DRIVER_INIT( mk3 );
DRIVER_INIT( mk3r20 );
DRIVER_INIT( mk3r10 );
DRIVER_INIT( umk3 );
DRIVER_INIT( umk3r11 );

DRIVER_INIT( openice );
DRIVER_INIT( nbahangt );
DRIVER_INIT( wwfmania );
DRIVER_INIT( rmpgwt );

MACHINE_RESET( midwunit );

READ16_HANDLER( midwunit_security_r );
WRITE16_HANDLER( midwunit_security_w );

READ16_HANDLER( midwunit_sound_r );
WRITE16_HANDLER( midwunit_sound_w );
