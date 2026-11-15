// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    SuperXaviX's three general-purpose I/O ports. Implements per-bit direction,
    output latches and pull controls, with input masks selecting external pin
    connections. The first two ports also serve the legacy GPIO register
    layout through the shared I/O interface.

***************************************************************************/

#include "emu.h"
#include "xavix_pio.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SUPERXAVIX_PIO, superxavix_pio_device, "superxavix_pio", "SuperXaviX PIO")

superxavix_pio_device::superxavix_pio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SUPERXAVIX_PIO, tag, owner, clock)
	, m_input_cb(*this, 0xff)
	, m_input_mask_cb(*this, 0xff)
	, m_output_cb(*this)
	, m_legacy_input_cb(*this, 0xff)
	, m_legacy_input_mask_cb(*this, 0)
	, m_legacy_output_cb(*this)
	, m_legacy_output_mask{ 0xff, 0xff }
{
}

void superxavix_pio_device::map(address_map &map)
{
	map(0x00, 0x02).rw(FUNC(superxavix_pio_device::pio_direction_r), FUNC(superxavix_pio_device::pio_direction_w));
	map(0x10, 0x12).rw(FUNC(superxavix_pio_device::pio_output_r), FUNC(superxavix_pio_device::pio_output_w));
	map(0x20, 0x22).r(FUNC(superxavix_pio_device::pio_input_r));
	map(0x30, 0x32).rw(FUNC(superxavix_pio_device::pio_pull_r), FUNC(superxavix_pio_device::pio_pull_w));
}

void superxavix_pio_device::device_start()
{
	save_item(NAME(m_pio_direction));
	save_item(NAME(m_pio_output));
	save_item(NAME(m_pio_pull));
}

void superxavix_pio_device::device_reset()
{
	m_pio_direction.fill(0);
	m_pio_output.fill(0);
	m_pio_pull.fill(0xff);
}

u8 superxavix_pio_device::pio_direction_r(offs_t offset)
{
	assert(offset < m_pio_direction.size());
	if (!machine().side_effects_disabled())
		LOG("%s: pio_direction_r (port %u)\n", machine().describe_context(), unsigned(offset));
	return m_pio_direction[offset];
}

void superxavix_pio_device::pio_direction_w(offs_t offset, u8 data)
{
	assert(offset < m_pio_direction.size());
	LOG("%s: pio_direction_w (port %u) %02x\n", machine().describe_context(), unsigned(offset), data);
	if (offset < m_pio_direction.size())
	{
		m_pio_direction[offset] = data;
		update_output(offset);
	}
}

u8 superxavix_pio_device::pio_output_r(offs_t offset)
{
	assert(offset < m_pio_output.size());
	if (!machine().side_effects_disabled())
		LOG("%s: pio_output_r (port %u)\n", machine().describe_context(), unsigned(offset));
	// Pin levels are read through pio_input_r; this register reads back the output latch.
	return m_pio_output[offset];
}

void superxavix_pio_device::pio_output_w(offs_t offset, u8 data)
{
	assert(offset < m_pio_output.size());
	LOG("%s: pio_output_w (port %u) %02x\n", machine().describe_context(), unsigned(offset), data);
	if (offset < m_pio_output.size())
	{
		m_pio_output[offset] = data;
		update_output(offset);
	}
}

u8 superxavix_pio_device::pio_input_r(offs_t offset)
{
	assert(offset < m_pio_direction.size());
	if (!machine().side_effects_disabled())
		LOG("%s: pio_input_r (port %u)\n", machine().describe_context(), unsigned(offset));
	return input_r(offset);
}

u8 superxavix_pio_device::input_r(unsigned port)
{
	u8 const drive = output_mask(port);
	u8 const input_mask = m_input_mask_cb[port]();
	u8 input = m_input_cb[port](0, drive) & input_mask;
	u8 connected = input_mask;
	if (port < 2)
	{
		u8 const legacy_mask = m_legacy_input_mask_cb(port) & ~input_mask;
		input |= m_legacy_input_cb(port, drive) & legacy_mask;
		connected |= legacy_mask;
	}
	// A released pin follows its pull-up, pull-down, or the floating high level.
	u8 const released = ~m_pio_pull[port];
	input |= released & ~connected;
	return (input & ~drive) | (m_pio_output[port] & drive);
}

void superxavix_pio_device::update_output(unsigned port)
{
	u8 const drive = output_mask(port);
	u8 const legacy_mask = port < 2 ? m_legacy_output_mask[port] : 0;
	if (legacy_mask)
		m_legacy_output_cb(port, input_r(port), drive & legacy_mask);
	if (legacy_mask != 0xff)
		m_output_cb[port](0, m_pio_output[port], drive & ~legacy_mask);
}

u8 superxavix_pio_device::pio_pull_r(offs_t offset)
{
	assert(offset < m_pio_pull.size());
	if (!machine().side_effects_disabled())
		LOG("%s: pio_pull_r (port %u)\n", machine().describe_context(), unsigned(offset));
	return m_pio_pull[offset];
}

void superxavix_pio_device::pio_pull_w(offs_t offset, u8 data)
{
	assert(offset < m_pio_pull.size());
	LOG("%s: pio_pull_w (port %u) %02x\n", machine().describe_context(), unsigned(offset), data);
	if (offset < m_pio_pull.size())
	{
		m_pio_pull[offset] = data;
		update_output(offset);
	}
}
