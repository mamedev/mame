// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC3

    MC and HC power supplies are based on a full custom liner ASIC known as ASIC3.
    This custom chip is manufactured by Maxim and has the Maxim part number MAX616.

    TODO:
    - Battery status readings, maybe different analogue channel.
    - Resolve ambiguity about status register, S3 and MC/HC machines expect different data.

******************************************************************************/

#include "emu.h"
#include "psion_asic3.h"

#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_ASIC3, psion_asic3_device, "psion_asic3", "Psion ASIC3")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_asic3_device::psion_asic3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_ASIC3, tag, owner, clock)
	, m_adin_cb(*this)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void psion_asic3_device::device_resolve_objects()
{
	m_adin_cb.resolve_safe(0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_asic3_device::device_start()
{
	save_item(NAME(m_a3_status));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic3_device::device_reset()
{
	m_a3_status = 0x00;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void psion_asic3_device::data_w(uint16_t data)
{
	switch (data & 0x300)
	{
	case NULL_FRAME:
		break;

	case CONTROL_FRAME:
		m_sibo_control = data & 0xff;

		switch (m_sibo_control & 0xc0)
		{
		case 0x80: // SerialWrite
			break;
		}
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

		case 0x04: // A3Dummy1
			LOG("%s data_w: A3Dummy1 register %02x\n", machine().describe_context(), data);
			break;

		case 0x07: // A3Control3
			// b0    xDummy1
			// b1    OffEnable
			// b2    AdcReadHighEnable
			LOG("%s data_w: A3Control3 register %02x\n", machine().describe_context(), data);
			m_a3_control3 = data;
			break;

		default:
			LOG("%s data_w: Unhandled register %02x\n", machine().describe_context(), m_sibo_control & 0x0f);
			break;
		}
	}
}

uint8_t psion_asic3_device::data_r()
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
			// A3AdcLsbR
			//   b0-b6 OtherBits
			//   b7    InvertedBit
			// A3AdcMsbR
			//   b0-b3 AdcBits
			//   b4    Overrange
			//   b5    Polarity
			if (BIT(m_a3_control3, 2)) // AdcReadHighEnable
				data = BIT(m_adin_cb(), 8, 4);
			else
				data = (m_adin_cb() & 0xff) ^ 0x80;
			LOG("%s data_r: A3Adc register %02x\n", machine().describe_context(), data);
			break;

		case 0x0d: // A3Status
			// b0-b2 Admsb
			// b3    Nc
			// b4    Penup
			// b5    Vhready
			// b6    ColdStart
			// b7    PowerFail

			// b0    ColdStart
			// b1    PowerFail
			data = 0x40;
			LOG("%s data_r: A3Status register %02x\n", machine().describe_context(), data);
			break;

		default:
			LOG("%s data_r: Unhandled register %02x\n", machine().describe_context(), m_sibo_control & 0x0f);
			break;
		}
		break;
	}

	return data;
}
