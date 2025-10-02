// license:BSD-3-Clause
// copyright-holders:David Haywood

// has separate input / output addresses but still uses direction registers, or I've misunderstood this entirely.

#include "emu.h"
#include "xavix2002_io.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX2002IO, xavix2002_io_device, "xavix2002io", "XaviX 2002 IO")

xavix2002_io_device::xavix2002_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIX2002IO, tag, owner, clock)
	, m_in0_cb(*this, 0xff)
	, m_in1_cb(*this, 0xff)
	, m_in2_cb(*this, 0xff)
	, m_out0_cb(*this)
	, m_out1_cb(*this)
	, m_out2_cb(*this)
{
}

void xavix2002_io_device::device_start()
{
	save_item(NAME(m_sx_pio_dir));
	save_item(NAME(m_sx_pio_out));
}

void xavix2002_io_device::device_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_sx_pio_dir[i] = 0;
		m_sx_pio_out[i] = 0;
	}
}

void xavix2002_io_device::pio_dir_w(offs_t offset, uint8_t data)
{
	LOG("%s: superxavix pio_dir_w (port %d) %02x\n", machine().describe_context(), offset, data);

	if (offset < 3)
	{
		m_sx_pio_dir[offset] = data;
		// update output port after direction change?
		pio_out_w(offset, m_sx_pio_out[offset]);
	}
}

uint8_t xavix2002_io_device::pio_dir_r(offs_t offset)
{
	LOG("%s: superxavix pio_dir_r (port %d)\n", machine().describe_context(), offset);
	uint8_t ret = 0x00;

	if (offset < 3)
	{
		ret = m_sx_pio_dir[offset];
	}

	return ret;
}

void xavix2002_io_device::pio_out_w(offs_t offset, uint8_t data)
{
	LOG("%s: superxavix pio_out_w (port %d) %02x\n", machine().describe_context(), offset, data);

	if (offset < 3)
	{
		m_sx_pio_out[offset] = data;

		switch (offset)
		{
			case 0: m_out0_cb(0, m_sx_pio_out[offset], m_sx_pio_dir[offset]); break;
			case 1: m_out1_cb(0, m_sx_pio_out[offset], m_sx_pio_dir[offset]); break;
			case 2: m_out2_cb(0, m_sx_pio_out[offset], m_sx_pio_dir[offset]); break;
			default: break;
		}
	}
}

uint8_t xavix2002_io_device::pio_out_r(offs_t offset)
{
	// what does this actually read?

	LOG("%s: superxavix pio_out_r (port %d)\n", machine().describe_context(), offset);

	uint8_t ret = 0x00;

	if (offset<3)
		ret = m_sx_pio_out[offset];

	return ret;
}


uint8_t xavix2002_io_device::pio_in_r(offs_t offset)
{
	LOG("%s: superxavix pio_in_r (port %d)\n", machine().describe_context(), offset);

	uint8_t ret = 0x00;

	switch (offset)
	{
		case 0: ret = m_in0_cb(0, m_sx_pio_dir[offset]); break;
		case 1: ret = m_in1_cb(0, m_sx_pio_dir[offset]); break;
		case 2: ret = m_in2_cb(0, m_sx_pio_dir[offset]); break;
		default: ret = 0x00; break;
	}

	ret &= ~m_sx_pio_dir[offset];
	ret |= m_sx_pio_out[offset] & m_sx_pio_dir[offset];

	return ret;
}
