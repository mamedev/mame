// license:BSD-3-Clause
// copyright-holders: R. Belmont, Olivier Galibert

// ADB - Apple Desktop Bus
//
// The serial desktop device bus from before USB was cool.
//
// Single data wire + poweron line, open collector

#include "emu.h"
#include "adb.h"

#define LOG_LINE (1U << 1)
#define LOG_CONNECT (1U << 2)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(ADB_BUS, adb_bus_device, "adbbus", "Apple Desktop Bus")
DEFINE_DEVICE_TYPE(ADB_CONNECTOR, adb_connector, "adbslot", "ADB connector")

adb_bus_device::adb_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADB_BUS, tag, owner, clock),
	m_external_devices(*this, finder_base::DUMMY_TAG, 0),
	m_adb_handler(*this),
	m_poweron_handler(*this),
	m_devcnt(0),
	m_adb_line(ASSERT_LINE),
	m_poweron_line(ASSERT_LINE),
	m_anykeydown_state(CLEAR_LINE),
	m_host_drive_state(ASSERT_LINE),
	m_updating_adb(false),
	m_adb_update_pending(false)
{
	std::fill(std::begin(m_dev), std::end(m_dev), adb_dev_t{nullptr, ASSERT_LINE, ASSERT_LINE, CLEAR_LINE});
}

void adb_bus_device::device_start()
{
	save_item(NAME(m_adb_line));
	save_item(NAME(m_poweron_line));
	save_item(NAME(m_anykeydown_state));
	save_item(NAME(m_host_drive_state));

	for (unsigned i = 0; i < m_devcnt; i++)
	{
		save_item(NAME(m_dev[i].m_adb_state), i);
		save_item(NAME(m_dev[i].m_poweron_state), i);
		save_item(NAME(m_dev[i].m_anykeydown_state), i);
	}
}

void adb_bus_device::device_reset()
{
	m_updating_adb = false;
	m_adb_update_pending = false;
	m_host_drive_state = ASSERT_LINE;
	for (unsigned i = 0; i < m_devcnt; i++)
	{
		m_dev[i].m_adb_state = ASSERT_LINE;
		m_dev[i].m_poweron_state = ASSERT_LINE;
		m_dev[i].m_anykeydown_state = CLEAR_LINE;
	}

	// Force initial callbacks so the host and peripherals all see the pulled-up lines.
	m_adb_line = CLEAR_LINE;
	m_poweron_line = CLEAR_LINE;
	m_anykeydown_state = CLEAR_LINE;
	update_adb_line();
	update_poweron_line();
}

void adb_bus_device::update_adb_line()
{
	if (m_updating_adb)
	{
		m_adb_update_pending = true;
		return;
	}

	m_updating_adb = true;
	do
	{
		m_adb_update_pending = false;

		int new_state = m_host_drive_state;
		for (unsigned i = 0; i < m_devcnt; i++)
		{
			new_state &= m_dev[i].m_adb_state;
		}

		if (new_state != m_adb_line)
		{
			m_adb_line = new_state;
			LOGMASKED(LOG_LINE, "ADB line is now %d\n", m_adb_line);

			m_adb_handler(m_adb_line);
			for (unsigned i = 0; i < m_devcnt; i++)
			{
				m_dev[i].m_dev->adb_w(m_adb_line);
			}

			// The host and low-level devices often poll the line in firmware.  Keep
			// them interleaved through the longest in-progress bus interval.
			machine().scheduler().perfect_quantum(attotime::from_usec(500));
		}
	}
	while (m_adb_update_pending);
	m_updating_adb = false;
}

void adb_bus_device::update_poweron_line()
{
	int new_state = ASSERT_LINE;
	for (unsigned i = 0; i < m_devcnt; i++)
	{
		new_state &= m_dev[i].m_poweron_state;
	}

	if (new_state != m_poweron_line)
	{
		m_poweron_line = new_state;
		m_poweron_handler(m_poweron_line);
	}
}

void adb_bus_device::update_anykeydown()
{
	int new_state = CLEAR_LINE;
	for (unsigned i = 0; i < m_devcnt; i++)
	{
		new_state |= m_dev[i].m_anykeydown_state;
	}
	m_anykeydown_state = new_state;
}

void adb_bus_device::adb_host_line_w(int state)
{
	state = state ? ASSERT_LINE : CLEAR_LINE;
	if (state != m_host_drive_state)
	{
		m_host_drive_state = state;
		update_adb_line();
	}
}

void adb_bus_device::poll_devices()
{
	for (unsigned i = 0; i < m_devcnt; i++)
	{
		m_dev[i].m_dev->adb_poll_inputs();
	}
}

void adb_bus_device::adb_device_line_w(unsigned refid, int state)
{
	if (refid >= m_devcnt)
	{
		fatalerror("%s: ADB line write with invalid device reference %u\n", tag(), refid);
	}

	state = state ? ASSERT_LINE : CLEAR_LINE;
	if (state != m_dev[refid].m_adb_state)
	{
		m_dev[refid].m_adb_state = state;
		update_adb_line();
	}
}

void adb_bus_device::poweron_device_line_w(unsigned refid, int state)
{
	if (refid >= m_devcnt)
	{
		fatalerror("%s: power-on line write with invalid device reference %u\n", tag(), refid);
	}

	state = state ? ASSERT_LINE : CLEAR_LINE;
	if (state != m_dev[refid].m_poweron_state)
	{
		m_dev[refid].m_poweron_state = state;
		update_poweron_line();
	}
}

void adb_bus_device::anykeydown_device_w(unsigned refid, int state)
{
	if (refid >= m_devcnt)
	{
		fatalerror("%s: any-key-down write with invalid device reference %u\n", tag(), refid);
	}

	state = state ? ASSERT_LINE : CLEAR_LINE;
	if (state != m_dev[refid].m_anykeydown_state)
	{
		m_dev[refid].m_anykeydown_state = state;
		update_anykeydown();
	}
}

void adb_bus_device::device_resolve_objects()
{
	m_devcnt = 0;
	for (unsigned i = 0; i < MAX_DEVICES; i++)
	{
		adb_device_interface *sdev = nullptr;
		if (m_external_devices[i])
		{
			sdev = m_external_devices[i];
		}
		else
		{
			adb_connector *const connector = subdevice<adb_connector>(string_format("%u", i));
			sdev = connector ? connector->get_device() : nullptr;
		}
		if (sdev)
		{
			unsigned const rid = m_devcnt++;
			m_dev[rid].m_dev = sdev;
			sdev->connect_to_bus(*this, rid);
		}
	}
}

adb_connector::adb_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADB_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<adb_slot_card_interface>(mconfig, *this)
{
}

void adb_connector::device_start()
{
}

adb_device_interface *adb_connector::get_device()
{
	adb_slot_card_interface *const connected = get_card_device();
	if (connected)
	{
		return connected->device().subdevice<adb_device_interface>(connected->m_adb.finder_tag());
	}
	else
	{
		return nullptr;
	}
}

adb_slot_card_interface::adb_slot_card_interface(const machine_config &mconfig, device_t &device, const char *adb_tag) :
	device_interface(device, "adb"),
	m_adb(*this, adb_tag)
{
}

adb_device_interface::adb_device_interface(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_bus(nullptr),
	m_refid(0)
{
}

void adb_device_interface::device_start()
{
}

void adb_device_interface::device_reset()
{
}

void adb_device_interface::connect_to_bus(adb_bus_device &bus, unsigned refid)
{
	if (m_bus)
	{
		fatalerror("%s: attempted to connect to multiple ADB buses\n", tag());
	}

	LOGMASKED(LOG_CONNECT, "Connected to %s as device %u\n", bus.tag(), refid);
	m_bus = &bus;
	m_refid = refid;
}

int adb_device_interface::adb_line_r() const
{
	if (!m_bus)
	{
		fatalerror("%s: ADB line read before bus connection\n", tag());
	}

	return m_bus->adb_line_r();
}

int adb_device_interface::adb_poweron_r() const
{
	if (!m_bus)
	{
		fatalerror("%s: ADB power-on line read before bus connection\n", tag());
	}

	return m_bus->adb_poweron_r();
}

void adb_device_interface::adb_drive_w(int state)
{
	if (!m_bus)
	{
		fatalerror("%s: ADB line write before bus connection\n", tag());
	}

	m_bus->adb_device_line_w(m_refid, state);
}

void adb_device_interface::adb_poweron_w(int state)
{
	if (!m_bus)
	{
		fatalerror("%s: ADB power-on line write before bus connection\n", tag());
	}

	m_bus->poweron_device_line_w(m_refid, state);
}

void adb_device_interface::adb_anykeydown_w(int state)
{
	if (!m_bus)
	{
		fatalerror("%s: any-key-down write before bus connection\n", tag());
	}

	m_bus->anykeydown_device_w(m_refid, state);
}
