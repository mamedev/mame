// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IEEE-488.1 General Purpose Interface Bus emulation
    (aka HP-IB, GPIB, CBM IEEE)

**********************************************************************/

#include "emu.h"
#include "ieee488.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


static const char *const SIGNAL_NAME[] = { "EOI", "DAV", "NRFD", "NDAC", "IFC", "SRQ", "ATN", "REN" };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(IEEE488,      ieee488_device,      "ieee488",      "IEEE-488 bus")
DEFINE_DEVICE_TYPE(IEEE488_SLOT, ieee488_slot_device, "ieee488_slot", "IEEE-488 slot")



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ieee488_interface - constructor
//-------------------------------------------------

device_ieee488_interface::device_ieee488_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "ieee488"), m_bus(nullptr), m_slot(nullptr), m_next(nullptr)
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

ieee488_slot_device::ieee488_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IEEE488_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_address(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ieee488_slot_device::device_start()
{
	ieee488_device* bus = nullptr;

	for (device_t *device = owner(); device != nullptr; device = device->owner())
	{
		bus = device->subdevice<ieee488_device>(IEEE488_TAG);
		if (bus != nullptr) break;
	}

	assert(bus);

	device_ieee488_interface *dev = dynamic_cast<device_ieee488_interface *>(get_card_device());
	if (dev) bus->add_device(this, get_card_device());
}


void ieee488_slot_device::add_cbm_slot(machine_config &config, const char *_tag, int _address, const char *_def_slot)
{
	ieee488_slot_device &slot(IEEE488_SLOT(config, _tag, 0));
	cbm_ieee488_devices(slot);
	slot.set_default_option(_def_slot);
	slot.set_address(_address);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ieee488_device - constructor
//-------------------------------------------------

ieee488_device::ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IEEE488, tag, owner, clock),
	m_write_eoi(*this),
	m_write_dav(*this),
	m_write_nrfd(*this),
	m_write_ndac(*this),
	m_write_ifc(*this),
	m_write_srq(*this),
	m_write_atn(*this),
	m_write_ren(*this),
	m_write_dio(*this),
	m_dio(0xff)
{
	for (auto & elem : m_line)
	{
		elem = 1;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ieee488_device::device_start()
{
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
	auto entry = new daisy_entry(target);

	entry->m_interface->m_bus = this;
	entry->m_interface->m_slot = slot;

	m_device_list.append(*entry);
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

ieee488_device::daisy_entry::daisy_entry(device_t *device)
	: m_next(nullptr),
		m_device(device),
		m_interface(nullptr),
		m_dio(0xff)
{
	for (auto & elem : m_line)
	{
		elem = 1;
	}

	device->interface(m_interface);
}


//-------------------------------------------------
//  set_signal -
//-------------------------------------------------

void ieee488_device::set_signal(device_t *device, int signal, int state)
{
	bool changed = false;
	int old_state = get_signal(signal);

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

	if (!changed) {
		return;
	}

	state = get_signal(signal);

	if (old_state != state) {
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

	daisy_entry *entry = m_device_list.first();

	while (state && entry)
		{
			if (!entry->m_line[signal])
				{
					state = 0;
					break;
				}

			entry = entry->next();
		}

	return state;
}


//-------------------------------------------------
//  set_data -
//-------------------------------------------------

void ieee488_device::set_data(device_t *device, uint8_t data)
{
	bool changed = false;
	uint8_t old_state = get_data();

	if (device == this)
	{
		if (LOG) logerror("%s IEEE488: '%s' DIO %02x\n", machine().describe_context(), tag(), data);
		if (m_dio != data) {
			m_dio = data;
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
				if (entry->m_dio != data)
				{
					if (LOG) logerror("%s IEEE488: '%s' DIO %02x\n", machine().describe_context(), device->tag(), data);
					entry->m_dio = data;
					changed = true;
				}
			}

			entry = entry->next();
		}
	}

	if (!changed) {
		return;
	}

	uint8_t new_state = get_data();
	if (old_state != new_state) {
		m_write_dio(new_state);
	}
}


//-------------------------------------------------
//  get_data -
//-------------------------------------------------

uint8_t ieee488_device::get_data()
{
	uint8_t data = m_dio;

	daisy_entry *entry = m_device_list.first();

	while (data && entry)
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

void cbm_ieee488_devices(device_slot_interface &device)
{
	device.option_add("c2040", C2040);
	device.option_add("c3040", C3040);
	device.option_add("c4040", C4040);
	device.option_add("c8050", C8050);
	device.option_add("c8250", C8250);
	device.option_add("sfd1001", SFD1001);
	device.option_add("c2031", C2031);
	device.option_add("c8280", C8280);
	device.option_add("d9060", D9060);
	device.option_add("d9090", D9090);
	device.option_add("softbox", SOFTBOX);
	device.option_add("hardbox", HARDBOX);
	device.option_add("shark", MSHARK);
	device.option_add("c4023", C4023);
}

//-------------------------------------------------
//  SLOT_INTERFACE( hp_ieee488_devices )
//-------------------------------------------------

// slot devices
#include "hp9122c.h"
#include "hp9133.h"
#include "hp9895.h"

void hp_ieee488_devices(device_slot_interface &device)
{
	device.option_add("hp9122c", HP9122C);
	device.option_add("hp9133", HP9133);
	device.option_add("hp9895", HP9895);
}

//-------------------------------------------------
//  SLOT_INTERFACE( remote488_devices )
//-------------------------------------------------

// slot devices
#include "remote488.h"

void remote488_devices(device_slot_interface &device)
{
	device.option_add("remote488", REMOTE488);
}


//-------------------------------------------------
//  SLOT_INTERFACE( grid_ieee488_devices )
//-------------------------------------------------

// slot devices
#include "grid2102.h"

void grid_ieee488_devices(device_slot_interface &device)
{
	device.option_add("grid2102", GRID2102).clock(XTAL(4'000'000));
	device.option_add("grid2101_floppy", GRID2101_FLOPPY).clock(XTAL(4'000'000));
	device.option_add("grid2101_hdd", GRID2101_HDD).clock(XTAL(4'000'000));
}
