// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes
/******************************************************************************
    uPD7002 Analogue to Digital Converter

    MESS Driver By:

    Gordon Jefferyes
    mess_bbc@gjeffery.dircon.co.uk

******************************************************************************/

#include "emu.h"
#include "upd7002.h"


/* Device Interface */

const device_type UPD7002 = &device_creator<upd7002_device>;

upd7002_device::upd7002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD7002, "uPD7002", tag, owner, clock, "upd7002", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7002_device::device_start()
{
	m_get_analogue_cb.bind_relative_to(*owner());
	m_eoc_cb.bind_relative_to(*owner());

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


READ8_MEMBER( upd7002_device::eoc_r )
{
	return (m_status>>7)&0x01;
}


void upd7002_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CONVERSION_COMPLETE:
		{
		int counter_value = param;
		if (counter_value==m_conversion_counter)
		{
			// this really always does a 12 bit conversion
			m_data1 = m_digitalvalue>>8;
			m_data0 = m_digitalvalue&0xf0;

			// set the status register with top 2 MSB, not busy and conversion complete
			m_status = (m_status & 0x0f)|((m_data1 & 0xc0)>>2)|0x40;

			// call the EOC function with EOC from status
			// eoc_r(0) this has just been set to 0
			if (!m_eoc_cb.isnull()) m_eoc_cb(0);
			m_conversion_counter=0;
		}
		break;
		}
	default:
		assert_always(FALSE, "Unknown id in upd7002_device::device_timer");
	}
}


READ8_MEMBER( upd7002_device::read )
{
	switch(offset&0x03)
	{
		case 0:
			return m_status;

		case 1:
			return m_data1;

		case 2: case 3:
			return m_data0;
	}
	return 0;
}



WRITE8_MEMBER( upd7002_device::write )
{
	/* logerror("write to uPD7002 $%02X = $%02X\n",offset,data); */

	switch(offset&0x03)
	{
		case 0:
		/*
		Data Latch/AD start
		    D0 and D1 together define which one of the four input channels is selected
		    D2 flag input, normally set to 0????
		    D3 defines whether an 8 (0) or 12 (1) bit resolution conversion should occur
		    D4 to D7 not used.

		    an 8  bit conversion typically takes 4ms
		    an 12 bit conversion typically takes 10ms

		    writing to this register will initiate a conversion.
		*/

		/* set D6=0 busy ,D7=1 conversion not complete */
		m_status=(data & 0x0f) | 0x80;

		// call the EOC function with EOC from status
		// eoc_r(0) this has just been set to 1
		if (!m_eoc_cb.isnull()) m_eoc_cb(1);

		/* the uPD7002 works by sampling the analogue value at the start of the conversion
		   so it is read hear and stored until the end of the A to D conversion */

		// this function should return a 16 bit value.
		m_digitalvalue = m_get_analogue_cb(m_status & 0x03);

		m_conversion_counter++;

		// call a timer to start the conversion
		if (m_status & 0x08)
		{
			// 12 bit conversion takes 10ms
			timer_set(attotime::from_msec(10), TIMER_CONVERSION_COMPLETE, m_conversion_counter);
		} else {
			// 8 bit conversion takes 4ms
			timer_set(attotime::from_msec(4), TIMER_CONVERSION_COMPLETE, m_conversion_counter);
		}
		break;

		case 1: case 2:
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
