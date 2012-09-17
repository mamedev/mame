/***************************************************************************

  pcecommn.h

  Headers for the common stuff for NEC PC Engine/TurboGrafx16.

***************************************************************************/

#ifndef PCECOMMON_H
#define PCECOMMON_H

#define	PCE_MAIN_CLOCK		21477270

DECLARE_WRITE8_HANDLER ( pce_joystick_w );
 DECLARE_READ8_HANDLER ( pce_joystick_r );

#define TG_16_JOY_SIG		0x00
#define PCE_JOY_SIG			0x40
#define NO_CD_SIG			0x80
#define CD_SIG				0x00
/* these might be used to indicate something, but they always seem to return 1 */
#define CONST_SIG			0x30

struct pce_struct
{
	UINT8 io_port_options; /*driver-specific options for the PCE*/
};
extern struct pce_struct pce;
void init_pce();
MACHINE_RESET( pce );

void pce_set_joystick_readinputport_callback( UINT8 (*joy_read)(running_machine &));
#endif
