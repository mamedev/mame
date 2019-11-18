// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    z80daisy.c

    Z80/180 daisy chaining support functions.

***************************************************************************/

#include "emu.h"
#include "z80daisy.h"

#define VERBOSE 0


//**************************************************************************
//  DEVICE Z80 DAISY INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_z80daisy_interface - constructor
//-------------------------------------------------

device_z80daisy_interface::device_z80daisy_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "z80daisy"),
		m_daisy_next(nullptr),
		m_last_opcode(0)
{
}


//-------------------------------------------------
//  ~device_z80daisy_interface - destructor
//-------------------------------------------------

device_z80daisy_interface::~device_z80daisy_interface()
{
}


//-------------------------------------------------
//  interface_post_start - work to be done after
//  actually starting a device
//-------------------------------------------------

void device_z80daisy_interface::interface_post_start()
{
	device().save_item(NAME(m_last_opcode));
}


//-------------------------------------------------
//  interface_post_reset - work to be done after a
//  device is reset
//-------------------------------------------------

void device_z80daisy_interface::interface_post_reset()
{
	m_last_opcode = 0;
}


//-------------------------------------------------
//  z80daisy_decode - handle state machine that
//  decodes the RETI instruction from M1 fetches
//-------------------------------------------------

void device_z80daisy_interface::z80daisy_decode(uint8_t opcode)
{
	switch (m_last_opcode)
	{
	case 0xed:
		// ED 4D = RETI
		if (opcode == 0x4d)
			z80daisy_irq_reti();

		m_last_opcode = 0;
		break;

	case 0xcb:
	case 0xdd:
	case 0xfd:
		// CB xx, DD xx, FD xx are just two-byte opcodes
		m_last_opcode = 0;
		break;

	default:
		// TODO: ED affects IEO
		m_last_opcode = opcode;
		break;
	}
}


//**************************************************************************
//  Z80 DAISY CHAIN
//**************************************************************************

//-------------------------------------------------
//  z80_daisy_chain_interface - constructor
//-------------------------------------------------

z80_daisy_chain_interface::z80_daisy_chain_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "z80daisychain"),
		m_daisy_config(nullptr),
		m_chain(nullptr)
{
}


//-------------------------------------------------
//  z80_daisy_chain_interface - destructor
//-------------------------------------------------
z80_daisy_chain_interface::~z80_daisy_chain_interface()
{
}


//-------------------------------------------------
//  interface_post_start - work to be done after
//  actually starting a device
//-------------------------------------------------

void z80_daisy_chain_interface::interface_post_start()
{
	if (m_daisy_config != nullptr)
		daisy_init(m_daisy_config);
}

void z80_daisy_chain_interface::daisy_init(const z80_daisy_config *daisy)
{
	assert(daisy != nullptr);

	// create a linked list of devices
	device_z80daisy_interface **tailptr = &m_chain;
	for ( ; daisy->devname != nullptr; daisy++)
	{
		// find the device
		device_t *target = device().subdevice(daisy->devname);
		if (target == nullptr)
		{
			target = device().siblingdevice(daisy->devname);
			if (target == nullptr)
				fatalerror("Unable to locate device '%s'\n", daisy->devname);
		}

		// make sure it has an interface
		device_z80daisy_interface *intf;
		if (!target->interface(intf))
			fatalerror("Device '%s' does not implement the z80daisy interface!\n", daisy->devname);

		// splice it out of the list if it was previously added
		device_z80daisy_interface **oldtailptr = tailptr;
		while (*oldtailptr != nullptr)
		{
			if (*oldtailptr == intf)
				*oldtailptr = (*oldtailptr)->m_daisy_next;
			else
				oldtailptr = &(*oldtailptr)->m_daisy_next;
		}

		// add the interface to the list
		intf->m_daisy_next = *tailptr;
		*tailptr = intf;
		tailptr = &(*tailptr)->m_daisy_next;
	}

	osd_printf_verbose("Daisy chain = %s\n", daisy_show_chain());
}


//-------------------------------------------------
//  interface_post_reset - work to be done after a
//  device is reset
//-------------------------------------------------

void z80_daisy_chain_interface::interface_post_reset()
{
	// loop over all chained devices and call their reset function
	for (device_z80daisy_interface *intf = m_chain; intf != nullptr; intf = intf->m_daisy_next)
		intf->device().reset();
}


//-------------------------------------------------
//  update_irq_state - update the IRQ state and
//  return assert/clear based on the state
//-------------------------------------------------

int z80_daisy_chain_interface::daisy_update_irq_state()
{
	// loop over all devices; dev[0] is highest priority
	for (device_z80daisy_interface *intf = m_chain; intf != nullptr; intf = intf->m_daisy_next)
	{
		// if this device is asserting the INT line, that's the one we want
		int state = intf->z80daisy_irq_state();
		if (state & Z80_DAISY_INT)
			return ASSERT_LINE;

		// if this device is asserting the IEO line, it blocks everyone else
		if (state & Z80_DAISY_IEO)
			return CLEAR_LINE;
	}
	return CLEAR_LINE;
}


//-------------------------------------------------
//  daisy_get_irq_device - return the device
//  in the chain that requested the interrupt
//-------------------------------------------------

device_z80daisy_interface *z80_daisy_chain_interface::daisy_get_irq_device()
{
	// loop over all devices; dev[0] is the highest priority
	for (device_z80daisy_interface *intf = m_chain; intf != nullptr; intf = intf->m_daisy_next)
	{
		// if this device is asserting the INT line, that's the one we want
		int state = intf->z80daisy_irq_state();
		if (state & Z80_DAISY_INT)
			return intf;
	}

	if (VERBOSE && daisy_chain_present())
		device().logerror("Interrupt from outside Z80 daisy chain\n");
	return nullptr;
}


//-------------------------------------------------
//  call_reti_device - signal a RETI operator to
//  the chain
//-------------------------------------------------

void z80_daisy_chain_interface::daisy_call_reti_device()
{
	// loop over all devices; dev[0] is the highest priority
	for (device_z80daisy_interface *intf = m_chain; intf != nullptr; intf = intf->m_daisy_next)
	{
		// if this device is asserting the IEO line, that's the one we want
		int state = intf->z80daisy_irq_state();
		if (state & Z80_DAISY_IEO)
		{
			intf->z80daisy_decode(0xed);
			intf->z80daisy_decode(0x4d);
			return;
		}
	}
}


//-------------------------------------------------
//  daisy_show_chain - list devices in the chain
//  in string format (for debugging purposes)
//-------------------------------------------------

std::string z80_daisy_chain_interface::daisy_show_chain() const
{
	std::ostringstream result;

	// loop over all devices
	for (device_z80daisy_interface *intf = m_chain; intf != nullptr; intf = intf->m_daisy_next)
	{
		if (intf != m_chain)
			result << " -> ";
		result << intf->device().tag();
	}

	return result.str();
}
