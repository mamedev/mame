// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX's two bidirectional 8-bit GPIO ports and four latched I/O-event
    inputs. Implements per-bit direction, mixed input/output readback and
    event enables, status and acknowledgements, with an IRQ summary output.
    The SuperXaviX implementation routes the legacy port registers through
    its PIO device, sharing the same event interrupt interface.

***************************************************************************/

#include "emu.h"
#include "xavix_io.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_IO, xavix_io_device, "xavix_io", "XaviX IO")
DEFINE_DEVICE_TYPE(SUPERXAVIX_IO, superxavix_io_device, "superxavix_io", "SuperXaviX IO")

xavix_io_device::xavix_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_io_device(mconfig, XAVIX_IO, tag, owner, clock)
{
}

xavix_io_device::xavix_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_input_cb(*this, 0xff)
	, m_output_cb(*this)
	, m_irq_cb(*this)
	, m_event_enable(0)
{
}

superxavix_io_device::superxavix_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_io_device(mconfig, SUPERXAVIX_IO, tag, owner, clock)
	, m_pio(*this, finder_base::DUMMY_TAG)
{
}

void xavix_io_device::map(address_map &map)
{
	map(0x0, 0x1).rw(FUNC(xavix_io_device::data_r), FUNC(xavix_io_device::data_w));
	map(0x2, 0x3).rw(FUNC(xavix_io_device::direction_r), FUNC(xavix_io_device::direction_w));
}

void xavix_io_device::event_map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(xavix_io_device::event_enable_r), FUNC(xavix_io_device::event_enable_w));
	map(0x1, 0x1).rw(FUNC(xavix_io_device::event_status_r), FUNC(xavix_io_device::event_ack_w));
}

void xavix_io_device::device_start()
{
	save_item(NAME(m_direction));
	save_item(NAME(m_data));
	save_item(NAME(m_event_enable));
	save_item(NAME(m_event_pending));
}

void xavix_io_device::device_reset()
{
	m_direction.fill(0);
	m_data.fill(0);
	m_event_enable = 0;
	m_event_pending = 0;
	update_irq();
}

u8 xavix_io_device::event_enable_r()
{
	if (!machine().side_effects_disabled())
		LOG("%s: event_enable_r\n", machine().describe_context());
	return m_event_enable;
}

void xavix_io_device::event_enable_w(u8 data)
{
	LOG("%s: event_enable_w %02x\n", machine().describe_context(), data);

	// Some serial receivers acknowledge an event by disabling its channel.
	m_event_pending &= data;
	m_event_enable = data;
	update_irq();
}

u8 xavix_io_device::event_status_r()
{
	if (!machine().side_effects_disabled())
		LOG("%s: event_status_r\n", machine().describe_context());
	return m_event_pending;
}

void xavix_io_device::event_ack_w(u8 data)
{
	LOG("%s: event_ack_w %02x\n", machine().describe_context(), data);
	m_event_pending &= ~(data & 0x0f);
	update_irq();
}

void xavix_io_device::update_irq()
{
	m_irq_cb((m_event_pending & 0x0f) ? ASSERT_LINE : CLEAR_LINE);
}

u8 xavix_io_device::external_input_r(offs_t port, u8 mem_mask)
{
	return m_input_cb[port](0, mem_mask);
}

void xavix_io_device::external_output_w(offs_t port, u8 data, u8 mem_mask)
{
	m_output_cb[port](0, data, mem_mask);
}

// direction: 1 is out, 0 is in

void xavix_io_device::direction_w(offs_t offset, u8 data)
{
	assert(offset < m_direction.size());
	LOG("%s: xavix IO direction_w (port %u) %02x\n", machine().describe_context(), unsigned(offset), data);
	m_direction[offset] = data;
	update_output(offset);
}

void xavix_io_device::data_w(offs_t offset, u8 data)
{
	assert(offset < m_data.size());
	LOG("%s: xavix IO data_w (port %u) %02x\n", machine().describe_context(), unsigned(offset), data);
	m_data[offset] = data;
	update_output(offset);
}

void xavix_io_device::update_output(unsigned port)
{
	u8 const input = m_input_cb[port](0, m_direction[port]);
	u8 const levels = (input & ~m_direction[port]) | (m_data[port] & m_direction[port]);
	m_output_cb[port](0, levels, m_direction[port]);
}

u8 xavix_io_device::direction_r(offs_t offset)
{
	assert(offset < m_direction.size());
	if (!machine().side_effects_disabled())
		LOG("%s: xavix IO direction_r (port %u)\n", machine().describe_context(), unsigned(offset));
	return m_direction[offset];
}

u8 xavix_io_device::data_r(offs_t offset)
{
	assert(offset < m_direction.size());
	if (!machine().side_effects_disabled())
		LOG("%s: xavix IO data_r (port %u)\n", machine().describe_context(), unsigned(offset));
	u8 const input = m_input_cb[offset](0, m_direction[offset]);
	return (input & ~m_direction[offset]) | (m_data[offset] & m_direction[offset]);
}

u8 xavix_io_device::output_mask(unsigned port) const
{
	return m_direction[port];
}

u8 xavix_io_device::output_latch(unsigned port)
{
	return m_data[port];
}

void superxavix_io_device::direction_w(offs_t offset, u8 data)
{
	m_pio->pio_direction_w(offset, data);
}

void superxavix_io_device::data_w(offs_t offset, u8 data)
{
	m_pio->pio_output_w(offset, data);
}

u8 superxavix_io_device::direction_r(offs_t offset)
{
	return m_pio->pio_direction_r(offset);
}

u8 superxavix_io_device::data_r(offs_t offset)
{
	return m_pio->pio_input_r(offset);
}

u8 superxavix_io_device::output_mask(unsigned port) const
{
	return m_pio->output_mask(port);
}

u8 superxavix_io_device::output_latch(unsigned port)
{
	return m_pio->pio_output_r(port);
}
