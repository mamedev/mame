/*************************************************************************

    Driver for Midway Wolf-unit games.

**************************************************************************/

#include "midtunit.h"

/*----------- defined in machine/midwunit.c -----------*/

extern UINT8 *midwunit_decode_memory;

WRITE16_HANDLER( midwunit_cmos_enable_w );
WRITE16_HANDLER( midwunit_cmos_w );
WRITE16_HANDLER( midxunit_cmos_w );
READ16_HANDLER( midwunit_cmos_r );

WRITE16_HANDLER( midwunit_io_w );
WRITE16_HANDLER( midxunit_io_w );
WRITE16_HANDLER( midxunit_unknown_w );

READ16_HANDLER( midwunit_io_r );
READ16_HANDLER( midxunit_io_r );
READ16_HANDLER( midxunit_analog_r );
WRITE16_HANDLER( midxunit_analog_select_w );
READ16_HANDLER( midxunit_status_r );

READ16_HANDLER( midxunit_uart_r );
WRITE16_HANDLER( midxunit_uart_w );

DRIVER_INIT( mk3 );
DRIVER_INIT( mk3r20 );
DRIVER_INIT( mk3r10 );
DRIVER_INIT( umk3 );
DRIVER_INIT( umk3r11 );

DRIVER_INIT( openice );
DRIVER_INIT( nbahangt );
DRIVER_INIT( wwfmania );
DRIVER_INIT( rmpgwt );
DRIVER_INIT( revx );

MACHINE_RESET( midwunit );
MACHINE_RESET( midxunit );

READ16_HANDLER( midwunit_security_r );
WRITE16_HANDLER( midwunit_security_w );
WRITE16_HANDLER( midxunit_security_w );
WRITE16_HANDLER( midxunit_security_clock_w );

READ16_HANDLER( midwunit_sound_r );
WRITE16_HANDLER( midwunit_sound_w );
