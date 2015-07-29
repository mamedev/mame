// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IEEE-488.1 General Purpose Interface Bus emulation
    (aka HP-IB, GPIB, CBM IEEE)

**********************************************************************/

#include "ieee488.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


static const char *const SIGNAL_NAME[] = { "EOI", "DAV", "NRFD", "NDAC", "IFC", "SRQ", "ATN", "REN" };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type IEEE488 = &device_creator<ieee488_device>;
const device_type IEEE488_SLOT = &device_creator<ieee488_slot_device>;



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ieee488_interface - constructor
//-------------------------------------------------

device_ieee488_interface::device_ieee488_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_ieee488_interface - destructor
//-------------------------------------------------

device_ieee488_interface::~device_ieee488_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ieee488_slot_device - constructor
//-------------------------------------------------

ieee488_slot_device::ieee488_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, IEEE488_SLOT, "IEEE-488 slot", tag, owner, clock, "ieee488_slot", __FILE__),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ieee488_slot_device::device_start()
{
	ieee488_device* bus = NULL;

	for (device_t *device = owner(); device != NULL; device = device->owner())
	{
		bus = device->subdevice<ieee488_device>(IEEE488_TAG);
		if (bus != NULL) break;
	}

	assert(bus);

	device_ieee488_interface *dev = dynamic_cast<device_ieee488_interface *>(get_card_device());
	if (dev) bus->add_device(this, get_card_device());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ieee488_device - constructor
//-------------------------------------------------

ieee488_device::ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IEEE488, "IEEE-488 bus", tag, owner, clock, "ieee488", __FILE__),
		m_write_eoi(*this),
		m_write_dav(*this),
		m_write_nrfd(*this),
		m_write_ndac(*this),
		m_write_ifc(*this),
		m_write_srq(*this),
		m_write_atn(*this),
		m_write_ren(*this),
		m_dio(0xff)
{
	for (int i = 0; i < SIGNAL_COUNT; i++)
	{
		m_line[i] = 1;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ieee488_device::device_start()
{
	// resolve callbacks
	m_write_eoi.resolve_safe();
	m_write_dav.resolve_safe();
	m_write_nrfd.resolve_safe();
	m_write_ndac.resolve_safe();
	m_write_ifc.resolve_safe();
	m_write_srq.resolve_safe();
	m_write_atn.resolve_safe();
	m_write_ren.resolve_safe();
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ieee488_device::device_stop()
{
	m_device_list.reset();
}


//-------------------------------------------------
//  add_device -
//-------------------------------------------------

void ieee488_device::add_device(ieee488_slot_device *slot, device_t *target)
{
	daisy_entry *entry = global_alloc(daisy_entry(target));

	entry->m_interface->m_bus = this;
	entry->m_interface->m_slot = slot;

	m_device_list.append(*entry);
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

ieee488_device::daisy_entry::daisy_entry(device_t *device)
	: m_next(NULL),
		m_device(device),
		m_interface(NULL),
		m_dio(0xff)
{
	for (int i = 0; i < SIGNAL_COUNT; i++)
	{
		m_line[i] = 1;
	}

	device->interface(m_interface);
}


//-------------------------------------------------
//  set_signal -
//-------------------------------------------------

void ieee488_device::set_signal(device_t *device, int signal, int state)
{
	bool changed = false;

	if (device == this)
	{
		if (m_line[signal] != state)
		{
			if (LOG) logerror("%s IEEE488: '%s' %s %u\n", machine().describe_context(), tag(), SIGNAL_NAME[signal], state);
			m_line[signal] = state;
			changed = true;
		}
	}
	else
	{
		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			if (!strcmp(entry->m_device->tag(), device->tag()))
			{
				if (entry->m_line[signal] != state)
				{
					if (LOG) logerror("%s IEEE488: '%s' %s %u\n", machine().describe_context(), device->tag(), SIGNAL_NAME[signal], state);
					entry->m_line[signal] = state;
					changed = true;
				}
			}

			entry = entry->next();
		}
	}

	if (changed)
	{
		switch (signal)
		{
		case EOI:   m_write_eoi(state);  break;
		case DAV:   m_write_dav(state);  break;
		case NRFD:  m_write_nrfd(state); break;
		case NDAC:  m_write_ndac(state); break;
		case IFC:   m_write_ifc(state);  break;
		case SRQ:   m_write_srq(state);  break;
		case ATN:   m_write_atn(state);  break;
		case REN:   m_write_ren(state);  break;
		}

		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			switch (signal)
			{
			case EOI:
				entry->m_interface->ieee488_eoi(state);
				break;

			case DAV:
				entry->m_interface->ieee488_dav(state);
				break;

			case NRFD:
				entry->m_interface->ieee488_nrfd(state);
				break;

			case NDAC:
				entry->m_interface->ieee488_ndac(state);
				break;

			case IFC:
				entry->m_interface->ieee488_ifc(state);
				break;

			case SRQ:
				entry->m_interface->ieee488_srq(state);
				break;

			case ATN:
				entry->m_interface->ieee488_atn(state);
				break;

			case REN:
				entry->m_interface->ieee488_ren(state);
				break;
			}

			entry = entry->next();
		}

		if (LOG) logerror("IEEE488: EOI %u DAV %u NRFD %u NDAC %u IFC %u SRQ %u ATN %u REN %u DIO %02x\n",
			get_signal(EOI), get_signal(DAV), get_signal(NRFD), get_signal(NDAC),
			get_signal(IFC), get_signal(SRQ), get_signal(ATN), get_signal(REN), get_data());
	}
}


//-------------------------------------------------
//  get_signal -
//-------------------------------------------------

int ieee488_device::get_signal(int signal)
{
	int state = m_line[signal];

	if (state)
	{
		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			if (!entry->m_line[signal])
			{
				state = 0;
				break;
			}

			entry = entry->next();
		}
	}

	return state;
}


//-------------------------------------------------
//  set_data -
//-------------------------------------------------

void ieee488_device::set_data(device_t *device, UINT8 data)
{
	if (device == this)
	{
		if (LOG) logerror("%s IEEE488: '%s' DIO %02x\n", machine().describe_context(), tag(), data);

		m_dio = data;
	}
	else
	{
		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			if (!strcmp(entry->m_device->tag(), device->tag()))
			{
				if (entry->m_dio != data)
				{
					if (LOG) logerror("%s IEEE488: '%s' DIO %02x\n", machine().describe_context(), device->tag(), data);
					entry->m_dio = data;
				}
			}

			entry = entry->next();
		}
	}
}


//-------------------------------------------------
//  get_data -
//-------------------------------------------------

UINT8 ieee488_device::get_data()
{
	UINT8 data = m_dio;

	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		data &= entry->m_dio;

		entry = entry->next();
	}

	return data;
}


//-------------------------------------------------
//  SLOT_INTERFACE( cbm_ieee488_devices )
//-------------------------------------------------

// slot devices
#include "bus/cbmiec/c1526.h"
#include "c2031.h"
#include "c2040.h"
#include "c8050.h"
#include "c8280.h"
#include "d9060.h"
#include "hardbox.h"
#include "shark.h"
#include "softbox.h"

SLOT_INTERFACE_START( cbm_ieee488_devices )
	SLOT_INTERFACE("c2040", C2040)
	SLOT_INTERFACE("c3040", C3040)
	SLOT_INTERFACE("c4040", C4040)
	SLOT_INTERFACE("c8050", C8050)
	SLOT_INTERFACE("c8250", C8250)
	SLOT_INTERFACE("sfd1001", SFD1001)
	SLOT_INTERFACE("c2031", C2031)
	SLOT_INTERFACE("c8280", C8280)
	SLOT_INTERFACE("d9060", D9060)
	SLOT_INTERFACE("d9090", D9090)
	SLOT_INTERFACE("softbox", SOFTBOX)
	SLOT_INTERFACE("hardbox", HARDBOX)
	SLOT_INTERFACE("shark", SHARK)
	SLOT_INTERFACE("c4023", C4023)
SLOT_INTERFACE_END
