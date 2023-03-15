// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADC0844/ADC0848

    A/D Converter With Multiplexer Options

***************************************************************************/

#include "emu.h"
#include "adc0844.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADC0844, adc0844_device, "adc0844", "ADC0844 A/D Converter")
DEFINE_DEVICE_TYPE(ADC0848, adc0848_device, "adc0848", "ADC0848 A/D Converter")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adc0844_device - constructor
//-------------------------------------------------

adc0844_device::adc0844_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_intr_cb(*this),
	m_ch1_cb(*this), m_ch2_cb(*this), m_ch3_cb(*this), m_ch4_cb(*this),
	m_conversion_timer(nullptr),
	m_channel(0x0f),
	m_result(0xff)
{
}

adc0844_device::adc0844_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	adc0844_device(mconfig, ADC0844, tag, owner, clock)
{
}

//-------------------------------------------------
//  adc0848_device - constructor
//-------------------------------------------------

adc0848_device::adc0848_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	adc0844_device(mconfig, ADC0848, tag, owner, clock),
	m_ch5_cb(*this), m_ch6_cb(*this), m_ch7_cb(*this), m_ch8_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc0844_device::device_start()
{
	// resolve callbacks
	m_intr_cb.resolve_safe();
	m_ch1_cb.resolve_safe(0xff);
	m_ch2_cb.resolve_safe(0xff);
	m_ch3_cb.resolve_safe(0xff);
	m_ch4_cb.resolve_safe(0xff);

	// allocate timers
	m_conversion_timer = timer_alloc(FUNC(adc0844_device::conversion_complete), this);

	// register for save states
	save_item(NAME(m_channel));
	save_item(NAME(m_result));
}

void adc0848_device::device_start()
{
	adc0844_device::device_start();

	// resolve callbacks
	m_ch5_cb.resolve_safe(0xff);
	m_ch6_cb.resolve_safe(0xff);
	m_ch7_cb.resolve_safe(0xff);
	m_ch8_cb.resolve_safe(0xff);
}

//-------------------------------------------------
//  clamp - restrict value to 0..255
//-------------------------------------------------

uint8_t adc0844_device::clamp(int value)
{
	if (value > 0xff)
		return 0xff;
	else if (value < 0)
		return 0x00;
	else
		return value;
}

//-------------------------------------------------
//  conversion_complete - finish ADC conversion
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(adc0844_device::conversion_complete)
{
	switch (m_channel)
	{
	// differential
	case 0x00:
	case 0x08: m_result = clamp(0xff - (m_ch2_cb(0) - m_ch1_cb(0))); break;
	case 0x01:
	case 0x09: m_result = clamp(0xff - (m_ch1_cb(0) - m_ch2_cb(0))); break;
	case 0x02:
	case 0x0a: m_result = clamp(0xff - (m_ch4_cb(0) - m_ch3_cb(0))); break;
	case 0x03:
	case 0x0b: m_result = clamp(0xff - (m_ch3_cb(0) - m_ch4_cb(0))); break;
	// single-ended
	case 0x04: m_result = m_ch1_cb(0); break;
	case 0x05: m_result = m_ch2_cb(0); break;
	case 0x06: m_result = m_ch3_cb(0); break;
	case 0x07: m_result = m_ch4_cb(0); break;
	// pseudo-differential
	case 0x0c: m_result = clamp(0xff - (m_ch4_cb(0) - m_ch1_cb(0))); break;
	case 0x0d: m_result = clamp(0xff - (m_ch4_cb(0) - m_ch2_cb(0))); break;
	case 0x0e: m_result = clamp(0xff - (m_ch4_cb(0) - m_ch3_cb(0))); break;
	// undefined
	case 0x0f: m_result = 0x00; break;
	}

	m_intr_cb(ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(adc0848_device::conversion_complete)
{
	switch (m_channel)
	{
	// differential
	case 0x00:
	case 0x10: m_result = clamp(0xff - (m_ch2_cb(0) - m_ch1_cb(0))); break;
	case 0x01:
	case 0x11: m_result = clamp(0xff - (m_ch1_cb(0) - m_ch2_cb(0))); break;
	case 0x02:
	case 0x12: m_result = clamp(0xff - (m_ch4_cb(0) - m_ch3_cb(0))); break;
	case 0x03:
	case 0x13: m_result = clamp(0xff - (m_ch3_cb(0) - m_ch4_cb(0))); break;
	case 0x04:
	case 0x14: m_result = clamp(0xff - (m_ch6_cb(0) - m_ch5_cb(0))); break;
	case 0x05:
	case 0x15: m_result = clamp(0xff - (m_ch5_cb(0) - m_ch6_cb(0))); break;
	case 0x06:
	case 0x16: m_result = clamp(0xff - (m_ch8_cb(0) - m_ch7_cb(0))); break;
	case 0x07:
	case 0x17: m_result = clamp(0xff - (m_ch7_cb(0) - m_ch8_cb(0))); break;
	// single-ended
	case 0x08: m_result = m_ch1_cb(0); break;
	case 0x09: m_result = m_ch2_cb(0); break;
	case 0x0a: m_result = m_ch3_cb(0); break;
	case 0x0b: m_result = m_ch4_cb(0); break;
	case 0x0c: m_result = m_ch5_cb(0); break;
	case 0x0d: m_result = m_ch6_cb(0); break;
	case 0x0e: m_result = m_ch7_cb(0); break;
	case 0x0f: m_result = m_ch8_cb(0); break;
	// pseudo-differential
	case 0x18: m_result = clamp(0xff - (m_ch8_cb(0) - m_ch1_cb(0))); break;
	case 0x19: m_result = clamp(0xff - (m_ch8_cb(0) - m_ch2_cb(0))); break;
	case 0x1a: m_result = clamp(0xff - (m_ch8_cb(0) - m_ch3_cb(0))); break;
	case 0x1b: m_result = clamp(0xff - (m_ch8_cb(0) - m_ch4_cb(0))); break;
	case 0x1c: m_result = clamp(0xff - (m_ch8_cb(0) - m_ch5_cb(0))); break;
	case 0x1d: m_result = clamp(0xff - (m_ch8_cb(0) - m_ch6_cb(0))); break;
	case 0x1e: m_result = clamp(0xff - (m_ch8_cb(0) - m_ch7_cb(0))); break;
	// undefined
	case 0x1f: m_result = 0x00; break;
	}

	m_intr_cb(ASSERT_LINE);
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

u8 adc0844_device::read()
{
	m_intr_cb(CLEAR_LINE);

	return m_result;
}

void adc0844_device::write(u8 data)
{
	m_intr_cb(CLEAR_LINE);

	// set channel and start conversion
	m_channel = data & 0x0f;
	m_conversion_timer->adjust(attotime::from_usec(40));
}

void adc0848_device::write(u8 data)
{
	m_intr_cb(CLEAR_LINE);

	// set channel and start conversion
	m_channel = data & 0x1f;
	m_conversion_timer->adjust(attotime::from_usec(40));
}
