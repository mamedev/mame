// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the 28Fxxx flash memory devices, fabricated by many
 * suppliers including Intel, Atmel, AMD, SGS, TI and Toshiba. Range includes:
 *
 *   Part     Bits     Org.
 *   ------  -----   ------
 *   28F256   256K    32Kx8
 *   28F512   512K    64Kx8
 *   28F010  1024K   128Kx8
 *   28F020  2048K   256Kx8
 *   28F040  4096K   512Kx8
 *
 * These devices use a JEDEC-standard pinout allowing them to be fitted to a
 * standard EPROM socket, but have their own command set which is not
 * compatible with the set implemented by the intelfsh devices. It appears the
 * command set used here is a variation on the one defined by JEDEC standard
 * 21-C, as "dual power supply eeprom command set", in figure 3.5.1-11 on page
 * 23, here: https://www.jedec.org/system/files/docs/3_05_01R14.pdf
 *
 * This implementation has been developed primarily against Intel's datasheet,
 * with some minor adjustments to match details in the AMD equivalent.
 *
 * TODO
 *  - implement more variants
 *  - testing in systems other than InterPro
 *
 */

#include "emu.h"
#include "28fxxx.h"

#define VERBOSE 0
#include "logmacro.h"

// manufacturer codes defined by JEDEC: https://www.jedec.org/system/files/docs/JEP106AV.pdf
enum manufacturer_codes
{
	MFG_AMD   = 0x01,
	MFG_INTEL = 0x89,
};

DEFINE_DEVICE_TYPE(INTEL_28F010, intel_28f010_device, "intel_28f010", "Intel 28F010 1024K (128K x 8) CMOS Flash Memory")
DEFINE_DEVICE_TYPE(AMD_28F010, amd_28f010_device, "amd_28f010", "Am28F010 1 Megabit (128K x 8-Bit) CMOS 12.0 Volt, Bulk Erase Flash Memory")
DEFINE_DEVICE_TYPE(AMD_28F020, amd_28f020_device, "amd_28f020", "Am28F020 2 Megabit (256K x 8-Bit) CMOS 12.0 Volt, Bulk Erase Flash Memory")

ALLOW_SAVE_TYPE(base_28fxxx_device::state);

base_28fxxx_device::base_28fxxx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 size, u8 manufacturer_code, u8 device_code)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_region(*this, DEVICE_SELF)
	, m_size(size)
	, m_manufacturer_code(manufacturer_code)
	, m_device_code(device_code)
	, m_program_power(CLEAR_LINE)
	, m_state(STATE_READ_MEMORY)
{
	if (m_size & (m_size - 1))
		throw emu_fatalerror("%s(%s): memory size must be an exact power of two", type.shortname(), tag);
}

intel_28f010_device::intel_28f010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: base_28fxxx_device(mconfig, INTEL_28F010, tag, owner, clock, 0x20000, MFG_INTEL, 0xb4)
{
}

amd_28f010_device::amd_28f010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: base_28fxxx_device(mconfig, AMD_28F010, tag, owner, clock, 0x20000, MFG_AMD, 0xa7)
{
}

amd_28f020_device::amd_28f020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: base_28fxxx_device(mconfig, AMD_28F020, tag, owner, clock, 0x40000, MFG_AMD, 0x2a)
{
}

void base_28fxxx_device::device_start()
{
	m_data = std::make_unique<u8[]>(m_size);

	save_item(NAME(m_program_power));
	save_pointer(NAME(m_data), m_size);

	save_item(NAME(m_state));
	save_item(NAME(m_address_latch));
}

void base_28fxxx_device::nvram_default()
{
	if (m_region.found())
	{
		u32 bytes = m_region->bytes();
		if (bytes > m_size)
			bytes = m_size;

		for (offs_t offs = 0; offs < bytes; offs++)
			m_data[offs] = m_region->as_u8(offs);
	}
	else
		erase();
}

bool base_28fxxx_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_data.get(), m_size, actual) && actual == m_size;
}

bool base_28fxxx_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_data.get(), m_size, actual) && actual == m_size;
}

void base_28fxxx_device::erase()
{
	memset(m_data.get(), 0xff, m_size);
}

u8 base_28fxxx_device::read(address_space &space, offs_t offset, u8 mem_mask)
{
	switch (m_state)
	{
	case STATE_READ_MEMORY:
		return m_data.get()[offset & (m_size - 1)];

	case STATE_READ_IDENTIFIER:
		switch (offset)
		{
		case 0: return m_manufacturer_code;
		case 1: return m_device_code;
		}
		LOG("unknown read identifier offset 0x%08x (%s)\n", offset, machine().describe_context());
		return space.unmap();

	case STATE_ERASE_VERIFY:
		return m_data.get()[m_address_latch];

	case STATE_PROGRAM_VERIFY:
		return m_data.get()[m_address_latch];

	default:
		LOG("unexpected read offset 0x%08x mask 0x%08x state %d (%s)\n", offset, mem_mask, m_state, machine().describe_context());
		return space.unmap();
	}
}

void base_28fxxx_device::write(offs_t offset, u8 data)
{
	// writes are ignored unless Vpp is asserted
	if (!m_program_power)
		return;

	switch (m_state)
	{
	case STATE_ERASE_SETUP:
		if (data == ERASE)
		{
			LOG("erase command initiated (%s)\n", machine().describe_context());
			erase();
			m_state = STATE_ERASE;
		}
		else if (data == RESET)
		{
			LOG("erase command reset (%s)\n", machine().describe_context());
			m_state = STATE_READ_MEMORY;
		}
		else
			LOG("unexpected command 0x%02x in erase setup state (%s)\n", data, machine().describe_context());
		break;

	case STATE_ERASE:
		if (data == ERASE_VERIFY)
		{
			m_address_latch = offset & (m_size - 1);
			m_state = STATE_ERASE_VERIFY;
		}
		else
			LOG("unexpected command 0x%02x in erase state (%s)\n", data, machine().describe_context());
		break;

	case STATE_PROGRAM_SETUP:
		m_address_latch = offset & (m_size - 1);

		LOG("programming address 0x%08x data 0x%02x\n", m_address_latch, data);

		// bits can only be written from uncharged (logical 1) to charged (logical 0) state
		m_data.get()[m_address_latch] &= data;

		m_state = STATE_PROGRAM;
		break;

	case STATE_PROGRAM:
		if (data == PROGRAM_VERIFY)
			m_state = STATE_PROGRAM_VERIFY;
		else if (data == RESET)
		{
			LOG("program command reset (%s)\n", machine().describe_context());

			m_state = STATE_READ_MEMORY;
		}
		else
			LOG("unexpected command 0x%02x in program state (%s)\n", data, machine().describe_context());
		break;

	default:
		// start a new command
		switch (data)
		{
		case READ_MEMORY:
			m_state = STATE_READ_MEMORY;
			break;

		case READ_IDENTIFIER:
		case READ_IDENTIFIER_ALT:
			m_state = STATE_READ_IDENTIFIER;
			break;

		case ERASE:
			m_state = STATE_ERASE_SETUP;
			break;

		case ERASE_VERIFY:
			m_address_latch = offset & (m_size - 1);
			m_state = STATE_ERASE_VERIFY;
			break;

		case PROGRAM:
			m_state = STATE_PROGRAM_SETUP;
			break;

		case RESET:
			m_state = STATE_READ_MEMORY;
			break;

		default:
			LOG("unexpected command 0x%02x in state %d (%s)\n", data, m_state, machine().describe_context());
			break;
		}
		break;
	}
}
