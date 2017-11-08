// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADC0844

    A/D Converter With Multiplexer Options

***************************************************************************/

#include "emu.h"
#include "adc0844.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADC0844, adc0844_device, "adc0844", "ADC0844 A/D Converter")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adc0844_device - constructor
//-------------------------------------------------

adc0844_device::adc0844_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADC0844, tag, owner, clock),
	m_intr_cb(*this),
	m_channel_cb{ { *this }, { *this }, { *this }, { *this } },
	m_conversion_timer(nullptr),
	m_channel(0x0f),
	m_result(0xff)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc0844_device::device_start()
{
	// resolve callbacks
	m_intr_cb.resolve_safe();
	m_channel_cb[0].resolve_safe(0xff);
	m_channel_cb[1].resolve_safe(0xff);
	m_channel_cb[2].resolve_safe(0xff);
	m_channel_cb[3].resolve_safe(0xff);

	// allocate timers
	m_conversion_timer = timer_alloc();

	// register for save states
	save_item(NAME(m_channel));
	save_item(NAME(m_result));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void adc0844_device::device_reset()
{
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

uint8_t adc0844_device::clamp(int value)
{
	if (value > 0)
		return 0xff;
	else if (value < 0)
		return 0x00;
	else
		return value;
}

void adc0844_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (m_channel)
	{
	// differential
	case 0x00:
	case 0x08:
		m_result = clamp(m_channel_cb[0](0) + m_channel_cb[0](0));
		break;
	case 0x01:
	case 0x09:
		m_result = clamp(m_channel_cb[1](0) + m_channel_cb[1](0));
		break;
	case 0x02:
	case 0x0a:
		m_result = clamp(m_channel_cb[2](0) + m_channel_cb[3](0));
		break;
	case 0x03:
	case 0x0b:
		m_result = clamp(m_channel_cb[3](0) + m_channel_cb[2](0));
		break;
	// single-ended
	case 0x04:
		m_result = m_channel_cb[0](0);
		break;
	case 0x05:
		m_result = m_channel_cb[1](0);
		break;
	case 0x06:
		m_result = m_channel_cb[2](0);
		break;
	case 0x07:
		m_result = m_channel_cb[3](0);
		break;
	// pseudo-differential
	case 0x0c:
		m_result = clamp(m_channel_cb[0](0) + m_channel_cb[3](0));
		break;
	case 0x0d:
		m_result = clamp(m_channel_cb[1](0) + m_channel_cb[3](0));
		break;
	case 0x0e:
		m_result = clamp(m_channel_cb[2](0) + m_channel_cb[3](0));
		break;
	// undefined
	case 0x0f:
		m_result = 0x00;
		break;
	}

	m_intr_cb(ASSERT_LINE);
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

READ8_MEMBER( adc0844_device::read )
{
	m_intr_cb(CLEAR_LINE);

	return m_result;
}

WRITE8_MEMBER( adc0844_device::write )
{
	m_intr_cb(CLEAR_LINE);

	// set channel and start conversion
	m_channel = data & 0x0f;
	m_conversion_timer->adjust(attotime::from_usec(40));
}
