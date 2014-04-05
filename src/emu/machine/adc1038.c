/***************************************************************************

    National Semiconductor ADC1038

    10-Bit Serial I/O A/D Converters with Analog Multiplexer and
    Track/hold Function

***************************************************************************/

#include "emu.h"
#include "adc1038.h"


const device_type ADC1038 = &device_creator<adc1038_device>;

adc1038_device::adc1038_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADC1038, "ADC1038", tag, owner, clock, "adc1038", __FILE__)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void adc1038_device::device_config_complete()
{
	// inherit a copy of the static data
	const adc1038_interface *intf = reinterpret_cast<const adc1038_interface *>(static_config());
	if (intf != NULL)
		*static_cast<adc1038_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		input_callback_r = NULL;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc1038_device::device_start()
{
	m_input_callback_r_func = input_callback_r;

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
			m_adc_data = m_input_callback_r_func(this, m_adr);
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
	m_adc_data = m_input_callback_r_func(this, m_adr);

	m_sars ^= 1;
	return m_sars;
}
