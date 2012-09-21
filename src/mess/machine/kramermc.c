/***************************************************************************

        Kramer MC machine driver by Miodrag Milanovic

        13/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "machine/z80pio.h"
#include "includes/kramermc.h"


static READ8_DEVICE_HANDLER (kramermc_port_a_r)
{
	return 0xff;
}

static READ8_DEVICE_HANDLER (kramermc_port_b_r)
{
	kramermc_state *state = space.machine().driver_data<kramermc_state>();
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7" };

	return space.machine().root_device().ioport(keynames[state->m_key_row])->read();
}

static WRITE8_DEVICE_HANDLER (kramermc_port_a_w)
{
	kramermc_state *state = space.machine().driver_data<kramermc_state>();
	state->m_key_row = ((data >> 1) & 0x07);
}

Z80PIO_INTERFACE( kramermc_z80pio_intf )
{
	DEVCB_NULL,	/* callback when change interrupt status */
	DEVCB_HANDLER(kramermc_port_a_r),
	DEVCB_HANDLER(kramermc_port_a_w),
	DEVCB_NULL,
	DEVCB_HANDLER(kramermc_port_b_r),
	DEVCB_NULL,
	DEVCB_NULL
};

/* Driver initialization */
DRIVER_INIT_MEMBER(kramermc_state,kramermc)
{
}

void kramermc_state::machine_reset()
{
	m_key_row = 0;
}
