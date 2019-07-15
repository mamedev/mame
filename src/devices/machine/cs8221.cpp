// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Chips & Technologies CS8221 chipset

    a.k.a. NEW ENHANCED AT (NEAT)

    Consists of four individual chips:

    * 82C211 - CPU/Bus controller
    * 82C212 - Page/Interleave and EMS Memory controller
    * 82C215 - Data/Address buffer
    * 82C206 - Integrated Peripherals Controller(IPC)

***************************************************************************/

#include "emu.h"
#include "machine/cs8221.h"
#include "machine/ram.h"

#define LOG_GENERAL     (1U << 0)
#define LOG_REGISTER    (1U << 1)
#define LOG_MEMORY      (1U << 2)

#define VERBOSE (LOG_REGISTER | LOG_MEMORY)
#include "logmacro.h"

#define LOGREGISTER(...)    LOGMASKED(LOG_REGISTER, __VA_ARGS__)
#define LOGMEMORY(...)      LOGMASKED(LOG_MEMORY,   __VA_ARGS__)


DEFINE_DEVICE_TYPE(CS8221, cs8221_device, "cs8221", "CS8221 NEAT")


static const char *const register_names[] =
{
	/* 00 */ "PROCCLK",
	/* 01 */ "COMMAND DELAY",
	/* 02 */ "WAIT STATES",
	/* 03 */ "RESERVED",
	/* 04 */ "VERSION",
	/* 05 */ "ROM CONFIGURATION",
	/* 06 */ "MEMORY ENABLE-1",
	/* 07 */ "MEMORY ENABLE-2",
	/* 08 */ "MEMORY ENABLE-3",
	/* 09 */ "MEMORY ENABLE-4",
	/* 0a */ "BANK 0/1 ENABLE",
	/* 0b */ "DRAM CONFIGURATION",
	/* 0c */ "BANK 2/3 ENABLE",
	/* 0d */ "EMS BASE ADDRESS",
	/* 0e */ "EMS ADDRESS EXTENSION",
	/* 0f */ "MISCELLANEOUS"
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cs8221_device - constructor
//-------------------------------------------------

cs8221_device::cs8221_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CS8221, tag, owner, clock),
		m_address(0),
		m_address_valid(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cs8221_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cs8221_device::device_reset()
{
	// setup default values
	memset(&m_registers, 0x00, sizeof(m_registers));
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************
void cs8221_device::map(address_map &map)
{
	map(0x0022, 0x0022).w("cs8221", FUNC(cs8221_device::address_w));
	map(0x0023, 0x0023).rw("cs8221", FUNC(cs8221_device::data_r), FUNC(cs8221_device::data_w));
}

WRITE8_MEMBER( cs8221_device::address_w )
{
	m_address = data;
	m_address_valid = ((m_address & 0x60)== 0x60) ? true : false;
}

READ8_MEMBER( cs8221_device::data_r )
{
	uint8_t result = 0xff;

	if (m_address_valid)
	{
		LOGREGISTER("cs8221_device: read %s = %02x\n", register_names[m_address & 0x0f], m_registers[m_address & 0x0f]);

		result = m_registers[m_address & 0x0f];
	}

	// after a read the selected address needs to be reset
	m_address_valid = false;

	return result;
}

WRITE8_MEMBER( cs8221_device::data_w )
{
	if (m_address_valid)
	{
		LOGREGISTER("cs8221_device: write %s = %02x\n", register_names[m_address & 0x0f], data);

		// update register with new data
		m_registers[m_address & 0x0f] = data;

		// execute command
		switch (m_address)
		{
			case 0x60: break;
			case 0x61: break;
			case 0x62: break;
			case 0x63: break;
			case 0x64: break;
			case 0x65: break;
			case 0x66: break;
			case 0x67: break;
			case 0x68: break;
			case 0x69: break;
			case 0x6a: break;
			case 0x6b: break;
			case 0x6c: break;
			case 0x6d: break;
			case 0x6e: break;
			case 0x6f: break;
		}
	}

	// after a write the selected address needs to be reset
	m_address_valid = false;
}
