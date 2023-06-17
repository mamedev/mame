// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn Computers Econet local area network emulation

**********************************************************************/

#include "emu.h"
#include "econet.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


static const char *const SIGNAL_NAME[] = { "CLK", "DATA" };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ECONET,      econet_device,      "econet",      "Econet")
DEFINE_DEVICE_TYPE(ECONET_SLOT, econet_slot_device, "econet_slot", "Econet station")



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_econet_interface - constructor
//-------------------------------------------------

device_econet_interface::device_econet_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "econet"), m_econet(nullptr), m_address(0)
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  econet_slot_device - constructor
//-------------------------------------------------

econet_slot_device::econet_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ECONET_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_econet_interface>(mconfig, *this),
	m_address(0),
	m_econet(*this, finder_base::DUMMY_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void econet_slot_device::device_start()
{
	device_econet_interface *dev = get_card_device();
	if (dev)
		m_econet->add_device(*dev, m_address);
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
		for (auto &entry : m_device_list)
		{
			if (!strcmp(entry.device().tag(), device->tag()))
			{
				if (entry.m_line[signal] != state)
				{
					if (LOG) logerror("Econet: '%s' %s %u\n", device->tag(), SIGNAL_NAME[signal], state);
					entry.m_line[signal] = state;
					changed = true;
				}
			}
		}
	}

	if (changed)
	{
		switch (signal)
		{
		case CLK:   m_write_clk(state);  break;
		case DATA:  m_write_data(state); break;
		}

		for (auto &entry : m_device_list)
		{
			switch (signal)
			{
			case CLK:
				entry.interface().econet_clk(state);
				break;

			case DATA:
				entry.interface().econet_data(state);
				break;
			}
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
		for (auto &entry : m_device_list)
		{
			if (!entry.m_line[signal])
			{
				state = 0;
				break;
			}
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

econet_device::econet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ECONET, tag, owner, clock),
	m_write_clk(*this),
	m_write_data(*this)
{
	for (auto & elem : m_line)
	{
		elem = 1;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void econet_device::device_start()
{
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void econet_device::device_stop()
{
	m_device_list.clear();
}


//-------------------------------------------------
//  add_device -
//-------------------------------------------------

void econet_device::add_device(device_econet_interface &target, int address)
{
	target.m_econet = this;
	target.m_address = address;

	m_device_list.emplace_back(target);
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

econet_device::daisy_entry::daisy_entry(device_econet_interface &device) :
	m_device(&device.device()),
	m_interface(&device)
{
	for (auto &elem : m_line)
		elem = 1;
}


//-------------------------------------------------
//  clk_w -
//-------------------------------------------------

void econet_device::host_clk_w(int state)
{
	set_signal(this, CLK, state);
}


//-------------------------------------------------
//  data_w -
//-------------------------------------------------

void econet_device::host_data_w(int state)
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

void econet_devices(device_slot_interface &device)
{
	device.option_add("e01",  ECONET_E01);
	device.option_add("e01s", ECONET_E01S);
}
