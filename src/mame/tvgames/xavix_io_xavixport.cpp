// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    XaviXPORT panel and sensor receiver interface. Latches a 40-bit report for
    host-clocked reads and provides the receiver-present signal.

    Accessories supply decoded report bytes, including identity and presence;
    console-panel inputs stay on the host. This boundary does not model the
    physical mat-to-receiver signalling.

***************************************************************************/

#include "emu.h"
#include "xavix_io_xavixport.h"

DEFINE_DEVICE_TYPE(XAVIXPORT_IO, xavixport_io_device, "xavixport_io", "XaviXPORT I/O Interface")

xavixport_io_device::xavixport_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIXPORT_IO, tag, owner, clock)
	, m_panel_cb(*this, 0)
	, m_accessory_cb(*this, 0)
	, m_receiver_present(false)
{
}

void xavixport_io_device::device_start()
{
	save_item(NAME(m_report));
	save_item(NAME(m_bit));
	save_item(NAME(m_selected));
	save_item(NAME(m_clock));
}

void xavixport_io_device::device_reset()
{
	m_report.fill(0);
	m_bit = 0;
	m_selected = false;
	m_clock = false;
}

void xavixport_io_device::latch_report()
{
	u16 const accessory = m_accessory_cb();
	m_report[0] = m_panel_cb() & 0xfc;
	m_report[1] = u8(accessory);
	m_report[2] = u8(accessory >> 8);
	m_report[3] = 0;
	m_report[4] = 0;
	m_bit = 0;
}

u8 xavixport_io_device::pio_r(offs_t, u8)
{
	u8 result = m_receiver_present ? 0x80 : 0x00;

	// The receiver presents the complemented report bit before each host-generated clock pulse.
	if (m_selected && !BIT(m_report[m_bit / 8], 7 - (m_bit % 8)))
		result |= 0x20;

	return result;
}

void xavixport_io_device::pio_w(offs_t, u8 data, u8 direction)
{
	u8 const output = data & direction;
	bool const selected = BIT(output, 4);
	bool const clock = BIT(output, 3);

	if (selected && !m_selected)
		latch_report();
	else if (selected && m_selected && m_clock && !clock)
		m_bit = (m_bit + 1) % 40;

	m_selected = selected;
	m_clock = clock;
}
