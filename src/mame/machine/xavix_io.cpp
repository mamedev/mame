// license:BSD-3-Clause
// copyright-holders:David Haywood

// TODO: hook this back up, and work out why taikodp eeprom breaks when it is hooked up.  epo_efdx breakage is expected because that requires an ugly hack to get to the title
//       note, eeproms don't work properly in most games at the moment, but no behavioral change was expected with taikodp

#include "emu.h"
#include "xavix_io.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIXIO, xavix_io_device, "xavixio", "XaviX IO")

xavix_io_device::xavix_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIXIO, tag, owner, clock)
	, m_in0_cb(*this)
	, m_in1_cb(*this)
	, m_out0_cb(*this)
	, m_out1_cb(*this)
{
}

void xavix_io_device::device_start()
{
	m_in0_cb.resolve_safe(0xff);
	m_in1_cb.resolve_safe(0xff);

	m_out0_cb.resolve_safe();
	m_out1_cb.resolve_safe();

	save_item(NAME(m_dir));
	save_item(NAME(m_dat));
}

void xavix_io_device::device_reset()
{
	for (int i = 0; i < 2; i++)
	{
		m_dir[i] = 0;
		m_dat[i] = 0;
	}
}

// direction: 1 is out, 0 is in

WRITE8_MEMBER(xavix_io_device::xav_7a0x_dir_w)
{
	LOG("%s: xavix IO xav_7a0x_dir_w (port %d) %02x\n", machine().describe_context(), offset, data);
	if (offset < 2)
	{
		m_dir[offset] = data;
		// write back to the port
		xav_7a0x_dat_w(space,offset,m_dat[offset]);
	}

}

WRITE8_MEMBER(xavix_io_device::xav_7a0x_dat_w)
{
	LOG("%s: xavix IO xav_7a0x_dat_w (port %d) %02x\n", machine().describe_context(), offset, data);
	if (offset < 2)
	{
		m_dat[offset] = data;

		uint8_t outdata = m_dat[offset] & m_dir[offset];

		switch (offset)
		{
		case 0x0: m_out0_cb(space, 0, outdata); break;
		case 0x1: m_out1_cb(space, 0, outdata); break;
		}
	}
}

READ8_MEMBER(xavix_io_device::xav_7a0x_dir_r)
{
	uint8_t ret = 0x00;
	LOG("%s: xavix IO xav_7a0x_dir_r (port %d)\n", machine().describe_context(), offset);
	if (offset < 2)
	{
		ret = m_dir[offset];
	}
	return ret;
}

READ8_MEMBER(xavix_io_device::xav_7a0x_dat_r)
{
	uint8_t ret = 0x00;
	LOG("%s: xavix IO xav_7a0x_dat_r (port %d)\n", machine().describe_context(), offset);
	if (offset < 2)
	{
		switch (offset)
		{
		case 0x0: ret = m_in0_cb(space, 0); break;
		case 0x1: ret = m_in1_cb(space, 0); break;
		}

		ret &= ~m_dir[offset];
		ret |= m_dat[offset] & m_dir[offset];
	}
	return ret;
}
