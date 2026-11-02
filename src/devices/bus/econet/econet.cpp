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


static const char *const SIGNAL_NAME[] = { "CLK", "DATA", "DRIVE" };



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


void device_econet_interface::econet_valid(int state)
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
	int const old_clk = get_clk();
	int const old_data = get_data();
	int const old_valid = get_valid();

	if (device == this)
	{
		if (m_line[signal] != state)
		{
			if (LOG) logerror("Econet: '%s' %s %u\n", tag(), SIGNAL_NAME[signal], state);
			m_line[signal] = state;
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
				}
			}
		}
	}

	int const clk = get_clk();
	if (clk != old_clk)
	{
		m_write_clk(clk);
		for (auto &entry : m_device_list)
			entry.interface().econet_clk(clk);
	}

	int const data = get_data();
	if (data != old_data)
	{
		m_write_data(data);
		for (auto &entry : m_device_list)
			entry.interface().econet_data(data);
	}

	int const valid = get_valid();
	if (valid != old_valid)
	{
		m_write_valid(valid);
		for (auto &entry : m_device_list)
			entry.interface().econet_valid(valid);
	}

	if (LOG) logerror("Econet: CLK %u DATA %u VALID %u\n", clk, data, valid);
}


//-------------------------------------------------
//  get_clk - wired-AND of the clock line
//-------------------------------------------------

inline int econet_device::get_clk()
{
	int state = m_line[CLK];

	for (auto &entry : m_device_list)
		state &= entry.m_line[CLK];

	return state;
}


//-------------------------------------------------
//  get_data - the terminators pull undriven data
//  lines to 1
//-------------------------------------------------

inline int econet_device::get_data()
{
	int state = m_line[DRIVE] ? m_line[DATA] : 1;

	for (auto &entry : m_device_list)
		if (entry.m_line[DRIVE])
			state &= entry.m_line[DATA];

	return state;
}


//-------------------------------------------------
//  get_valid - data lines are driven without a
//  collision
//-------------------------------------------------

inline int econet_device::get_valid()
{
	int drivers = 0;
	int ones = 0;

	if (m_line[DRIVE])
	{
		drivers++;
		ones += m_line[DATA];
	}

	for (auto &entry : m_device_list)
	{
		if (entry.m_line[DRIVE])
		{
			drivers++;
			ones += entry.m_line[DATA];
		}
	}

	return drivers && ((ones == 0) || (ones == drivers));
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
	m_write_data(*this),
	m_write_valid(*this)
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
	save_item(NAME(m_line));
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

	daisy_entry &entry = m_device_list.emplace_back(target);
	save_item(NAME(entry.m_line), address);
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
//  host_rts_w - data line driver enable
//-------------------------------------------------

void econet_device::host_rts_w(int state)
{
	set_signal(this, DRIVE, state);
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
//  rts_w - data line driver enable
//-------------------------------------------------

void econet_device::rts_w(device_t *device, int state)
{
	set_signal(device, DRIVE, state);
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
