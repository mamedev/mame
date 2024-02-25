// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC4

    ASIC4 is a serial protocol slave IC for addressing memory and general
    memory-mapped peripherals. It is used in SSDs to convert SIBO serial protocol
    signals into addresses within the memory range of the memory pack. ASIC4 was
    designed to be a cut-down version of ASIC5 which was the original SIBO serial
    protocol slave chip.

    A27   A26 A25 A24                        D7  D6  D5                  D4  D3              D2  D1  D0
     M    Peripheral Id                      Memory Type                 No. devices         Memory Size
     1     0   0   0 No peripheral           0   0   0  RAM              0   0  1 device     0   0   0  No memory
           0   0   1 Turbo RS232 (16550)     0   0   1  Type 1 Flash     0   1  2 devices    0   0   1  32K
           0   1   0 3Fax                    0   1   0  Type 2 Flash     1   0  3 devices    0   1   0  64K
           1   1   1 Extended info in ROM    1   1   0  ROM              1   1  4 devices    0   1   1  128K
                                                                                             1   0   0  256K
                                                                                             1   0   1  512K
                                                                                             1   1   0  1M

******************************************************************************/

#include "emu.h"
#include "psion_asic4.h"

#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_ASIC4, psion_asic4_device, "psion_asic4", "Psion ASIC4")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_asic4_device::psion_asic4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_ASIC4, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("program", ENDIANNESS_LITTLE, 8, 28)
{
}


device_memory_interface::space_config_vector psion_asic4_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_space_config) };
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_asic4_device::device_start()
{
	m_space = &space(0);

	save_item(NAME(m_addr_latch));
	save_item(NAME(m_addr_writes));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic4_device::device_reset()
{
	m_addr_latch  = 0x00;
	m_addr_writes = 0;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void psion_asic4_device::data_w(uint16_t data)
{
	switch (data & 0x300)
	{
	case NULL_FRAME:
		device_reset();
		break;

	case CONTROL_FRAME:
		m_sibo_control = data & 0xff;

		// reset address register to LSB
		m_addr_writes = 0;
		break;

	case DATA_FRAME:
		data &= 0xff;
		switch (m_sibo_control & 0x0f)
		{
		case 0x00: // Data register
			LOG("%s data_w: Data register %02x\n", machine().describe_context(), data);
			m_space->write_byte(m_addr_latch, data);
			break;

		case 0x01: // Device Size register
			LOG("%s data_w: Device Size register %02x\n", machine().describe_context(), data);
			break;

		case 0x02: // Address Increment register
			LOG("%s data_w: Address Increment register %02x\n", machine().describe_context(), data);
			m_addr_latch = (m_addr_latch & 0xffffff0) | ((m_addr_latch + 1) & 0x0f);
			break;

		case 0x03: // Address register
			LOG("%s data_w: Address register %02x\n", machine().describe_context(), data);
			if (m_addr_writes == 0)
				m_addr_latch = 0x00;
			m_addr_latch |= data << (m_addr_writes << 3);
			m_addr_writes = (m_addr_writes + 1) & 3;
			break;

		case 0x07: // Control register
			LOG("%s data_w: Control register %02x\n", machine().describe_context(), data);
			break;

		default:
			LOG("%s data_w: unknown control %02x data %02x\n", machine().describe_context(), m_sibo_control, data);
			break;
		}
		break;
	}
}

uint8_t psion_asic4_device::data_r()
{
	uint8_t data = 0x00;

	switch (m_sibo_control & 0xc0)
	{
	case 0x40: // SerialSelect
		switch (m_sibo_control & 0x0f)
		{
		case 0x06: // Asic4Id
			data = m_info_byte & 0xff; // A4InfoByte
			break;
		}
		break;

	case 0xc0: // SerialRead
		switch (m_sibo_control & 0x0f)
		{
		case 0x00: // Data register
			data = m_space->read_byte(m_addr_latch);
			LOG("%s data_r: Data register %02x\n", machine().describe_context(), data);
			m_addr_latch++;
			break;

		case 0x01: // Input register
			data = (m_info_byte >> 4) & 0xf0;
			LOG("%s data_r: Input register %02x\n", machine().describe_context(), data);
			break;

		default:
			LOG("%s data_r: unknown control %02x data %02x\n", machine().describe_context(), m_sibo_control, data);
			break;
		}
		break;
	}

	return data;
}
