// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    z80daisy.c

    Z80/180 daisy chaining support functions.

***************************************************************************/

#include "emu.h"
#include "z80daisy.h"


//**************************************************************************
//  DEVICE Z80 DAISY INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_z80daisy_interface - constructor
//-------------------------------------------------

device_z80daisy_interface::device_z80daisy_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "z80daisy"),
		m_daisy_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_z80daisy_interface - destructor
//-------------------------------------------------

device_z80daisy_interface::~device_z80daisy_interface()
{
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
//  static_set_daisy_config - configuration helper
//  to set up the daisy chain
//-------------------------------------------------
void z80_daisy_chain_interface::static_set_daisy_config(device_t &device, const z80_daisy_config *config)
{
	z80_daisy_chain_interface *daisyintf;
	if (!device.interface(daisyintf))
		throw emu_fatalerror("MCFG_Z80_DAISY_CHAIN called on device '%s' with no daisy chain interface", device.tag());
	daisyintf->m_daisy_config = config;
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

		// append to the end, or overwrite existing entry
		device_z80daisy_interface *next = (*tailptr != nullptr) ? (*tailptr)->m_daisy_next : nullptr;
		*tailptr = intf;
		(*tailptr)->m_daisy_next = next;
		tailptr = &(*tailptr)->m_daisy_next;
	}
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
//  call_ack_device - acknowledge an interrupt
//  from a chained device and return the vector
//-------------------------------------------------

int z80_daisy_chain_interface::daisy_call_ack_device()
{
	int vector = 0;

	// loop over all devices; dev[0] is the highest priority
	for (device_z80daisy_interface *intf = m_chain; intf != nullptr; intf = intf->m_daisy_next)
	{
		// if this device is asserting the INT line, that's the one we want
		int state = intf->z80daisy_irq_state();
		vector = intf->z80daisy_irq_ack();
		if (state & Z80_DAISY_INT)
			return vector;
	}
	//logerror("z80daisy_call_ack_device: failed to find an device to ack!\n");
	return vector;
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
			intf->z80daisy_irq_reti();
			return;
		}
	}
	//logerror("z80daisy_call_reti_device: failed to find an device to reti!\n");
}
