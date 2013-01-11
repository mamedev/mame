/**********************************************************************

    Conitec Datensysteme ECB Bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "ecbbus.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ECBBUS_SLOT = &device_creator<ecbbus_slot_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ecbbus_slot_device - constructor
//-------------------------------------------------

ecbbus_slot_device::ecbbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ECBBUS_SLOT, "ECB bus slot", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_set_ecbbus_slot -
//-------------------------------------------------

void ecbbus_slot_device::static_set_ecbbus_slot(device_t &device, const char *tag, int num)
{
	ecbbus_slot_device &ecbbus_card = dynamic_cast<ecbbus_slot_device &>(device);
	ecbbus_card.m_bus_tag = tag;
	ecbbus_card.m_bus_num = num;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ecbbus_slot_device::device_start()
{
	m_bus = machine().device<ecbbus_device>(m_bus_tag);
	device_ecbbus_card_interface *dev = dynamic_cast<device_ecbbus_card_interface *>(get_card_device());
	if (dev) m_bus->add_ecbbus_card(dev, m_bus_num);
}



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ECBBUS = &device_creator<ecbbus_device>;


void ecbbus_device::static_set_cputag(device_t &device, const char *tag)
{
	ecbbus_device &ecbbus = downcast<ecbbus_device &>(device);
	ecbbus.m_cputag = tag;
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ecbbus_device::device_config_complete()
{
	// inherit a copy of the static data
	const ecbbus_interface *intf = reinterpret_cast<const ecbbus_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<ecbbus_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
		memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
	}
}



//**************************************************************************
//  DEVICE ECBBUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ecbbus_card_interface - constructor
//-------------------------------------------------

device_ecbbus_card_interface::device_ecbbus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_ecbbus_card_interface - destructor
//-------------------------------------------------

device_ecbbus_card_interface::~device_ecbbus_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ecbbus_device - constructor
//-------------------------------------------------

ecbbus_device::ecbbus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ECBBUS, "ECB bus", tag, owner, clock)
{
	for (int i = 0; i < MAX_ECBBUS_SLOTS; i++)
		m_ecbbus_device[i] = NULL;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ecbbus_device::device_start()
{
	m_maincpu = machine().device<cpu_device>(m_cputag);

	// resolve callbacks
	m_out_int_func.resolve(m_out_int_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ecbbus_device::device_reset()
{
}


//-------------------------------------------------
//  add_ecbbus_card - add ECB bus card
//-------------------------------------------------

void ecbbus_device::add_ecbbus_card(device_ecbbus_card_interface *card, int pos)
{
	m_ecbbus_device[pos] = card;
}


//-------------------------------------------------
//  mem_r -
//-------------------------------------------------

READ8_MEMBER( ecbbus_device::mem_r )
{
	UINT8 data = 0;

	for (int i = 0; i < MAX_ECBBUS_SLOTS; i++)
	{
		if (m_ecbbus_device[i] != NULL)
		{
			data |= m_ecbbus_device[i]->ecbbus_mem_r(offset);
		}
	}

	return data;
}


//-------------------------------------------------
//  mem_w -
//-------------------------------------------------

WRITE8_MEMBER( ecbbus_device::mem_w )
{
	for (int i = 0; i < MAX_ECBBUS_SLOTS; i++)
	{
		if (m_ecbbus_device[i] != NULL)
		{
			m_ecbbus_device[i]->ecbbus_mem_w(offset, data);
		}
	}
}


//-------------------------------------------------
//  io_r -
//-------------------------------------------------

READ8_MEMBER( ecbbus_device::io_r )
{
	UINT8 data = 0;

	for (int i = 0; i < MAX_ECBBUS_SLOTS; i++)
	{
		if (m_ecbbus_device[i] != NULL)
		{
			data |= m_ecbbus_device[i]->ecbbus_io_r(offset);
		}
	}

	return data;
}


//-------------------------------------------------
//  io_w -
//-------------------------------------------------

WRITE8_MEMBER( ecbbus_device::io_w )
{
	for (int i = 0; i < MAX_ECBBUS_SLOTS; i++)
	{
		if (m_ecbbus_device[i] != NULL)
		{
			m_ecbbus_device[i]->ecbbus_io_w(offset, data);
		}
	}
}


//-------------------------------------------------
//  int_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ecbbus_device::int_w )
{
	m_out_int_func(state);
}


//-------------------------------------------------
//  nmi_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ecbbus_device::nmi_w )
{
	m_out_nmi_func(state);
}
