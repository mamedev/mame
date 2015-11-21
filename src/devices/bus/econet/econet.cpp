// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn Computers Econet local area network emulation

**********************************************************************/

#include "econet.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


static const char *const SIGNAL_NAME[] = { "CLK", "DATA" };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ECONET = &device_creator<econet_device>;
const device_type ECONET_SLOT = &device_creator<econet_slot_device>;



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_econet_interface - constructor
//-------------------------------------------------

device_econet_interface::device_econet_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device), m_next(nullptr), m_econet(nullptr), m_address(0)
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  econet_slot_device - constructor
//-------------------------------------------------

econet_slot_device::econet_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ECONET_SLOT, "Econet station", tag, owner, clock, "econet_slot", __FILE__),
	device_slot_interface(mconfig, *this), m_address(0), m_econet(nullptr)
{
}


//-------------------------------------------------
//  static_set_slot -
//-------------------------------------------------

void econet_slot_device::static_set_slot(device_t &device, int address)
{
	econet_slot_device &econet_card = dynamic_cast<econet_slot_device &>(device);
	econet_card.m_address = address;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void econet_slot_device::device_start()
{
	m_econet = machine().device<econet_device>(ECONET_TAG);
	device_econet_interface *dev = dynamic_cast<device_econet_interface *>(get_card_device());
	if (dev) m_econet->add_device(get_card_device(), m_address);
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_signal -
//-------------------------------------------------

inline void econet_device::set_signal(device_t *device, int signal, int state)
{
	bool changed = false;

	if (device == this)
	{
		if (m_line[signal] != state)
		{
			if (LOG) logerror("Econet: '%s' %s %u\n", tag(), SIGNAL_NAME[signal], state);
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
					if (LOG) logerror("Econet: '%s' %s %u\n", device->tag(), SIGNAL_NAME[signal], state);
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
		case CLK:   m_write_clk(state);  break;
		case DATA:  m_write_data(state); break;
		}

		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			switch (signal)
			{
			case CLK:
				entry->m_interface->econet_clk(state);
				break;

			case DATA:
				entry->m_interface->econet_data(state);
				break;
			}

			entry = entry->next();
		}

		if (LOG) logerror("Econet: CLK %u DATA %u\n", get_signal(CLK), get_signal(DATA));
	}
}


//-------------------------------------------------
//  get_signal -
//-------------------------------------------------

inline int econet_device::get_signal(int signal)
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



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  econet_device - constructor
//-------------------------------------------------

econet_device::econet_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ECONET, "Econet", tag, owner, clock, "econet", __FILE__),
	m_write_clk(*this),
	m_write_data(*this)
{
	for (int i = 0; i < SIGNAL_COUNT; i++)
	{
		m_line[i] = 1;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void econet_device::device_start()
{
	// resolve callbacks
	m_write_clk.resolve_safe();
	m_write_data.resolve_safe();
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void econet_device::device_stop()
{
	m_device_list.reset();
}


//-------------------------------------------------
//  add_device -
//-------------------------------------------------

void econet_device::add_device(device_t *target, int address)
{
	daisy_entry *entry = global_alloc(daisy_entry(target));

	entry->m_interface->m_econet = this;
	entry->m_interface->m_address = address;

	m_device_list.append(*entry);
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

econet_device::daisy_entry::daisy_entry(device_t *device) :
	m_next(NULL),
	m_device(device),
	m_interface(NULL)
{
	for (int i = 0; i < SIGNAL_COUNT; i++)
	{
		m_line[i] = 1;
	}

	device->interface(m_interface);
}


//-------------------------------------------------
//  clk_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( econet_device::clk_w )
{
	set_signal(this, CLK, state);
}


//-------------------------------------------------
//  data_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( econet_device::data_w )
{
	set_signal(this, DATA, state);
}


//-------------------------------------------------
//  clk_w -
//-------------------------------------------------

void econet_device::clk_w(device_t *device, int state)
{
	set_signal(device, CLK, state);
}


//-------------------------------------------------
//  data_w -
//-------------------------------------------------

void econet_device::data_w(device_t *device, int state)
{
	set_signal(device, DATA, state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( econet_devices )
//-------------------------------------------------

// slot devices
#include "e01.h"

SLOT_INTERFACE_START( econet_devices )
	SLOT_INTERFACE("e01", E01)
	SLOT_INTERFACE("e01s", E01S)
SLOT_INTERFACE_END
