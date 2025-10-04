// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes
/******************************************************************************

    uPD7002 Analogue to Digital Converter

******************************************************************************/

#include "emu.h"
#include "upd7002.h"


DEFINE_DEVICE_TYPE(UPD7002, upd7002_device, "upd7002", "uPD7002 ADC")

upd7002_device::upd7002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UPD7002, tag, owner, clock)
	, m_status(0)
	, m_data1(0)
	, m_data0(0)
	, m_digitalvalue(0)
	, m_conversion_counter(0)
	, m_get_analogue_cb(*this, 0)
	, m_eoc_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7002_device::device_start()
{
	m_conversion_timer = timer_alloc(FUNC(upd7002_device::conversion_complete), this);

	// register for state saving
	save_item(NAME(m_status));
	save_item(NAME(m_data1));
	save_item(NAME(m_data0));
	save_item(NAME(m_digitalvalue));
	save_item(NAME(m_conversion_counter));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd7002_device::device_reset()
{
	m_status = 0;
	m_data1 = 0;
	m_data0 = 0;
	m_digitalvalue = 0;
	m_conversion_counter = 0;
}


/*****************************************************************************
 Implementation
*****************************************************************************/

int upd7002_device::eoc_r()
{
	return BIT(m_status, 7);
}


TIMER_CALLBACK_MEMBER(upd7002_device::conversion_complete)
{
	if (param == m_conversion_counter)
	{
		// this really always does a 12 bit conversion
		m_data1 = m_digitalvalue >> 8;
		m_data0 = m_digitalvalue & 0xf0;

		// set the status register with top 2 MSB, not busy and conversion complete
		m_status = (m_status & 0x0f) | ((m_data1 & 0xc0) >> 2) | 0x40;

		// call the EOC function with EOC from status
		// eoc_r(0) this has just been set to 0
		m_eoc_cb(0);
		m_conversion_counter = 0;
	}
}


uint8_t upd7002_device::read(offs_t offset)
{
	switch (offset & 0x03)
	{
	case 0:
		return m_status;

	case 1:
		return m_data1;

	case 2:
	case 3:
		return m_data0;
	}
	return 0;
}



void upd7002_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x03)
	{
	case 0:
		/*
		Data Latch/AD start
		    D0 and D1 together define which one of the four input channels is selected
		    D2 flag input, normally set to 0????
		    D3 defines whether an 8 (0) or 12 (1) bit resolution conversion should occur
		    D4 to D7 not used.

		    an 8 bit conversion typically takes 4ms
		    a 12 bit conversion typically takes 10ms

		    writing to this register will initiate a conversion.
		*/

		/* set D6=0 busy ,D7=1 conversion not complete */
		m_status = (data & 0x0f) | 0x80;

		// call the EOC function with EOC from status
		// eoc_r(0) this has just been set to 1
		m_eoc_cb(1);

		/* the uPD7002 works by sampling the analogue value at the start of the conversion
		   so it is read here and stored until the end of the A to D conversion */

		// this function should return a 16 bit value.
		m_digitalvalue = m_get_analogue_cb(m_status & 0x03);

		m_conversion_counter++;

		// call a timer to start the conversion
		if (m_status & 0x08)
		{
			// 12 bit conversion takes 10ms
			m_conversion_timer->adjust(attotime::from_msec(10), m_conversion_counter);
		}
		else
		{
			// 8 bit conversion takes 4ms
			m_conversion_timer->adjust(attotime::from_msec(4), m_conversion_counter);
		}
		break;

	case 1:
	case 2:
		/* Nothing */
		break;

	case 3:
		/* Test Mode: Used for inspecting the device, The data input-output terminals assume an input
		      state and are connected to the A/D counter. Therefore, the A/D conversion data
		      read out after this is meaningless.
		*/
		break;
	}
}
