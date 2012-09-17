
#include "emu.h"
#include "machine/pcecommn.h"
#include "video/vdc.h"
#include "cpu/h6280/h6280.h"

/* system RAM */
struct pce_struct pce;

/* joystick related data*/

#define JOY_CLOCK   0x01
#define JOY_RESET   0x02

static int joystick_port_select;        /* internal index of joystick ports */
static int joystick_data_select;        /* which nibble of joystick data we want */

static UINT8 (*pce_joystick_readinputport_callback)(running_machine &) = NULL;

void init_pce() {
	pce.io_port_options = PCE_JOY_SIG | CONST_SIG;
}

MACHINE_RESET( pce ) {
}

/* todo: how many input ports does the PCE have? */
WRITE8_HANDLER ( pce_joystick_w )
{
	h6280io_set_buffer(&space.device(), data);
    /* bump counter on a low-to-high transition of bit 1 */
    if((!joystick_data_select) && (data & JOY_CLOCK))
    {
        joystick_port_select = (joystick_port_select + 1) & 0x07;
    }

    /* do we want buttons or direction? */
    joystick_data_select = data & JOY_CLOCK;

    /* clear counter if bit 2 is set */
    if(data & JOY_RESET)
    {
        joystick_port_select = 0;
    }
}

READ8_HANDLER ( pce_joystick_r )
{
	UINT8 ret;
	int data;

	if ( pce_joystick_readinputport_callback != NULL )
	{
		data = pce_joystick_readinputport_callback(space.machine());
	}
	else
	{
		data = space.machine().root_device().ioport("JOY")->read();
	}
	if(joystick_data_select) data >>= 4;
	ret = (data & 0x0F) | pce.io_port_options;
#ifdef UNIFIED_PCE
	ret &= ~0x40;
#endif
	return (ret);
}

void pce_set_joystick_readinputport_callback( UINT8 (*joy_read)(running_machine &))
{
	pce_joystick_readinputport_callback = joy_read;
}
