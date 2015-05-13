// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco ADAMnet bus emulation

**********************************************************************/

#include "adamnet.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ADAMNET = &device_creator<adamnet_device>;
const device_type ADAMNET_SLOT = &device_creator<adamnet_slot_device>;



//**************************************************************************
//  DEVICE ADAMNET CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_adamnet_card_interface - constructor
//-------------------------------------------------

device_adamnet_card_interface::device_adamnet_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_adamnet_card_interface - destructor
//-------------------------------------------------

device_adamnet_card_interface::~device_adamnet_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adamnet_slot_device - constructor
//-------------------------------------------------
adamnet_slot_device::adamnet_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ADAMNET_SLOT, "ADAMnet slot", tag, owner, clock, "adamnet_slot", __FILE__),
	device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adamnet_slot_device::device_start()
{
	m_bus = machine().device<adamnet_device>(ADAMNET_TAG);
	device_adamnet_card_interface *dev = dynamic_cast<device_adamnet_card_interface *>(get_card_device());
	if (dev) m_bus->add_device(get_card_device());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adamnet_device - constructor
//-------------------------------------------------

adamnet_device::adamnet_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ADAMNET, "ADAMnet bus", tag, owner, clock, "adamnet", __FILE__),
	m_txd(1),
	m_reset(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adamnet_device::device_start()
{
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void adamnet_device::device_stop()
{
	m_device_list.reset();
}


//-------------------------------------------------
//  add_adamnet_card - add ADAMNET card
//-------------------------------------------------

void adamnet_device::add_device(device_t *target)
{
	daisy_entry *entry = global_alloc(daisy_entry(target));

	entry->m_interface->m_bus = this;

	m_device_list.append(*entry);
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

adamnet_device::daisy_entry::daisy_entry(device_t *device)
	: m_next(NULL),
		m_device(device),
		m_interface(NULL),
		m_txd(1)
{
	device->interface(m_interface);
}


//-------------------------------------------------
//  rxd_r - receive data
//-------------------------------------------------

READ_LINE_MEMBER( adamnet_device::rxd_r )
{
	int state = m_txd;//1;

	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		state &= entry->m_txd;

		entry = entry->next();
	}

	return state;
}


//-------------------------------------------------
//  rxd_r - receive data
//-------------------------------------------------

int adamnet_device::rxd_r(device_t *device)
{
	int state = m_txd;

	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		//if (strcmp(entry->m_device->tag(), device->tag()))
		{
			state &= entry->m_txd;
		}

		entry = entry->next();
	}

	return state;
}


//-------------------------------------------------
//  txd_w - transmit data
//-------------------------------------------------

WRITE_LINE_MEMBER( adamnet_device::txd_w )
{
	if (m_txd != state)
	{
		m_txd = state;
	}
}


//-------------------------------------------------
//  txd_w - transmit data
//-------------------------------------------------

void adamnet_device::txd_w(device_t *device, int state)
{
	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		if (!strcmp(entry->m_device->tag(), device->tag()))
		{
			if (entry->m_txd != state)
			{
				entry->m_txd = state;
			}
			break;
		}

		entry = entry->next();
	}
}


//-------------------------------------------------
//  reset_r - bus reset
//-------------------------------------------------

READ_LINE_MEMBER( adamnet_device::reset_r )
{
	return m_reset;
}


//-------------------------------------------------
//  reset_w - bus reset
//-------------------------------------------------

WRITE_LINE_MEMBER( adamnet_device::reset_w )
{
	m_reset = state;

	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		entry->m_interface->adamnet_reset_w(state);

		entry = entry->next();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( adamnet_devices )
//-------------------------------------------------

// slot devices
#include "ddp.h"
#include "fdc.h"
#include "kb.h"
#include "printer.h"
#include "spi.h"

SLOT_INTERFACE_START( adamnet_devices )
	SLOT_INTERFACE("ddp", ADAM_DDP)
	SLOT_INTERFACE("fdc", ADAM_FDC)
	SLOT_INTERFACE("kb", ADAM_KB)
	SLOT_INTERFACE("prn", ADAM_PRN)
	SLOT_INTERFACE("spi", ADAM_SPI)
SLOT_INTERFACE_END
