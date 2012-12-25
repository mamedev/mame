/***************************************************************************

	PC-9801 Keyboard simulation

***************************************************************************/

#include "emu.h"
#include "machine/pc9801_kbd.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type PC9801_KBD = &device_creator<pc9801_kbd_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_kbd_device - constructor
//-------------------------------------------------

pc9801_kbd_device::pc9801_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC9801_KBD, "pc9801_kbd", tag, owner, clock)
{

}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( pc9801_kbd )

INPUT_PORTS_END

ioport_constructor pc9801_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_kbd );
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void pc9801_kbd_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc9801_kbd_device::device_start()
{
    m_irq_func.resolve(m_irq_cb, *this);
   	m_rxtimer = timer_alloc(RX_TIMER);
   	m_rxtimer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_kbd_device::device_reset()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pc9801_kbd_device::device_config_complete()
{
	// inherit a copy of the static data
	const pc9801_kbd_interface *intf = reinterpret_cast<const pc9801_kbd_interface *>(static_config());
	if (intf != NULL)
		*static_cast<pc9801_kbd_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_irq_cb, 0, sizeof(m_irq_cb));
	}
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void pc9801_kbd_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// ...
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( pc9801_kbd_device::rx_r )
{
	m_irq_func(CLEAR_LINE);
	return 0;
}

WRITE8_MEMBER( pc9801_kbd_device::tx_w )
{
	// ...
}
