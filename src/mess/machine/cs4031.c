/***************************************************************************

    Chips & Technologies CS4031 chipset

    Chipset for 486 based PC/AT compatible systems. Consists of two
    individual chips:

    * F84031
        - DRAM controller
        - ISA-bus controller
        - VESA VL-BUS controller

    * F84035
        - 2x 8257 DMA controller
        - 2x 8259 interrupt controller
        - 8254 timer
        - MC14818 RTC

***************************************************************************/

#include "emu.h"
#include "machine/ram.h"
#include "machine/cs4031.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#define LOG_REGISTER	1
#define LOG_MEMORY		1

const device_type CS4031 = &device_creator<cs4031_device>;

enum
{
	DMA_WAIT_STATE = 0x01,
	PERFORMANCE = 0x08,
	F84035_MISC = 0x09,
	DMA_CLOCK = 0x0a,
	SHADOW_READ = 0x19,
	SHADOW_WRITE = 0x1a,
	ROMCS = 0x1b
};

static const char *const register_names[] =
{
	/* 00 */ "RESERVED",
	/* 01 */ "DMA WAIT STATE CONTROL",
	/* 02 */ "RESERVED",
	/* 03 */ "RESERVED",
	/* 04 */ "RESERVED",
	/* 05 */ "ISA BUS COMMAND DELAY",
	/* 06 */ "ISA BUS WAIT STATES AND ADDRESS HOLD",
	/* 07 */ "ISA BUS CLOCK SELECTION",
	/* 08 */ "PERFORMANCE CONTROL",
	/* 09 */ "84035 MISC CONTROL",
	/* 0a */ "DMA CLOCK SELECTION",
	/* 0b */ "RESERVED",
	/* 0c */ "RESERVED",
	/* 0d */ "RESERVED",
	/* 0e */ "RESERVED",
	/* 0f */ "RESERVED",
	/* 10 */ "DRAM TIMING",
	/* 11 */ "DRAM SETUP",
	/* 12 */ "DRAM CONFIGURATION 0 AND 1",
	/* 13 */ "DRAM CONFIGURATION 2 AND 3",
	/* 14 */ "DRAM BLOCK 0 STARTING ADDRESS",
	/* 15 */ "DRAM BLOCK 1 STARTING ADDRESS",
	/* 16 */ "DRAM BLOCK 2 STARTING ADDRESS",
	/* 17 */ "DRAM BLOCK 3 STARTING ADDRESS",
	/* 18 */ "VIDEO AREA SHADOW AND LOCAL BUS CONTROL",
	/* 19 */ "DRAM SHADOW READ ENABLE",
	/* 1a */ "DRAM SHADOW WRITE ENABLE",
	/* 1b */ "ROMCS ENABLE",
	/* 1c */ "SOFT RESET AND GATEA20",
	/* 1d */ "RESERVED",
	/* 1e */ "RESERVED",
	/* 1f */ "RESERVED"
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cs4031_device - constructor
//-------------------------------------------------

cs4031_device::cs4031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CS4031, "CS4031", tag, owner, clock),
	  m_address(0),
	  m_address_valid(false)
{
}

void cs4031_device::static_set_cputag(device_t &device, const char *tag)
{
	cs4031_device &cs4031 = downcast<cs4031_device &>(device);
	cs4031.m_cputag = tag;
}

void cs4031_device::static_set_isatag(device_t &device, const char *tag)
{
	cs4031_device &cs4031 = downcast<cs4031_device &>(device);
	cs4031.m_isatag = tag;
}

void cs4031_device::static_set_biostag(device_t &device, const char *tag)
{
	cs4031_device &cs4031 = downcast<cs4031_device &>(device);
	cs4031.m_biostag = tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cs4031_device::device_start()
{
	ram_device *ram_dev = machine().device<ram_device>(RAM_TAG);

	// make sure the ram device is already running
	if (!ram_dev->started())
		throw device_missing_dependencies();

	device_t *cpu = machine().device(m_cputag);
	m_space = cpu->memory().space(AS_PROGRAM);
	m_isa = machine().root_device().memregion(m_isatag)->base();
	m_bios = machine().root_device().memregion(m_biostag)->base();

	m_ram = ram_dev->pointer();
	UINT32 ram_size = ram_dev->size();

	// install base memory
	m_space->install_ram(0x000000, 0x09ffff, m_ram);

	// install extended memory
	if (ram_size > 0x100000) {
		m_space->install_ram(0x100000, ram_size - 1, m_ram + 0x100000);
	}

	// install bios rom at cpu inital pc
	m_space->install_rom(0xffff0000, 0xffffffff, m_bios + 0xf0000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cs4031_device::device_reset()
{
	// setup default values
	memset(&m_registers, 0x00, sizeof(m_registers));
	m_registers[ROMCS] = 0x60;

	// update rom/ram regions below 1mb
	update_read_regions();
	update_write_regions();
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void cs4031_device::update_read_region(int index, const char *region, offs_t start, offs_t end)
{
	if (!BIT(m_registers[SHADOW_READ], index) && BIT(m_registers[ROMCS], index))
	{
		if (LOG_MEMORY)
			logerror("ROM read from %x to %x\n", start, end);

		m_space->install_read_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_bios + start);
	}
	else if (!BIT(m_registers[SHADOW_READ], index) && !BIT(m_registers[ROMCS], index))
	{
		if (LOG_MEMORY)
			logerror("ISA read from %x to %x\n", start, end);

		m_space->install_read_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_isa + start - 0xc0000);
	}
	else if (BIT(m_registers[SHADOW_READ], index))
	{
		if (LOG_MEMORY)
			logerror("RAM read from %x to %x\n", start, end);

		m_space->install_read_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_ram + start);
	}
	else
	{
		if (LOG_MEMORY)
			logerror("NOP read from %x to %x\n", start, end);

		m_space->nop_read(start, end);
	}
}

void cs4031_device::update_write_region(int index, const char *region, offs_t start, offs_t end)
{
	if (!BIT(m_registers[SHADOW_WRITE], index) && BIT(m_registers[ROMCS], index) && BIT(m_registers[ROMCS], 7))
	{
		if (LOG_MEMORY)
			logerror("ROM write from %x to %x\n", start, end);

		m_space->install_write_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_bios + start);
	}
	else if (!BIT(m_registers[SHADOW_WRITE], index) && !BIT(m_registers[ROMCS], index))
	{
		if (LOG_MEMORY)
			logerror("ISA write from %x to %x\n", start, end);

		m_space->install_write_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_isa + start - 0xc0000);
	}
	else if (BIT(m_registers[SHADOW_WRITE], index))
	{
		if (LOG_MEMORY)
			logerror("RAM write from %x to %x\n", start, end);

		m_space->install_write_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_ram + start);
	}
	else
	{
		if (LOG_MEMORY)
			logerror("NOP write from %x to %x\n", start, end);

		m_space->nop_write(start, end);
	}
}

void cs4031_device::update_read_regions()
{
	update_read_region(0, "read_c0000", 0xc0000, 0xc3fff);
	update_read_region(1, "read_c4000", 0xc4000, 0xc7fff);
	update_read_region(2, "read_c8000", 0xc8000, 0xcbfff);
	update_read_region(3, "read_cc000", 0xcc000, 0xcffff);
	update_read_region(4, "read_d0000", 0xd0000, 0xdffff);
	update_read_region(5, "read_e0000", 0xe0000, 0xeffff);
	update_read_region(6, "read_f0000", 0xf0000, 0xfffff);
}

void cs4031_device::update_write_regions()
{
	update_write_region(0, "write_c0000", 0xc0000, 0xc3fff);
	update_write_region(1, "write_c4000", 0xc4000, 0xc7fff);
	update_write_region(2, "write_c8000", 0xc8000, 0xcbfff);
	update_write_region(3, "write_cc000", 0xcc000, 0xcffff);
	update_write_region(4, "write_d0000", 0xd0000, 0xdffff);
	update_write_region(5, "write_e0000", 0xe0000, 0xeffff);
	update_write_region(6, "write_f0000", 0xf0000, 0xfffff);
}

WRITE8_MEMBER( cs4031_device::address_w )
{
	m_address = data;
	m_address_valid = (m_address < 0x20) ? true : false;
}

READ8_MEMBER( cs4031_device::data_r )
{
	UINT8 result = 0xff;

	if (m_address_valid)
	{
		if (LOG_REGISTER)
			logerror("cs4031_device: read %s = %02x\n", register_names[m_address], m_registers[m_address]);

		result = m_registers[m_address];
	}

	// after a read the selected address needs to be reset
	m_address_valid = false;

	return result;
}

WRITE8_MEMBER( cs4031_device::data_w )
{
	if (m_address_valid)
	{
		if (LOG_REGISTER)
			logerror("cs4031_device: write %s = %02x\n", register_names[m_address], data);

		// update register with new data
		m_registers[m_address] = data;

		// execute command
		switch (m_address)
		{
		case 0x01: break;
		case 0x05: break;
		case 0x06: break;
		case 0x07: break;
		case 0x08: break;
		case 0x09: break;
		case 0x0a: break;
		case 0x10: break;
		case 0x11: break;
		case 0x12: break;
		case 0x13: break;
		case 0x14: break;
		case 0x15: break;
		case 0x16: break;
		case 0x17: break;
		case 0x18: break;

		case 0x19:
			update_read_regions();
			break;

		case 0x1a:
			update_write_regions();
			break;

		case 0x1b:
			update_read_regions();
			update_write_regions();
			break;

		case 0x1c: break;
		}
	}

	// after a write the selected address needs to be reset
	m_address_valid = false;
}
