// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Atari 2600 cart Starpath Supercharger (Cart + Tape drive!)



 From kevtris notes ( http://blog.kevtris.org/blogfiles/Atari%202600%20Mappers.txt ):


 - Control register [0x1ff8]

       7       0
       ---------
 1FF8: DDDB BBWE

 D: write delay (see below)
 B: bankswitching mode (see below)
 W: RAM write enable (1 = enabled, 0 = disabled)
 E: ROM power enable (0 = enabled, 1 = turn off ROM)

 - Audio input register [0x1ff9]

        7       0
        ---------
 1FF9:  0000 000A

 A: Supercharger audio data.  0 = low input, 1 = high input.


***************************************************************************/


#include "emu.h"
#include "scharger.h"
#include "sound/wave.h"
#include "formats/a26_cas.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(A26_ROM_SUPERCHARGER, a26_rom_ss_device, "a2600_ss", "Atari 2600 ROM Cart Supercharger")


a26_rom_ss_device::a26_rom_ss_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a26_rom_base_device(mconfig, A26_ROM_SUPERCHARGER, tag, owner, clock),
	m_cassette(*this, "cassette"),
	m_data(0),
	m_write_delay(0),
	m_ram_write_enabled(0),
	m_rom_enabled(0),
	m_last_address_bus(0),
	m_address_bus_changes(0)
{
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void a26_rom_ss_device::device_start()
{
	save_item(NAME(m_base_banks));
	save_item(NAME(m_data));
	save_item(NAME(m_write_delay));
	save_item(NAME(m_ram_write_enabled));
	save_item(NAME(m_rom_enabled));
	save_item(NAME(m_last_address_bus));
	save_item(NAME(m_address_bus_changes));
}

void a26_rom_ss_device::device_reset()
{
	// banks = 0, 1, 2 are 2K chunk of RAM (of the available 6K), banks = 3 is ROM
	m_base_banks[0] = 2;
	m_base_banks[1] = 3;
	m_ram_write_enabled = false;
	m_data = 0;
	m_write_delay = 0;
	m_rom_enabled = true;
	m_last_address_bus = 0;
	m_address_bus_changes = RAM_WRITE_TIME + 1;
}


void a26_rom_ss_device::device_add_mconfig(machine_config &config)
{
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(a26_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("a2600_cass");

//  WAVE(config, "wave", m_cassette).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void a26_rom_ss_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x17ff, read8sm_delegate(*this, FUNC(a26_rom_ss_device::read_lo)));
	space->install_read_handler(0x1800, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_ss_device::read_hi)));
	space->install_read_handler(0x1ff9, 0x1ff9, read8sm_delegate(*this, FUNC(a26_rom_ss_device::read_cass)));
	space->install_readwrite_tap(0x0000, 0x1fff, "watch",
		[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) tap(address); },
		[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) tap(address); });
}

void a26_rom_ss_device::tap(offs_t address)
{
	if (address == 0x1ff8)
	{
		LOG("write control register %02X\n", m_data);
		m_write_delay = m_data >> 5;
		m_ram_write_enabled = BIT(m_data, 1);
		m_rom_enabled = !BIT(m_data, 0);

		// prevent spurious RAM write
		m_address_bus_changes = RAM_WRITE_TIME + 1;

		// handle bankswitch
		switch (m_data & 0x1c)
		{
			case 0x00:
				m_base_banks[0] = 2;
				m_base_banks[1] = 3;
				break;
			case 0x04:
				m_base_banks[0] = 0;
				m_base_banks[1] = 3;
				break;
			case 0x08:
				m_base_banks[0] = 2;
				m_base_banks[1] = 0;
				break;
			case 0x0c:
				m_base_banks[0] = 0;
				m_base_banks[1] = 2;
				break;
			case 0x10:
				m_base_banks[0] = 2;
				m_base_banks[1] = 3;
				break;
			case 0x14:
				m_base_banks[0] = 1;
				m_base_banks[1] = 3;
				break;
			case 0x18:
				m_base_banks[0] = 2;
				m_base_banks[1] = 1;
				break;
			case 0x1c:
				m_base_banks[0] = 1;
				m_base_banks[1] = 2;
				break;
		}
	}
	if (m_address_bus_changes <= RAM_WRITE_TIME && m_last_address_bus != address)
		m_address_bus_changes++;
	m_last_address_bus = address;
}

uint8_t a26_rom_ss_device::read_lo(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_ram_write_enabled)
		{
			if (m_address_bus_changes == RAM_WRITE_TIME)
			{
				LOG("%s: RAM write offset %04X, data %02X\n", machine().describe_context(), offset + (m_base_banks[0] * 0x800), m_data);
				m_ram[offset + (m_base_banks[0] * 0x800)] = m_data;
			}
			else if (offset < 0x0100)
			{
				m_data = offset;
				m_address_bus_changes = 0;
			}
		}
		else if (offset < 0x0100)
		{
			m_data = offset;
			m_address_bus_changes = 0;
		}
	}
	return m_ram[offset + (m_base_banks[0] * 0x800)];
}

uint8_t a26_rom_ss_device::read_hi(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_ram_write_enabled)
		{
			if (m_address_bus_changes == RAM_WRITE_TIME)
			{
				if (m_base_banks[1] != 3)
				{
					LOG("%s: RAM write offset %04X, data %02X\n", machine().describe_context(), offset + (m_base_banks[1] * 0x800), m_data);
					m_ram[offset + (m_base_banks[1] * 0x800)] = m_data;
				}
			}
		}
	}
	if (m_base_banks[1] != 3)
		return m_ram[offset + (m_base_banks[1] * 0x800)];
	else if (m_rom_enabled)
		return m_rom[offset];
	else
		return 0xff;
}

uint8_t a26_rom_ss_device::read_cass(offs_t offset)
{
	// prevent spurious RAM write
	m_address_bus_changes = RAM_WRITE_TIME + 1;

	if (m_cassette->input() < 0)
		return 0x00;
	else
		return 0x01;
}
