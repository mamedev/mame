// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC3/PS34

    MC and HC power supplies are based on a full custom liner ASIC known as ASIC3.
    This custom chip is manufactured by Maxim and has the Maxim part number MAX616.

    TODO:
    - Battery status readings, maybe different analogue channel.

******************************************************************************/

#include "emu.h"
#include "psion_asic3.h"

#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_PSU_ASIC3, psion_psu_asic3_device, "psion_psu_asic3", "Psion PSU (ASIC3)")
DEFINE_DEVICE_TYPE(PSION_PSU_ASIC5, psion_psu_asic5_device, "psion_psu_asic5", "Psion PSU (ASIC5)")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_asic3_device::psion_asic3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_adin_cb(*this, 0)
{
}

psion_psu_asic5_device::psion_psu_asic5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psion_asic3_device(mconfig, PSION_PSU_ASIC5, tag, owner, clock)
{
}

psion_psu_asic3_device::psion_psu_asic3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psion_asic3_device(mconfig, PSION_PSU_ASIC3, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_asic3_device::device_start()
{
	save_item(NAME(m_a3_control1));
	save_item(NAME(m_a3_control2));
	save_item(NAME(m_a3_control3));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic3_device::device_reset()
{
	m_a3_control1 = 0x00;
	m_a3_control2 = 0x00;
	m_a3_control3 = 0x00;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void psion_psu_asic5_device::data_w(uint16_t data)
{
	switch (data & 0x300)
	{
	case NULL_FRAME:
		device_reset();
		break;

	case CONTROL_FRAME:
		m_sibo_control = data & 0xff;
		break;

	case DATA_FRAME:
		data &= 0xff;
		switch (m_sibo_control & 0x0f)
		{
		case 0x01: // A3Control1
			// b0-b4 DtoaBits
			// b5    Vcc3Enable LCD
			// b6    Vcc4Enable Sound subsystem
			// b7    Vcc5Enable SSDs
			LOG("%s data_w: A3Control1 register %02x\n", machine().describe_context(), data);
			m_a3_control1 = data;
			break;

		case 0x02: // A3Setup
			LOG("%s data_w: A3Setup register %02x\n", machine().describe_context(), data);
			break;

		case 0x03: // A3Control2
			// b0    Vee1Enable LCD
			// b1    Vee2Enable Sound subsystem
			// b2    VhEnable
			// b3    Vee1SoftStart
			// b4    Vee2SoftStart
			// b5    VhSoftStart
			// b6-b7 AnalogueMultiplex
			LOG("%s data_w: A3Control2 register %02x\n", machine().describe_context(), data);
			m_a3_control2 = data;
			break;

		case 0x07: // A3Control3
			// b0    0
			// b1    OffEnable
			// b2    AdcReadHighEnable
			// b3-b7 0
			LOG("%s data_w: A3Control3 register %02x\n", machine().describe_context(), data);
			m_a3_control3 = data;
			break;

		default:
			LOG("%s data_w: unknown control %02x data %02x\n", machine().describe_context(), m_sibo_control, data);
			break;
		}
	}
}

uint8_t psion_psu_asic5_device::data_r()
{
	uint8_t data = 0x00;

	switch (m_sibo_control & 0xc0)
	{
	case 0x40: // SerialSelect
		if (m_sibo_control == 0x43) // A3SelectId
			data = 0x80; // A3InfoByte
		break;

	case 0xc0: // SerialRead
		switch (m_sibo_control & 0x0f)
		{
		case 0x00: // A3Adc
		{
			// A3AdcLsbR
			//   b0-b6 OtherBits
			//   b7    InvertedBit
			// A3AdcMsbR
			//   b0-b3 AdcBits
			//   b4    Overrange
			//   b5    Polarity
			uint16_t data_in = 0;
			switch (BIT(m_a3_control2, 6, 2)) // AnalogueMultiplex
			{
			case 0: // AdcDigitizer
				data_in = m_adin_cb();
				break;
			case 1: // AdcVh
				break;
			case 2: // AdcMainBattery
				data_in = 0x7ff;
				break;
			case 3: // AdcLithiumBattery
				data_in = 0x7ff;
				break;
			}
			if (BIT(m_a3_control3, 2)) // AdcReadHighEnable
				data = BIT(data_in, 8, 4);
			else
				data = (data_in & 0xff) ^ 0x80;
			LOG("%s data_r: A3Adc %d register %02x\n", machine().describe_context(), BIT(m_a3_control2, 6, 2), data);
			break;
		}
		case 0x0d: // A3Status
			// b0    ColdStart
			// b1    PowerFail
			data = 0x02; // PowerFail (required to initialise MC400/200/Word)
			LOG("%s data_r: A3Status register %02x\n", machine().describe_context(), data);
			break;

		default:
			LOG("%s data_r: unknown control %02x data %02x\n", machine().describe_context(), m_sibo_control, data);
			break;
		}
		break;
	}

	return data;
}


void psion_psu_asic3_device::data_w(uint16_t data)
{
	switch (data & 0x300)
	{
	case NULL_FRAME:
		device_reset();
		break;

	case CONTROL_FRAME:
		m_sibo_control = data & 0xff;
		break;

	case DATA_FRAME:
		data &= 0xff;
		switch (m_sibo_control & 0x0f)
		{
		case 0x00: // PS34W_CONTROL
			// b0-b1 Vhpower
			// b2    Vh
			// b3    Vcc5
			// b4    Vee2
			// b5    Vcc4
			// b6    Vee1
			// b7    Vcc3
			LOG("%s data_w: PS34W_CONTROL %02x\n", machine().describe_context(), data);
			m_a3_control1 = data;
			break;

		case 0x01: // PS34W_DTOA
			// b0-b4 Dac
			// b5-b6 Adcsel
			// b7    Ncc
			LOG("%s data_w: PS34W_DTOA %02x\n", machine().describe_context(), data);
			m_a3_control2 = data;
			break;

		default:
			LOG("%s data_w: unknown control %02x data %02x\n", machine().describe_context(), m_sibo_control, data);
			break;
		}
	}
}

uint8_t psion_psu_asic3_device::data_r()
{
	uint8_t data = 0x00;

	switch (m_sibo_control & 0xc0)
	{
	case 0x40: // SerialSelect
		if (m_sibo_control == 0x43) // A3SelectId
			data = 0x80; // A3InfoByte
		break;

	case 0xc0: // SerialRead
		switch (m_sibo_control & 0x0f)
		{
		case 0x00: // PS34R_ADC
			switch (BIT(m_a3_control2, 5, 2)) // Adcsel
			{
			case 0: // ADCSEL_ADCIN
				data = m_adin_cb() & 0xff;
				break;
			case 1: // ADCSEL_VIN
				data = 0xff;
				break;
			case 2: // ADCSEL_VH
				break;
			case 3: // ADCSEL_VBATT
				data = 0xff;
				break;
			}
			LOG("%s data_r: PS34R_ADC %d %02x\n", machine().describe_context(), BIT(m_a3_control2, 5, 2), data);
			break;

		case 0x01: // PS34R_STATUS
			// b0-b2 Admsb
			// b3    Nc
			// b4    Penup
			// b5    Vhready
			// b6    ColdStart
			// b7    PowerFail
			switch (BIT(m_a3_control2, 5, 2)) // Adcsel
			{
			case 0: // ADCSEL_ADCIN
				data = BIT(m_adin_cb(), 8, 3);
				break;
			case 1: // ADCSEL_VIN
				data = 0x07;
				break;
			case 2: // ADCSEL_VH
				break;
			case 3: // ADCSEL_VBATT
				data = 0x07;
				break;
			}
			data |= 0x40; // PowerFail (required to initialise MC600)
			LOG("%s data_r: PS34R_STATUS %02x\n", machine().describe_context(), data);
			break;

		default:
			LOG("%s data_r: unknown control %02x data %02x\n", machine().describe_context(), m_sibo_control, data);
			break;
		}
		break;
	}

	return data;
}
