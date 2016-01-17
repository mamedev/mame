// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    National Semiconductor ADC1038

    10-Bit Serial I/O A/D Converters with Analog Multiplexer and
    Track/hold Function

***************************************************************************/

#include "emu.h"
#include "adc1038.h"


const device_type ADC1038 = &device_creator<adc1038_device>;

adc1038_device::adc1038_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADC1038, "ADC1038", tag, owner, clock, "adc1038", __FILE__), m_cycle(0), m_clk(0), m_adr(0), m_data_in(0), m_data_out(0), m_adc_data(0), m_sars(0),
		m_gticlub_hack(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc1038_device::device_start()
{
	m_input_cb.bind_relative_to(*owner());

	save_item(NAME(m_cycle));
	save_item(NAME(m_clk));
	save_item(NAME(m_adr));
	save_item(NAME(m_data_in));
	save_item(NAME(m_data_out));
	save_item(NAME(m_adc_data));
	save_item(NAME(m_sars));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void adc1038_device::device_reset()
{
	m_cycle = 0;
	m_clk = 0;
	m_adr = 0;
	m_data_in = 0;
	m_data_out = 0;
	m_adc_data = 0;
	m_sars = 1;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ_LINE_MEMBER( adc1038_device::do_read )
{
	m_data_out = (m_adc_data & 0x200) ? 1 : 0;
	m_adc_data <<= 1;

	//printf("ADC DO\n");
	return m_data_out;
}

WRITE_LINE_MEMBER( adc1038_device::di_write )
{
	m_data_in = state;
}

WRITE_LINE_MEMBER( adc1038_device::clk_write )
{
	// GTI Club doesn't sync on SARS
	if (m_gticlub_hack)
	{
		if (m_clk == 0 && state == 0)
		{
			m_cycle = 0;

			/* notice that m_adr is always < 7! */
			m_adc_data = m_input_cb(m_adr);
		}
	}

	if (state == 1)
	{
		//printf("ADC CLK, DI = %d, cycle = %d\n", m_data_in, m_cycle);

		if (m_cycle == 0)            // A2
		{
			m_adr = 0;
			m_adr |= (m_data_in << 2);
		}
		else if (m_cycle == 1)   // A1
		{
			m_adr |= (m_data_in << 1);
		}
		else if (m_cycle == 2)   // A0
		{
			m_adr |= (m_data_in << 0);
		}

		m_cycle++;
	}

	m_clk = state;
}

READ_LINE_MEMBER( adc1038_device::sars_read )
{
	m_cycle = 0;

	/* notice that m_adr is always < 7! */
	m_adc_data = m_input_cb(m_adr);

	m_sars ^= 1;
	return m_sars;
}
