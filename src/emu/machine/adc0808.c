// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor ADC0808/ADC0809 8-Bit A/D Converter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "adc0808.h"




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type ADC0808 = &device_creator<adc0808_device>;

//-------------------------------------------------
//  adc0808_device - constructor
//-------------------------------------------------

adc0808_device::adc0808_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADC0808, "ADC0808", tag, owner, clock, "adc0808", __FILE__),
		m_address(0),
		m_start(0),
		m_eoc(0),
		m_next_eoc(0),
		m_cycle(0),
		m_bit(0)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void adc0808_device::device_config_complete()
{
	// inherit a copy of the static data
	const adc0808_interface *intf = reinterpret_cast<const adc0808_interface *>(static_config());
	if (intf != NULL)
		*static_cast<adc0808_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_eoc_cb, 0, sizeof(m_out_eoc_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc0808_device::device_start()
{
	// resolve callbacks
	m_out_eoc_func.resolve(m_out_eoc_cb, *this);

	// allocate timers
	m_cycle_timer = timer_alloc();
	m_cycle_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));

	// register for state saving
	save_item(NAME(m_address));
	save_item(NAME(m_start));
	save_item(NAME(m_eoc));
	save_item(NAME(m_next_eoc));
	save_item(NAME(m_sar));
	save_item(NAME(m_cycle));
	save_item(NAME(m_bit));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void adc0808_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (!m_start)
	{
		if (m_cycle == 7)
		{
			m_bit++;

			if (m_bit == 8)
			{
				/* sample input */
				double vref_pos = m_in_vref_pos_func(this);
				double vref_neg = m_in_vref_neg_func(this);

				double input = m_in_in_func[m_address](this);

				m_sar = (255 * (input - vref_neg)) / (vref_pos - vref_neg);

				/* trigger end of conversion */
				m_next_eoc = 1;
			}
		}
	}

	if (m_cycle == 0)
	{
		/* set end of conversion pin */
		if (m_next_eoc != m_eoc)
		{
			m_out_eoc_func(m_next_eoc);
			m_eoc = m_next_eoc;
		}
	}

	m_cycle++;

	if (m_cycle == 8)
	{
		m_cycle = 0;
	}
}


//-------------------------------------------------
//  data_r - data read
//-------------------------------------------------

READ8_MEMBER( adc0808_device::data_r )
{
	return m_sar;
}


//-------------------------------------------------
//  ale_w - address write
//-------------------------------------------------

WRITE8_MEMBER( adc0808_device::ale_w )
{
	m_address = data;
}


//-------------------------------------------------
//  start_w - start conversion
//-------------------------------------------------

WRITE_LINE_MEMBER( adc0808_device::start_w )
{
	if (!m_start && state) // rising edge
	{
		// reset registers

		m_sar = 0;
		m_bit = 0;
	}
	else if (m_start && !state) // falling edge
	{
		// start conversion

		m_next_eoc = 0;
	}

	m_start = state;
}
