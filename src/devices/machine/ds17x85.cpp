// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Dallas DS17x85/DS17x87 Real Time Clocks with extended NVSRAM

    These MC146818-compatible RTCs overlay the last 64 bytes of user
    RAM (not provided by the MC146818) with a second page of registers,
    including a Y2K-updated century count and registers for indirectly
    accessing extended SRAM which is backed with a separate auxiliary
    battery power input.

    The DS1685 and DS17x85 use conventional PDIP/SO/TSOP packages,
    while the DS1687 and DS17x87 integrate battery, oscillator and chip
    into one of Dallas's trademark EDIPs. They are indistinguishable
    from the perspective of software.

    Currently unemulated features include:
    - Kickstart input
    - Date alarm wakeup
    - Increment in progress flag
    - RAM clear function (software-enabled, hardware-triggered)
    - 32.768 kHz frequency option for SQW output

**********************************************************************/

#include "emu.h"
#include "ds17x85.h"

#define VERBOSE 0
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(DS1685, ds1685_device, "ds1685", "DS1685 RTC")
DEFINE_DEVICE_TYPE(DS1687, ds1687_device, "ds1687", "DS1687 RTC")
DEFINE_DEVICE_TYPE(DS17285, ds17285_device, "ds17285", "DS17285 RTC")
DEFINE_DEVICE_TYPE(DS17287, ds17287_device, "ds17287", "DS17287 RTC")
DEFINE_DEVICE_TYPE(DS17485, ds17485_device, "ds17485", "DS17485 RTC")
DEFINE_DEVICE_TYPE(DS17487, ds17487_device, "ds17487", "DS17487 RTC")
DEFINE_DEVICE_TYPE(DS17885, ds17885_device, "ds17885", "DS17885 RTC")
DEFINE_DEVICE_TYPE(DS17887, ds17887_device, "ds17887", "DS17887 RTC")

//-------------------------------------------------
//  ds17x85_device - constructor
//-------------------------------------------------

ds17x85_device::ds17x85_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 model, u32 extram_size)
	: mc146818_device(mconfig, type, tag, owner, clock)
	, m_model(model)
	, m_extram_size(extram_size)
	, m_smi_stack1(0)
{
	m_century_index = REG_EXT_CENTURY;
}

//-------------------------------------------------
//  ds1685_device - constructor
//-------------------------------------------------

ds1685_device::ds1685_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds17x85_device(mconfig, DS1685, tag, owner, clock, 0x71, 128)
{
}

//-------------------------------------------------
//  ds1687_device - constructor
//-------------------------------------------------

ds1687_device::ds1687_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds17x85_device(mconfig, DS1687, tag, owner, clock, 0x71, 128)
{
}

//-------------------------------------------------
//  ds17285_device - constructor
//-------------------------------------------------

ds17285_device::ds17285_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds17x85_device(mconfig, DS17285, tag, owner, clock, 0x72, 2048)
{
}

//-------------------------------------------------
//  ds17287_device - constructor
//-------------------------------------------------

ds17287_device::ds17287_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds17x85_device(mconfig, DS17287, tag, owner, clock, 0x72, 2048)
{
}

//-------------------------------------------------
//  ds17485_device - constructor
//-------------------------------------------------

ds17485_device::ds17485_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds17x85_device(mconfig, DS17485, tag, owner, clock, 0x74, 4096)
{
}

//-------------------------------------------------
//  ds17487_device - constructor
//-------------------------------------------------

ds17487_device::ds17487_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds17x85_device(mconfig, DS17487, tag, owner, clock, 0x74, 4096)
{
}

//-------------------------------------------------
//  ds17885_device - constructor
//-------------------------------------------------

ds17885_device::ds17885_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds17x85_device(mconfig, DS17885, tag, owner, clock, 0x78, 8192)
{
}

//-------------------------------------------------
//  ds17887_device - constructor
//-------------------------------------------------

ds17887_device::ds17887_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds17x85_device(mconfig, DS17887, tag, owner, clock, 0x78, 8192)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds17x85_device::device_start()
{
	mc146818_device::device_start();

	save_item(NAME(m_smi_stack1));
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void ds17x85_device::nvram_default()
{
	mc146818_device::nvram_default();

	if (!m_region.found())
	{
		// initialize read-only model number and 64-bit serial number
		m_data[REG_EXT_MODEL] = m_model;
		m_data[REG_EXT_SERIAL1] = 0x4d;
		m_data[REG_EXT_SERIAL2] = 0x41;
		m_data[REG_EXT_SERIAL3] = 0x4d;
		m_data[REG_EXT_SERIAL4] = 0x45;
		m_data[REG_EXT_SERIAL5] = 0x64;
		m_data[REG_EXT_SERIAL6] = 0x76;
		m_data[REG_EXT_SERIAL_CRC] = 0xf8;
	}
}

//---------------------------------------------------------------
//  get_timer_bypass - get main clock divisor based on A register
//---------------------------------------------------------------

int ds17x85_device::get_timer_bypass() const
{
	int bypass;

	// DV0 is used for bank switching only
	switch (m_data[REG_A] & (REG_A_DV2 | REG_A_DV1))
	{
	case REG_A_DV1:
		bypass = 7;
		break;

	case REG_A_DV2 | REG_A_DV1:
		bypass = 22;
		break;

	default:
		// TODO: other combinations of divider bits disable the oscillator
		bypass = 22;
		break;
	}

	return bypass;
}

//---------------------------------------------------------------
//  internal_set_address - latch address on ALE
//---------------------------------------------------------------

void ds17x85_device::internal_set_address(uint8_t address)
{
	// push previous address onto SMI recovery stack
	m_data[REG_EXT_SMI_STACK3] = m_data[REG_EXT_SMI_STACK2];
	m_data[REG_EXT_SMI_STACK2] = m_smi_stack1;
	m_smi_stack1 = m_index | ((m_data[REG_A] & REG_A_DV0) ? 0x80 : 0x00);

	mc146818_device::internal_set_address(address);
}

//---------------------------------------------------------------
//  internal_read - read internal data
//---------------------------------------------------------------

uint8_t ds17x85_device::internal_read(offs_t offset)
{
	if (offset >= 0x40 && (m_data[REG_A] & REG_A_DV0))
	{
		offset += 0x40;
		switch (offset)
		{
		case REG_EXT_4A:
			return m_data[offset] | REG_EXT_4A_VRT2;

		case REG_EXT_RAM_DATA:
			LOG("%s: Reading from extended RAM at %04X\n", machine().describe_context(),
				m_data[REG_EXT_RAM_ADDRL] | offs_t(m_data[REG_EXT_RAM_ADDRH]) << 8);
			offset = REG_EXT_RAM_BASE + (m_data[REG_EXT_RAM_ADDRL] | offs_t(m_data[REG_EXT_RAM_ADDRH]) << 8) % m_extram_size;
			if ((m_data[REG_EXT_4A] & REG_EXT_4A_BME) && !machine().side_effects_disabled())
			{
				m_data[REG_EXT_RAM_ADDRL]++;
				if (m_data[REG_EXT_RAM_ADDRL] == 0 && m_extram_size > 256)
					m_data[REG_EXT_RAM_ADDRH]++;
			}
			break;

		default:
			break;
		}
	}

	return mc146818_device::internal_read(offset);
}

//---------------------------------------------------------------
//  internal_write - write internal data
//---------------------------------------------------------------

void ds17x85_device::internal_write(offs_t offset, uint8_t data)
{
	if (!machine().side_effects_disabled())
		m_data[REG_EXT_WRITE_COUNTER]++;

	if (offset >= 0x40 && (m_data[REG_A] & REG_A_DV0))
	{
		offset += 0x40;
		switch (offset)
		{
		case REG_EXT_RAM_DATA:
			LOG("%s: Writing %02X to extended RAM at %04X\n", machine().describe_context(),
				data, m_data[REG_EXT_RAM_ADDRL] | offs_t(m_data[REG_EXT_RAM_ADDRH]) << 8);
			offset = REG_EXT_RAM_BASE + (m_data[REG_EXT_RAM_ADDRL] | offs_t(m_data[REG_EXT_RAM_ADDRH]) << 8) % m_extram_size;
			if ((m_data[REG_EXT_4A] & REG_EXT_4A_BME) && !machine().side_effects_disabled())
			{
				m_data[REG_EXT_RAM_ADDRL]++;
				if (m_data[REG_EXT_RAM_ADDRL] == 0 && m_extram_size > 256)
					m_data[REG_EXT_RAM_ADDRH]++;
			}
			break;

		case REG_EXT_CENTURY:
		case REG_EXT_DATE_ALARM: // TODO: unimplemented
		case REG_EXT_4A: // TODO: mostly unimplemented
		case REG_EXT_4B: // TODO: unimplemented
		case REG_EXT_RAM_ADDRL:
			break;

		case REG_EXT_RAM_ADDRH:
			if (m_extram_size <= 256)
			{
				logerror("%s: RTC extended addressing uses one byte only\n", machine().describe_context());
				return;
			}
			break;

		default:
			logerror("%s: Write to reserved/read-only bank 1 register %02X\n", machine().describe_context(), offset - 0x40);
			return;
		}
	}

	mc146818_device::internal_write(offset, data);
}
