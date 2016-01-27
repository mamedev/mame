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

const device_type A26_ROM_SUPERCHARGER = &device_creator<a26_rom_ss_device>;


a26_rom_ss_device::a26_rom_ss_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
						: a26_rom_f6_device(mconfig, A26_ROM_SUPERCHARGER, "Atari 2600 ROM Cart Supercharger", tag, owner, clock, "a2600_ss", __FILE__),
							m_cassette(*this, "cassette"),
	m_maincpu(nullptr),
	m_reg(0),
	m_write_delay(0),
	m_ram_write_enabled(0),
	m_rom_enabled(0),
	m_byte_started(0),
	m_last_address(0),
	m_diff_adjust(0)
					{
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void a26_rom_ss_device::device_start()
{
	m_maincpu = machine().device<cpu_device>("maincpu");

	save_item(NAME(m_base_banks));
	save_item(NAME(m_reg));
	save_item(NAME(m_write_delay));
	save_item(NAME(m_ram_write_enabled));
	save_item(NAME(m_rom_enabled));
	save_item(NAME(m_byte_started));
	save_item(NAME(m_last_address));
	save_item(NAME(m_diff_adjust));
}

void a26_rom_ss_device::device_reset()
{
	// banks = 0, 1, 2 are 2K chunk of RAM (of the available 6K), banks = 3 is ROM!
	m_base_banks[0] = 2;
	m_base_banks[1] = 3;
	m_ram_write_enabled = 0;
	m_byte_started = 0;
	m_reg = 0;
	m_write_delay = 0;
	m_rom_enabled = 1;
	m_last_address = 0;
	m_diff_adjust = 0;
}


static MACHINE_CONFIG_FRAGMENT( a26_ss )
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(a26_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("a2600_cass")

//  MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

machine_config_constructor a26_rom_ss_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a26_ss );
}

inline UINT8 a26_rom_ss_device::read_byte(UINT32 offset)
{
	if (offset < 0x800)
		return m_ram[(offset & 0x7ff) + (m_base_banks[0] * 0x800)];
	else if (m_base_banks[1] != 3)
		return m_ram[(offset & 0x7ff) + (m_base_banks[1] * 0x800)];
	else if (m_rom_enabled)
		return m_rom[offset & 0x7ff];
	else
		return 0xff;
}

READ8_MEMBER(a26_rom_ss_device::read_rom)
{
	if (space.debugger_access())
		return read_byte(offset);

	// Bankswitch
	if (offset == 0xff8)
	{
		//logerror("%04X: Access to control register data = %02X\n", m_maincpu->pc(), m_modeSS_byte);
		m_write_delay = m_reg >> 5;
		m_ram_write_enabled = BIT(m_reg, 1);
		m_rom_enabled = !BIT(m_reg, 0);

		// compensate time spent in this access to avoid spurious RAM write
		m_byte_started -= 5;

		// handle bankswitch
		switch (m_reg & 0x1c)
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

		return read_byte(offset);
	}
	// Cassette port read
	else if (offset == 0xff9)
	{
		//logerror("%04X: Cassette port read, tap_val = %f\n", m_maincpu->pc(), tap_val);
		double tap_val = m_cassette->input();

		// compensate time spent in this access to avoid spurious RAM write
		m_byte_started -= 5;

		if (tap_val < 0)
			return 0x00;
		else
			return 0x01;
	}
	// Possible RAM write
	else
	{
		if (m_ram_write_enabled)
		{
			/* Check for dummy read from same address */
			if (m_last_address == offset)
				m_diff_adjust++;

			int diff = m_maincpu->total_cycles() - m_byte_started;
			//logerror("%04X: offset = %04X, %d\n", m_maincpu->pc(), offset, diff);

			if (diff - m_diff_adjust == 5)
			{
				//logerror("%04X: RAM write offset = %04X, data = %02X\n", m_maincpu->pc(), offset, m_modeSS_byte );
				if (offset < 0x800)
					m_ram[(offset & 0x7ff) + (m_base_banks[0] * 0x800)] = m_reg;
				else if (m_base_banks[1] != 3)
					m_ram[(offset & 0x7ff) + (m_base_banks[1] * 0x800)] = m_reg;
			}
			else if (offset < 0x0100)
			{
				m_reg = offset;
				m_byte_started = m_maincpu->total_cycles();
				m_diff_adjust = 0;
			}
		}
		else if (offset < 0x0100)
		{
			m_reg = offset;
			m_byte_started = m_maincpu->total_cycles();
			m_diff_adjust = 0;
		}
		m_last_address = offset;
		return read_byte(offset);
	}
}
