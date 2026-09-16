// license:BSD-3-Clause
// copyright-holders: AJR
/**********************************************************************

    OKI MSM6253 8-Bit 4-Channel A/D Converter

**********************************************************************/

#include "emu.h"
#include "msm6253.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MSM6253, msm6253_device, "msm6253", "OKI MSM6253 A/D Converter")

//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  msm6253_device - constructor
//-------------------------------------------------

msm6253_device::msm6253_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSM6253, tag, owner, clock)
	, m_analog_ports(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
	, m_analog_input_cb(*this)
	, m_shift_register(0)
{
	m_analog_input_cb[0].set(*this, FUNC(msm6253_device::port_read<0>));
	m_analog_input_cb[1].set(*this, FUNC(msm6253_device::port_read<1>));
	m_analog_input_cb[2].set(*this, FUNC(msm6253_device::port_read<2>));
	m_analog_input_cb[3].set(*this, FUNC(msm6253_device::port_read<3>));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm6253_device::device_start()
{
	// resolve each callback
	m_analog_input_cb.resolve_all();

	for (int port = 0; port < 4; port++)
	{
		// ensure that any configured ports truly are analog
		if (m_analog_ports[port].found())
		{
			for (ioport_field &field : m_analog_ports[port]->fields())
				if (!field.is_analog() && field.type() != IPT_UNKNOWN && field.type() != IPT_UNUSED)
					throw emu_fatalerror("Port %s is not an analog port\n", m_analog_ports[port]->tag());
		}
	}

	// save our state
	save_item(NAME(m_shift_register));
}

//-------------------------------------------------
//  port_read - helper to read configured ports
//-------------------------------------------------

template<int port>
ioport_value msm6253_device::port_read()
{
	if (m_analog_ports[port].found())
		return m_analog_ports[port]->read();

	logerror("%s: Read from unassigned IN%d\n", machine().describe_context(), port);
	return 0xff;
}

//-------------------------------------------------
//  address_w - write from address bus to select
//  one of four internal latches
//-------------------------------------------------

void msm6253_device::address_w(offs_t offset, u8 data)
{
	// fill the shift register from the internal A/D latch
	m_shift_register = m_analog_input_cb[offset & 3]();
}

//-------------------------------------------------
//  select_w - write D0/D1 to address latch
//-------------------------------------------------

void msm6253_device::select_w(offs_t offset, u8 data)
{
	// fill the shift register from the internal A/D latch
	m_shift_register = m_analog_input_cb[data & 3]();
}

//-------------------------------------------------
//  shift_out - MSB-first serial data output
//-------------------------------------------------

bool msm6253_device::shift_out()
{
	// capture the shifted bit
	bool msb = BIT(m_shift_register, 7);

	// shift the bit out, with zero coming in on the other end
	if (!machine().side_effects_disabled())
		m_shift_register <<= 1;

	// return the bit
	return msb;
}

//-------------------------------------------------
//  d0_r - shift data bit out to D0
//-------------------------------------------------

u8 msm6253_device::d0_r(address_space &space)
{
	// offset is ignored
	return shift_out() | (space.unmap() & 0xfe);
}

//-------------------------------------------------
//  d7_r - shift data bit out to D7
//-------------------------------------------------

u8 msm6253_device::d7_r(address_space &space)
{
	// offset is ignored
	return (shift_out() << 7) | (space.unmap() & 0x7f);
}
