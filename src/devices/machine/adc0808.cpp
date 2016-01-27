// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor ADC0808/ADC0809 8-Bit A/D Converter emulation

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
		m_out_eoc_cb(*this),
		m_address(0),
		m_start(0),
		m_eoc(0),
		m_next_eoc(0), m_sar(0),
		m_cycle(0),
		m_bit(0), m_cycle_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc0808_device::device_start()
{
	// resolve callbacks
	m_out_eoc_cb.resolve_safe();
	m_in_vref_pos_cb.bind_relative_to(*owner());
	m_in_vref_neg_cb.bind_relative_to(*owner());
	m_in_in_0_cb.bind_relative_to(*owner());
	m_in_in_1_cb.bind_relative_to(*owner());
	m_in_in_2_cb.bind_relative_to(*owner());
	m_in_in_3_cb.bind_relative_to(*owner());
	m_in_in_4_cb.bind_relative_to(*owner());
	m_in_in_5_cb.bind_relative_to(*owner());
	m_in_in_6_cb.bind_relative_to(*owner());
	m_in_in_7_cb.bind_relative_to(*owner());

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
				double vref_pos = m_in_vref_pos_cb();
				double vref_neg = m_in_vref_neg_cb();

				double input = 0;

				switch (m_address)
				{
					case 0:
						input = m_in_in_0_cb();
						break;
					case 1:
						input = m_in_in_1_cb();
						break;
					case 2:
						input = m_in_in_2_cb();
						break;
					case 3:
						input = m_in_in_3_cb();
						break;
					case 4:
						input = m_in_in_4_cb();
						break;
					case 5:
						input = m_in_in_5_cb();
						break;
					case 6:
						input = m_in_in_6_cb();
						break;
					case 7:
						input = m_in_in_7_cb();
						break;
				}
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
			m_out_eoc_cb(m_next_eoc);
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
