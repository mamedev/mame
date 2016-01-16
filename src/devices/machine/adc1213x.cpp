// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    National Semiconductor ADC12130 / ADC12132 / ADC12138

    Self-calibrating 12-bit Plus Sign Serial I/O A/D Converters with MUX
        and Sample/Hold

    TODO:
        - Only ADC12138 currently supported

    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "adc1213x.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define ADC1213X_CONV_MODE_12_MSB_FIRST         0
#define ADC1213X_CONV_MODE_16_MSB_FIRST         1
#define ADC1213X_CONV_MODE_12_LSB_FIRST         2
#define ADC1213X_CONV_MODE_16_LSB_FIRST         3

#define ADC1213X_ACQUISITION_TIME_6_CCLK        0
#define ADC1213X_ACQUISITION_TIME_10_CCLK       1
#define ADC1213X_ACQUISITION_TIME_18_CCLK       2
#define ADC1213X_ACQUISITION_TIME_34_CCLK       3



const device_type ADC12130 = &device_creator<adc12130_device>;

adc12130_device::adc12130_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: adc12138_device(mconfig, ADC12130, "ADC12130", tag, owner, clock, "adc12130", __FILE__)
{
}


const device_type ADC12132 = &device_creator<adc12132_device>;

adc12132_device::adc12132_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: adc12138_device(mconfig, ADC12132, "ADC12132", tag, owner, clock, "adc12132", __FILE__)
{
}


const device_type ADC12138 = &device_creator<adc12138_device>;

adc12138_device::adc12138_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADC12138, "ADC12138", tag, owner, clock, "adc12138", __FILE__)
{
}
adc12138_device::adc12138_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc12138_device::device_start()
{
	m_cycle = 0;
	m_data_out = 0;
	m_data_in = 0;
	m_auto_cal = 0;
	m_auto_zero = 0;
	m_input_shift_reg = 0;
	m_output_shift_reg = 0;
	m_end_conv = 0;

	/* resolve callbacks */
	m_ipt_read_cb.bind_relative_to(*owner());

	/* register for state saving */
	save_item(NAME(m_cycle));
	save_item(NAME(m_data_out));
	save_item(NAME(m_data_in));
	save_item(NAME(m_conv_mode));
	save_item(NAME(m_auto_cal));
	save_item(NAME(m_auto_zero));
	save_item(NAME(m_acq_time));
	save_item(NAME(m_data_out_sign));
	save_item(NAME(m_input_shift_reg));
	save_item(NAME(m_output_shift_reg));
	save_item(NAME(m_end_conv));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void adc12138_device::device_reset()
{
	m_conv_mode = ADC1213X_CONV_MODE_12_MSB_FIRST;
	m_data_out_sign = 1;
	m_auto_cal = 0;
	m_auto_zero = 0;
	m_acq_time = ADC1213X_ACQUISITION_TIME_10_CCLK;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    di_w
-------------------------------------------------*/

WRITE8_MEMBER( adc12138_device::di_w )
{
	m_data_in = data & 1;
}

/*-------------------------------------------------
    convert
-------------------------------------------------*/

void adc12138_device::convert(int channel, int bits16, int lsbfirst)
{
	int bits;
	int input_value;
	double input;

	if (bits16)
		fatalerror("ADC1213X: 16-bit mode not supported\n");

	if (lsbfirst)
		fatalerror("ADC1213X: LSB first not supported\n");

	switch (channel)
	{
		case 0x8:       // H L L L - CH0 (single-ended)
		{
			input = m_ipt_read_cb(0);
			break;
		}
		case 0xc:       // H H L L - CH1 (single-ended)
		{
			input = m_ipt_read_cb(1);
			break;
		}
		case 0x9:       // H L L H - CH2 (single-ended)
		{
			input = m_ipt_read_cb(2);
			break;
		}
		case 0xd:       // H H L H - CH3 (single-ended)
		{
			input = m_ipt_read_cb(3);
			break;
		}
		case 0xa:       // H L H L - CH4 (single-ended)
		{
			input = m_ipt_read_cb(4);
			break;
		}
		case 0xe:       // H H H L - CH5 (single-ended)
		{
			input = m_ipt_read_cb(5);
			break;
		}
		case 0xb:       // H L H H - CH6 (single-ended)
		{
			input = m_ipt_read_cb(6);
			break;
		}
		case 0xf:       // H H H H - CH7 (single-ended)
		{
			input = m_ipt_read_cb(7);
			break;
		}
		default:
		{
			fatalerror("ADC1213X: unsupported channel %02X\n", channel);
		}
	}

	input_value = (int)(input * 2047.0);

	bits = 12;

	// sign-extend if needed
	if (m_data_out_sign)
	{
		input_value = input_value | ((input_value & 0x800) << 1);
		bits++;
	}

	m_output_shift_reg = 0;

	for (int i = 0; i < bits; i++)
	{
		if (input_value & (1 << ((bits - 1) - i)))
		{
			m_output_shift_reg |= (1 << i);
		}
	}

	m_data_out = m_output_shift_reg & 1;
	m_output_shift_reg >>= 1;
}

/*-------------------------------------------------
    cs_w
-------------------------------------------------*/

WRITE8_MEMBER( adc12138_device::cs_w )
{
	if (data)
	{
		//printf("ADC: CS\n");

		if (m_cycle >= 7)
		{
			int mode = m_input_shift_reg >> (m_cycle - 8);

			switch (mode & 0xf)
			{
				case 0x0:       // X X X X L L L L - 12 or 13 Bit MSB First conversion
				{
					convert((mode >> 4) & 0xf, 0, 0);
					break;
				}
				case 0x1:       // X X X X L L L H - 16 or 17 Bit MSB First conversion
				{
					convert((mode >> 4) & 0xf, 1, 0);
					break;
				}
				case 0x4:       // X X X X L H L L - 12 or 13 Bit LSB First conversion
				{
					convert((mode >> 4) & 0xf, 0, 1);
					break;
				}
				case 0x5:       // X X X X L H L H - 16 or 17 Bit LSB First conversion
				{
					convert((mode >> 4) & 0xf, 1, 1);
					break;
				}

				default:
				{
					switch (mode)
					{
						case 0x08:      // L L L L H L L L - Auto cal
						{
							m_auto_cal = 1;
							break;
						}

						case 0x0e:      // L L L L H H H L - Acquisition time 6 CCLK cycles
						{
							m_acq_time = ADC1213X_ACQUISITION_TIME_6_CCLK;
							break;
						}

						case 0x8d:      // H L L L H H L H - Data out with sign
						{
							m_data_out_sign = 1;
							break;
						}

						case 0x0f:      // L L L L H H H H - User mode
						{
							break;
						}

						default:
						{
							fatalerror("ADC1213X: unknown config mode %02X\n", mode);
						}
					}
					break;
				}
			}
		}

		m_cycle = 0;
		m_input_shift_reg = 0;

		m_end_conv = 0;
	}
}

/*-------------------------------------------------
    sclk_w
-------------------------------------------------*/

WRITE8_MEMBER( adc12138_device::sclk_w )
{
	if (data)
	{
		//printf("ADC: cycle %d, DI = %d\n", adc1213x->cycle, adc1213x->data_in);

		m_input_shift_reg <<= 1;
		m_input_shift_reg |= m_data_in;

		m_data_out = m_output_shift_reg & 1;
		m_output_shift_reg >>= 1;

		m_cycle++;
	}
}

/*-------------------------------------------------
    conv_w
-------------------------------------------------*/

WRITE8_MEMBER( adc12138_device::conv_w )
{
	m_end_conv = 1;
}

/*-------------------------------------------------
    do_r
-------------------------------------------------*/

READ8_MEMBER( adc12138_device::do_r )
{
	//printf("ADC: DO\n");
	return m_data_out;
}

/*-------------------------------------------------
    eoc_r
-------------------------------------------------*/

READ8_MEMBER( adc12138_device::eoc_r )
{
	return m_end_conv;
}
