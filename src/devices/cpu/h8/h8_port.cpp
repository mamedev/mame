// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_port.cpp

    H8 8 bits digital port

    TODO:
    - Many I/O port pins have a double function, eg. for IRQs or timer input
      strobe. Ideally this device should know the input pin state, instead
      of having to implement it in the MAME drivers.

***************************************************************************/

#include "emu.h"
#include "h8_port.h"

DEFINE_DEVICE_TYPE(H8_PORT, h8_port_device, "h8_digital_port", "H8 digital port")

h8_port_device::h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, H8_PORT, tag, owner, clock),
	m_cpu(*this, DEVICE_SELF_OWNER), m_address(0), m_default_ddr(0), m_ddr(0), m_pcr(0), m_odr(0), m_mask(0), m_dr(0), m_last_output(0)
{
}

void h8_port_device::ddr_w(u8 data)
{
	//  logerror("ddr_w %02x\n", data);
	m_ddr = data;
	update_output();
}

u8 h8_port_device::ddr_r()
{
	return m_ddr;
}

void h8_port_device::dr_w(u8 data)
{
	//  logerror("dr_w %02x\n", data);
	m_dr = data;
	update_output();
}

u8 h8_port_device::dr_r()
{
	//  logerror("dr_r %02x\n", (dr | mask) & 0xff);
	return m_dr | m_mask;
}

u8 h8_port_device::port_r()
{
	u8 res = m_mask | (m_dr & m_ddr);
	if((m_ddr & ~m_mask) != u8(~m_mask))
		res |= m_cpu->do_read_port(m_address) & ~m_ddr;

	//  logerror("port_r %02x (%02x %02x)\n", res, ddr & ~mask, u8(~mask));
	return res;
}

void h8_port_device::pcr_w(u8 data)
{
	logerror("pcr_w %02x\n", data);
	m_pcr = data;
}

u8 h8_port_device::pcr_r()
{
	logerror("pcr_r %02x\n", (m_pcr | m_mask) & 0xff);
	return m_pcr | m_mask;
}

void h8_port_device::odr_w(u8 data)
{
	logerror("odr_w %02x\n", data);
	m_odr = data;
}

u8 h8_port_device::odr_r()
{
	logerror("odr_r %02x\n", (m_odr | ~m_mask) & 0xff);
	return m_odr | ~m_mask;
}

void h8_port_device::update_output()
{
	u8 data = m_dr & m_ddr & ~m_mask;
	u8 ddr = m_ddr & ~m_mask; // 0-bits = hi-z
	u16 res = ddr << 8 | data;

	if(res != m_last_output) {
		m_last_output = res;
		m_cpu->do_write_port(m_address, data, ddr);
	}
}

void h8_port_device::device_start()
{
	save_item(NAME(m_ddr));
	save_item(NAME(m_dr));
	save_item(NAME(m_pcr));
	save_item(NAME(m_odr));
	save_item(NAME(m_last_output));

	m_last_output = -1;
	m_dr = 0;
	m_ddr = m_default_ddr;
	m_pcr = 0;
	m_odr = 0;
}

void h8_port_device::device_reset()
{
	update_output();
}

bool h8_port_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	u8 buf[4];

	buf[0] = m_ddr;
	buf[1] = m_dr;
	buf[2] = m_pcr;
	buf[3] = m_odr;

	if(file.write(&buf, sizeof(buf), actual) || (sizeof(buf) != actual))
		return false;

	return true;
}

bool h8_port_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	u8 buf[4];

	if(file.read(&buf, sizeof(buf), actual) || (sizeof(buf) != actual))
		return false;

	m_ddr = buf[0];
	m_dr = buf[1];
	m_pcr = buf[2];
	m_odr = buf[3];

	return true;
}
